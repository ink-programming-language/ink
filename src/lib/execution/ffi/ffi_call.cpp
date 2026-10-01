#include "ink/execution/ffi/ffi_call.h"

#include "ink/execution/ffi/ffi_argument.h"
#include "ink/execution/ffi/ffi_type.h"
#include "ink/ir/context.h"
#include "ink/ir/function/function.h"

#include <ffi.h>

#include <algorithm>
#include <array>
#include <bit>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

namespace ink::execution
{
  namespace
  {
    ExecutionStatus validateCall(ExecutionHeap &Heap, NativeSymbol Symbol, const ir::Function &Function, std::span<const ExecutionValueRef> Arguments)
    {
      ir::IRContext &Context = Heap.context();
      if (&Function.context() != &Context)
      {
        return ExecutionStatus::ForeignContext;
      }
      if (!Context.compilationContext().targetContext().isNativeAbiCompatible())
      {
        return ExecutionStatus::HostAbiMismatch;
      }
      if (Function.languageLinkage() != ir::LanguageLinkage::C || Function.callingConvention() != ir::CallingConvention::C)
      {
        return ExecutionStatus::UnsupportedExternalSignature;
      }
      if (!Symbol)
      {
        return ExecutionStatus::SymbolNotFound;
      }
      if (Function.parameters().size() != Arguments.size() || Arguments.size() > std::numeric_limits<unsigned int>::max())
      {
        return ExecutionStatus::InvalidArguments;
      }
      for (const ExecutionValueRef &Argument : Arguments)
      {
        if (!Argument.type())
        {
          return ExecutionStatus::InvalidArguments;
        }
        if (&Argument.type()->context() != &Context)
        {
          return ExecutionStatus::ForeignContext;
        }
      }
      return ExecutionStatus::Success;
    }

    ExecutionStatus prepareSignature(const ir::Function &Function, ffi_cif &Interface, std::vector<ffi_type *> &ArgumentTypes)
    {
      ffi_type *ReturnType = ffiType(Function.functionType().returnType(), FfiTypeUsage::Return);
      if (!ReturnType)
      {
        return ExecutionStatus::UnsupportedExternalSignature;
      }
      const auto &Parameters = Function.parameters();
      ArgumentTypes.resize(Parameters.size());
      for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
      {
        if (Parameters[Index]->parameterKind() == ir::ParameterKind::Variadic)
        {
          return ExecutionStatus::UnsupportedExternalSignature;
        }
        ArgumentTypes[Index] = ffiType(Parameters[Index]->type(), FfiTypeUsage::Argument);
        if (!ArgumentTypes[Index])
        {
          return ExecutionStatus::UnsupportedExternalSignature;
        }
      }
      const ffi_status Status = ffi_prep_cif(&Interface, FFI_DEFAULT_ABI, static_cast<unsigned int>(Parameters.size()), ReturnType, ArgumentTypes.data());
      if (Status != FFI_OK)
      {
        return Status == FFI_BAD_ABI ? ExecutionStatus::HostAbiMismatch : ExecutionStatus::UnsupportedExternalSignature;
      }
      return ExecutionStatus::Success;
    }

