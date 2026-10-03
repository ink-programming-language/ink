#include "ink/execution/ffi/ffi_call.h"

#include "ink/execution/ffi/ffi_argument.h"
#include "ink/execution/ffi/ffi_type.h"
#include "ink/execution/ffi/native_call_cache.h"
#include "ink/execution/ffi/native_symbol_cache.h"

#include <ffi.h>

#include <algorithm>
#include <array>
#include <bit>
#include <climits>
#include <cstring>
#include <limits>
#include <unordered_map>
#include <vector>

namespace ink::execution
{
  namespace
  {
    struct NativeCallPlan final
    {
        RuntimeTypeId Signature = InvalidRuntimeType;
        NativeSymbol Symbol = nullptr;
        ffi_cif Interface{};
        std::vector<ffi_type *> NativeArguments;
        std::vector<FfiValueLayout> Arguments;
        FfiValueLayout Return;
    };

    FfiValueLayout valueLayout(const RuntimeTypeTable &Types, const StorageLayout &Type)
    {
      FfiValueLayout Result;
      Result.Domain = Type.Domain;
      Result.Type = Type.Type;
      Result.Kind = Type.Kind;
      Result.BitWidth = Type.BitWidth;
      Result.Signed = Type.Signed;
      Result.Pointee = Type.Pointee;
      Result.Writable = Type.Writable;
      if (Type.Kind == RuntimeKind::Pointer)
      {
        if (const StorageLayout *Pointee = Types.get(Type.Pointee))
        {
          Result.VoidPointer = Pointee->Kind == RuntimeKind::Void;
          Result.ByteBufferPointer = Pointee->Kind == RuntimeKind::Integer && Pointee->BitWidth == 8;
          Result.BytePointer = Result.ByteBufferPointer && !Pointee->Signed;
        }
      }
      return Result;
    }

    ExecutionStatus validateFunction(const RuntimeTypeTable &Types, const RuntimeFunctionDescriptor &Function, std::span<const RuntimeValue> Arguments)
    {
      if (!Function.NativeAbi)
      {
        return ExecutionStatus::HostAbiMismatch;
      }
      if (!Function.Supported)
      {
        return ExecutionStatus::UnsupportedExternalSignature;
      }
      const StorageLayout *Signature = Types.get(Function.Signature);
      if (!Signature || Signature->Kind != RuntimeKind::Function)
      {
        return ExecutionStatus::UnsupportedExternalSignature;
      }
      if (Signature->Parameters.size() != Arguments.size() || Arguments.size() > std::numeric_limits<unsigned int>::max())
      {
        return ExecutionStatus::InvalidArguments;
      }
      for (const RuntimeValue &Argument : Arguments)
      {
        const StorageLayout *Type = Types.get(Argument.Type);
        if (!Argument.Initialized || !Type)
        {
          return ExecutionStatus::InvalidArguments;
        }
        if (Argument.kind() == RuntimeKind::String && Type->Kind != RuntimeKind::String)
        {
          return ExecutionStatus::TypeMismatch;
        }
      }
      return ExecutionStatus::Success;
    }

    ExecutionStatus preparePlan(const RuntimeTypeTable &Types, const RuntimeFunctionDescriptor &Function, NativeCallPlan &Plan)
    {
      Plan.Signature = Function.Signature;
      const StorageLayout &Signature = *Types.get(Function.Signature);
      const StorageLayout *Return = Types.get(Signature.ReturnType);
      ffi_type *NativeReturn = Return ? ffiType(*Return, FfiTypeUsage::Return) : nullptr;
      if (!NativeReturn)
      {
        return ExecutionStatus::UnsupportedExternalSignature;
      }
      Plan.Return = valueLayout(Types, *Return);
      Plan.NativeArguments.resize(Signature.Parameters.size());
      Plan.Arguments.reserve(Signature.Parameters.size());
      for (std::size_t Index = 0; Index < Signature.Parameters.size(); ++Index)
      {
        const StorageLayout *Argument = Types.get(Signature.Parameters[Index]);
        Plan.NativeArguments[Index] = Argument ? ffiType(*Argument, FfiTypeUsage::Argument) : nullptr;
        if (!Plan.NativeArguments[Index])
        {
          return ExecutionStatus::UnsupportedExternalSignature;
        }
        Plan.Arguments.push_back(valueLayout(Types, *Argument));
      }
      const ffi_status Status = ffi_prep_cif(&Plan.Interface, FFI_DEFAULT_ABI, static_cast<unsigned int>(Plan.Arguments.size()), NativeReturn, Plan.NativeArguments.data());
      if (Status != FFI_OK)
      {
        return Status == FFI_BAD_ABI ? ExecutionStatus::HostAbiMismatch : ExecutionStatus::UnsupportedExternalSignature;
      }
      return ExecutionStatus::Success;
    }

    template <typename T>
    T readNativeValue(const void *Storage)
    {
      T Result;
      std::memcpy(&Result, Storage, sizeof(Result));
      return Result;
    }

    RuntimeValueResult readResult(ExecutionMemoryManager &Memory, const FfiValueLayout &Type, const void *Storage, std::span<FfiArgument> Arguments)
    {
      switch (Type.Kind)
      {
      case RuntimeKind::Void:
        return {ExecutionStatus::Success, RuntimeValue::fromBits(0, Type.Type)};
      case RuntimeKind::Boolean:
        return {ExecutionStatus::Success, RuntimeValue::fromBits(readNativeValue<ffi_arg>(Storage) != 0, Type.Type)};
      case RuntimeKind::Integer:
      {
        std::uint64_t Bits = Type.BitWidth <= sizeof(ffi_arg) * CHAR_BIT ? static_cast<std::uint64_t>(readNativeValue<ffi_arg>(Storage)) : readNativeValue<std::uint64_t>(Storage);
        if (Type.BitWidth < 64)
        {
          Bits &= (std::uint64_t{1} << Type.BitWidth) - 1;
        }
        return {ExecutionStatus::Success, RuntimeValue::fromBits(Bits, Type.Type)};
      }
      case RuntimeKind::Float:
      {
        const std::uint64_t Bits = Type.BitWidth == 32 ? std::bit_cast<std::uint32_t>(readNativeValue<float>(Storage)) : std::bit_cast<std::uint64_t>(readNativeValue<double>(Storage));
        return {ExecutionStatus::Success, RuntimeValue::fromBits(Bits, Type.Type)};
      }
      case RuntimeKind::Pointer:
      {
        const ExecutionPointer Pointer = Memory.pointerFromAddress(readNativeValue<void *>(Storage));
        const ExecutionStatus Status = Pointer.status();
        if (Status != ExecutionStatus::Success)
        {
          return {Status};
        }
        if (Pointer.kind() == ExecutionPointer::Kind::Buffer)
        {
          for (FfiArgument &Argument : Arguments)
          {
            if (Argument.bufferRef() == Pointer.bufferRef())
            {
              Argument.promoteBuffer();
            }
          }
        }
        return {ExecutionStatus::Success, RuntimeValue::fromPointer(Pointer, Type.Type)};
      }
      default:
        return {ExecutionStatus::UnsupportedExternalSignature};
      }
    }