    ExecutionStatus prepareArguments(ExecutionHeap &Heap, const ir::Function &Function, std::span<const ExecutionValueRef> Values, std::span<FfiArgument> Arguments, std::span<void *> Addresses)
    {
      for (std::size_t Index = 0; Index < Values.size(); ++Index)
      {
        const ExecutionStatus Status = Arguments[Index].prepare(Heap, Function.parameters()[Index]->type(), Values[Index]);
        if (Status != ExecutionStatus::Success)
        {
          return Status;
        }
        Addresses[Index] = Arguments[Index].address();
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

    ExecutionValueResult allocatedResult(ExecutionHeap &Heap, ExecutionValueRef Value)
    {
      return Value.valid() ? ExecutionValueResult{ExecutionStatus::Success, Value} : ExecutionValueResult{Heap.lastStatus()};
    }

    ExecutionValueResult readBufferResult(ExecutionHeap &Heap, const ir::Type &Type, FfiArgument &Argument, std::size_t Offset)
    {
      const ExecutionStatus Status = Argument.bufferRef().status();
      if (Status != ExecutionStatus::Success)
      {
        return {Status};
      }
      ExecutionValueResult Result = allocatedResult(Heap, Heap.pointer(Type, ExecutionPointer::fromBuffer(Argument.bufferRef(), Offset)));
      if (Result)
      {
        Argument.promoteBuffer();
      }
      return Result;
    }

    ExecutionValueResult readPointerResult(ExecutionHeap &Heap, const ir::Type &Type, const void *Storage, std::span<FfiArgument> Arguments)
    {
      void *Pointer = readNativeValue<void *>(Storage);
      if (Pointer)
      {
        const std::uintptr_t Address = reinterpret_cast<std::uintptr_t>(Pointer);
        FfiArgument *OnePastArgument = nullptr;
        for (FfiArgument &Argument : Arguments)
        {
          const ExecutionBuffer *Buffer = Argument.buffer();
          if (!Buffer)
          {
            continue;
          }
          const std::uintptr_t Begin = reinterpret_cast<std::uintptr_t>(Buffer->data());
          // Numeric subtraction avoids comparisons between unrelated pointers.
          // Prefer an allocation's interior over an adjacent one-past address.
          if (Address >= Begin && Address - Begin <= Buffer->size())
          {
            if (Address - Begin == Buffer->size())
            {
              OnePastArgument = &Argument;
              continue;
            }
            return readBufferResult(Heap, Type, Argument, static_cast<std::size_t>(Address - Begin));
          }
        }
        if (OnePastArgument)
        {
          return readBufferResult(Heap, Type, *OnePastArgument, OnePastArgument->buffer()->size());
        }
      }
      return allocatedResult(Heap, Heap.pointer(Type, ExecutionPointer::fromNative(Pointer)));
    }

    ExecutionValueResult readResult(ExecutionHeap &Heap, const ir::Type &Type, const void *Storage, std::span<FfiArgument> Arguments)
    {
      switch (Type.typeKind())
      {
      case ir::TypeKind::Void:
        return allocatedResult(Heap, Heap.voidValue(Type));
      case ir::TypeKind::Bool:
        return allocatedResult(Heap, Heap.boolean(Type, readNativeValue<ffi_arg>(Storage) != 0));
      case ir::TypeKind::Integer:
      {
        const auto &Integer = static_cast<const ir::IntegerType &>(Type);
        const std::uint32_t Width = Integer.bitWidth();
        std::uint64_t Bits = Width <= sizeof(ffi_arg) * CHAR_BIT ? static_cast<std::uint64_t>(readNativeValue<ffi_arg>(Storage)) : readNativeValue<std::uint64_t>(Storage);
        if (Width < 64)
        {
          Bits &= (std::uint64_t{1} << Width) - 1;
        }
        return allocatedResult(Heap, Heap.integer(Type, ExecutionInteger(Width, Bits)));
      }
      case ir::TypeKind::Float:
      {
        const auto &Float = static_cast<const ir::FloatType &>(Type);
        const std::uint64_t Bits = Float.bitWidth() == 32 ? std::bit_cast<std::uint32_t>(readNativeValue<float>(Storage)) : std::bit_cast<std::uint64_t>(readNativeValue<double>(Storage));
        return allocatedResult(Heap, Heap.floating(Type, ir::FloatBits(Float.bitWidth(), Bits)));
      }
      case ir::TypeKind::Pointer:
        return readPointerResult(Heap, Type, Storage, Arguments);
      default:
        return {ExecutionStatus::UnsupportedExternalSignature};
      }
    }

    ExecutionValueResult invokeFunction(ExecutionHeap &Heap, NativeSymbol Symbol, const ir::Type &ReturnType, ffi_cif &Interface, std::span<void *> Addresses, std::span<FfiArgument> Arguments)
    {
      // libffi widens narrow integral results to ffi_arg; floating results retain
      // their declared native width. Decode numeric values before extracting bits.
      alignas(std::max_align_t) std::array<std::byte, std::max({sizeof(ffi_arg), sizeof(std::uint64_t), sizeof(void *)})> ReturnStorage{};
      ffi_call(&Interface, Symbol, ReturnType.typeKind() == ir::TypeKind::Void ? nullptr : ReturnStorage.data(), Addresses.data());
      return readResult(Heap, ReturnType, ReturnStorage.data(), Arguments);
    }
  } // namespace

  ExecutionValueResult callWithLibffi(ExecutionHeap &Heap, NativeSymbol Symbol, const ir::Function &Function, std::span<const ExecutionValueRef> Arguments)
  {
    ExecutionStatus Status = validateCall(Heap, Symbol, Function, Arguments);
    if (Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    ffi_cif Interface{};
    std::vector<ffi_type *> ArgumentTypes;
    Status = prepareSignature(Function, Interface, ArgumentTypes);
    if (Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    // Finish sizing storage before exposing addresses to libffi. FfiArgument
    // cannot move or copy because those addresses refer to its own native slots.
    std::vector<FfiArgument> NativeArguments(Arguments.size());
    std::vector<void *> Addresses(Arguments.size());
    Status = prepareArguments(Heap, Function, Arguments, NativeArguments, Addresses);
    if (Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    return invokeFunction(Heap, Symbol, Function.functionType().returnType(), Interface, Addresses, NativeArguments);
  }
} // namespace ink::execution