    RuntimeValueResult invokePlan(ExecutionMemoryManager &Memory, NativeCallPlan &Plan, std::span<const RuntimeValue> Values)
    {
      if (Values.size() != Plan.Arguments.size())
      {
        return {ExecutionStatus::InvalidArguments};
      }
      // Argument holders never move after addresses are exposed to libffi.
      std::vector<FfiArgument> Arguments(Values.size());
      std::vector<void *> Addresses(Values.size());
      for (std::size_t Index = 0; Index < Values.size(); ++Index)
      {
        const ExecutionStatus Status = Arguments[Index].prepare(Memory, Plan.Arguments[Index], Values[Index]);
        if (Status != ExecutionStatus::Success)
        {
          return {Status};
        }
        Addresses[Index] = Arguments[Index].address();
      }
      // libffi widens narrow integer results; floats retain their native width.
      alignas(std::max_align_t) std::array<std::byte, std::max({sizeof(ffi_arg), sizeof(std::uint64_t), sizeof(void *)})> ReturnStorage{};
      ffi_call(&Plan.Interface, Plan.Symbol, Plan.Return.Kind == RuntimeKind::Void ? nullptr : ReturnStorage.data(), Addresses.data());
      return readResult(Memory, Plan.Return, ReturnStorage.data(), Arguments);
    }
  } // namespace

  RuntimeValueResult callWithLibffi(ExecutionMemoryManager &Memory, const RuntimeTypeTable &Types, NativeSymbol Symbol, const RuntimeFunctionDescriptor &Function, std::span<const RuntimeValue> Arguments)
  {
    ExecutionStatus Status = validateFunction(Types, Function, Arguments);
    if (Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    if (!Symbol)
    {
      return {ExecutionStatus::SymbolNotFound};
    }
    NativeCallPlan Plan;
    Plan.Symbol = Symbol;
    Status = preparePlan(Types, Function, Plan);
    return Status == ExecutionStatus::Success ? invokePlan(Memory, Plan, Arguments) : RuntimeValueResult{Status};
  }

  struct NativeCallCache::Impl final
  {
      // Retained domain tokens isolate local IDs even after a table is destroyed
      // and its address is reused. Active calls retain their prepared interfaces.
      std::unordered_map<std::shared_ptr<const RuntimeTypeDomain>, std::unordered_map<FunctionId, std::shared_ptr<NativeCallPlan>>> Plans;
  };

  NativeCallCache::NativeCallCache()
      : Implementation(std::make_unique<Impl>())
  {
  }

  NativeCallCache::~NativeCallCache() = default;

  RuntimeValueResult NativeCallCache::invoke(ExecutionMemoryManager &Memory, const RuntimeTypeTable &Types, NativeSymbolCache &Symbols, const RuntimeFunctionDescriptor &Function, std::span<const RuntimeValue> Arguments)
  {
    ExecutionStatus Status = validateFunction(Types, Function, Arguments);
    if (Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    if (Function.Id == InvalidFunction)
    {
      return {ExecutionStatus::InvalidArguments};
    }
    std::shared_ptr<NativeCallPlan> Plan;
    const auto Table = Implementation->Plans.find(Types.domain());
    if (Table != Implementation->Plans.end())
    {
      const auto Found = Table->second.find(Function.Id);
      if (Found != Table->second.end())
      {
        Plan = Found->second;
      }
    }
    if (!Plan)
    {
      const NativeSymbol Symbol = Symbols.find(Function.Symbol);
      if (!Symbol)
      {
        return {ExecutionStatus::SymbolNotFound};
      }
      Plan = std::make_shared<NativeCallPlan>();
      Plan->Symbol = Symbol;
      Status = preparePlan(Types, Function, *Plan);
      if (Status != ExecutionStatus::Success)
      {
        return {Status};
      }
      Implementation->Plans[Types.domain()].emplace(Function.Id, Plan);
    }
    if (Plan->Signature != Function.Signature)
    {
      return {ExecutionStatus::TypeMismatch};
    }
    return invokePlan(Memory, *Plan, Arguments);
  }

  void NativeCallCache::clear() noexcept
  {
    Implementation->Plans.clear();
  }

  std::size_t NativeCallCache::size() const noexcept
  {
    std::size_t Result = 0;
    for (const auto &[Types, Plans] : Implementation->Plans)
    {
      Result += Plans.size();
    }
    return Result;
  }
} // namespace ink::execution
