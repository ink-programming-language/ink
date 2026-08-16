#include "ink/execution/execution_engine.h"

#include "function_executor.h"
#include "ink/ir/context.h"
#include "ink/ir/verifier.h"

#include <ffi.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    static_assert(sizeof(float) == sizeof(std::uint32_t) && std::numeric_limits<float>::is_iec559);
    static_assert(sizeof(double) == sizeof(std::uint64_t) && std::numeric_limits<double>::is_iec559);

    class NativeTypeCache
    {
      public:
        ffi_type *get(const ir::Type &TypeValue)
        {
          switch (TypeValue.kind())
          {
          case ir::TypeKind::Void:
            return &ffi_type_void;
          case ir::TypeKind::Bool:
          case ir::TypeKind::Byte:
            return &ffi_type_uint8;
          case ir::TypeKind::I32:
            return &ffi_type_sint32;
          case ir::TypeKind::F16:
            return nullptr;
          case ir::TypeKind::F32:
            return &ffi_type_float;
          case ir::TypeKind::F64:
            return &ffi_type_double;
          case ir::TypeKind::PointerSize:
            return sizeof(std::size_t) == sizeof(std::uint64_t) ? &ffi_type_uint64 : &ffi_type_uint32;
          case ir::TypeKind::BytePointer:
          case ir::TypeKind::ConstBytePointer:
            return &ffi_type_pointer;
          case ir::TypeKind::ByteSlice:
          case ir::TypeKind::ConstByteSlice:
            return nullptr;
          case ir::TypeKind::Struct:
            return getStruct(static_cast<const ir::StructType &>(TypeValue));
          case ir::TypeKind::Count:
            return nullptr;
          }
          return nullptr;
        }

        const std::vector<std::size_t> *offsets(const ir::StructType &TypeValue)
        {
          if (get(TypeValue) == nullptr)
          {
            return nullptr;
          }
          const auto Found = StructTypes.find(&TypeValue);
          return Found == StructTypes.end() ? nullptr : &Found->second->Offsets;
        }

      private:
        struct NativeStructType
        {
            ffi_type Type{};
            std::vector<ffi_type *> Elements;
            std::vector<std::size_t> Offsets;
        };

        ffi_type *getStruct(const ir::StructType &TypeValue)
        {
          const auto Existing = StructTypes.find(&TypeValue);
          if (Existing != StructTypes.end())
          {
            return &Existing->second->Type;
          }

          auto Result = std::make_unique<NativeStructType>();
          Result->Elements.reserve(TypeValue.fieldTypes().size() + 1);
          for (const ir::Type *FieldType : TypeValue.fieldTypes())
          {
            ffi_type *NativeFieldType = get(*FieldType);
            if (NativeFieldType == nullptr)
            {
              return nullptr;
            }
            Result->Elements.push_back(NativeFieldType);
          }
          Result->Elements.push_back(nullptr);
          Result->Type.size = 0;
          Result->Type.alignment = 0;
          Result->Type.type = FFI_TYPE_STRUCT;
          Result->Type.elements = Result->Elements.data();
          Result->Offsets.resize(TypeValue.fieldTypes().size());
          if (ffi_get_struct_offsets(FFI_DEFAULT_ABI, &Result->Type, Result->Offsets.data()) != FFI_OK)
          {
            return nullptr;
          }
          ffi_type *NativeType = &Result->Type;
          StructTypes.emplace(&TypeValue, std::move(Result));
          return NativeType;
        }

        std::unordered_map<const ir::StructType *, std::unique_ptr<NativeStructType>> StructTypes;
    };

    class NativeCallSlot
    {
      public:
        explicit NativeCallSlot(std::size_t Size) : Storage((Size + sizeof(std::max_align_t) - 1) / sizeof(std::max_align_t))
        {
        }

        void *data() noexcept
        {
          return Storage.data();
        }

        const void *data() const noexcept
        {
          return Storage.data();
        }

      private:
        std::vector<std::max_align_t> Storage;
    };

    bool storeNativeValue(const RuntimeValue &Value, NativeTypeCache &Types, void *Destination)
    {
      const std::optional<std::uint64_t> Integer = Value.integer();
      const std::optional<std::uint64_t> FloatingPointBits = Value.floatingPointBits();
      switch (Value.type().kind())
      {
      case ir::TypeKind::Bool:
      case ir::TypeKind::Byte:
      {
        if (!Integer.has_value())
        {
          return false;
        }
        const std::uint8_t NativeValue = static_cast<std::uint8_t>(*Integer);
        std::memcpy(Destination, &NativeValue, sizeof(NativeValue));
        return true;
      }
      case ir::TypeKind::I32:
      {
        if (!Integer.has_value())
        {
          return false;
        }
        const std::int32_t NativeValue = static_cast<std::int32_t>(*Integer);
        std::memcpy(Destination, &NativeValue, sizeof(NativeValue));
        return true;
      }
      case ir::TypeKind::F16:
        return false;
      case ir::TypeKind::F32:
      {
        if (!FloatingPointBits.has_value() || !isValidRuntimeFloatingPointValue(Value.type(), *FloatingPointBits))
        {
          return false;
        }
        const std::uint32_t NativeBits = static_cast<std::uint32_t>(*FloatingPointBits);
        std::memcpy(Destination, &NativeBits, sizeof(NativeBits));
        return true;
      }
      case ir::TypeKind::F64:
      {
        if (!FloatingPointBits.has_value() || !isValidRuntimeFloatingPointValue(Value.type(), *FloatingPointBits))
        {
          return false;
        }
        std::memcpy(Destination, &*FloatingPointBits, sizeof(*FloatingPointBits));
        return true;
      }
      case ir::TypeKind::PointerSize:
      {
        if (!Integer.has_value())
        {
          return false;
        }
        const std::size_t NativeValue = static_cast<std::size_t>(*Integer);
        std::memcpy(Destination, &NativeValue, sizeof(NativeValue));
        return true;
      }
      case ir::TypeKind::BytePointer:
      {
        if (!Value.memoryAlive())
        {
          return false;
        }
        void *NativeValue = Value.mutablePointer();
        const std::optional<std::size_t> Length = Value.byteLength();
        const std::optional<std::uint64_t> Offset = runtimePointerByteOffset(Value);
        if (Length.has_value() && (!Offset.has_value() || *Offset > *Length || (NativeValue == nullptr && *Length != 0)))
        {
          return false;
        }
        std::memcpy(Destination, &NativeValue, sizeof(NativeValue));
        return true;
      }
      case ir::TypeKind::ConstBytePointer:
      {
        if (!Value.memoryAlive())
        {
          return false;
        }
        const void *NativeValue = Value.pointer();
        const std::optional<std::size_t> Length = Value.byteLength();
        const std::optional<std::uint64_t> Offset = runtimePointerByteOffset(Value);
        if (Length.has_value() && (!Offset.has_value() || *Offset > *Length || (NativeValue == nullptr && *Length != 0)))
        {
          return false;
        }
        std::memcpy(Destination, &NativeValue, sizeof(NativeValue));
        return true;
      }
      case ir::TypeKind::Struct:
      {
        const ir::StructType &Struct = static_cast<const ir::StructType &>(Value.type());
        const std::vector<std::size_t> *Offsets = Types.offsets(Struct);
        if (Offsets == nullptr || Value.fieldCount() != Struct.fieldTypes().size())
        {
          return false;
        }
        auto *Bytes = static_cast<std::byte *>(Destination);
        for (std::size_t FieldIndex = 0; FieldIndex < Struct.fieldTypes().size(); ++FieldIndex)
        {
          const RuntimeValue *Field = Value.field(FieldIndex);
          if (Field == nullptr || &Field->type() != Struct.fieldTypes()[FieldIndex] || !storeNativeValue(*Field, Types, Bytes + (*Offsets)[FieldIndex]))
          {
            return false;
          }
        }
        return true;
      }
      case ir::TypeKind::Void:
      case ir::TypeKind::ByteSlice:
      case ir::TypeKind::ConstByteSlice:
      case ir::TypeKind::Count:
        return false;
      }
      return false;
    }

    RuntimeValueRef loadNativeValue(const ir::Type &TypeValue, NativeTypeCache &Types, const void *Source, RuntimeValueArena &Values)
    {
      switch (TypeValue.kind())
      {
      case ir::TypeKind::Void:
        return Values.voidValue(TypeValue);
      case ir::TypeKind::Bool:
      case ir::TypeKind::Byte:
      {
        std::uint8_t NativeValue = 0;
        std::memcpy(&NativeValue, Source, sizeof(NativeValue));
        return Values.integerValue(TypeValue, NativeValue);
      }
      case ir::TypeKind::I32:
      {
        std::int32_t NativeValue = 0;
        std::memcpy(&NativeValue, Source, sizeof(NativeValue));
        return Values.integerValue(TypeValue, static_cast<std::uint64_t>(static_cast<std::int64_t>(NativeValue)));
      }
      case ir::TypeKind::F16:
        return nullptr;
      case ir::TypeKind::F32:
      {
        std::uint32_t NativeBits = 0;
        std::memcpy(&NativeBits, Source, sizeof(NativeBits));
        return Values.floatingPointValue(TypeValue, NativeBits);
      }
      case ir::TypeKind::F64:
      {
        std::uint64_t NativeBits = 0;
        std::memcpy(&NativeBits, Source, sizeof(NativeBits));
        return Values.floatingPointValue(TypeValue, NativeBits);
      }
      case ir::TypeKind::PointerSize:
      {
        std::size_t NativeValue = 0;
        std::memcpy(&NativeValue, Source, sizeof(NativeValue));
        return Values.integerValue(TypeValue, NativeValue);
      }
      case ir::TypeKind::BytePointer:
      {
        void *NativeValue = nullptr;
        std::memcpy(&NativeValue, Source, sizeof(NativeValue));
        return Values.mutablePointerValue(TypeValue, NativeValue);
      }
      case ir::TypeKind::ConstBytePointer:
      {
        const void *NativeValue = nullptr;
        std::memcpy(&NativeValue, Source, sizeof(NativeValue));
        return Values.pointerValue(TypeValue, NativeValue);
      }
      case ir::TypeKind::ByteSlice:
      case ir::TypeKind::ConstByteSlice:
        return nullptr;
      case ir::TypeKind::Struct:
      {
        const ir::StructType &Struct = static_cast<const ir::StructType &>(TypeValue);
        const std::vector<std::size_t> *Offsets = Types.offsets(Struct);
        if (Offsets == nullptr)
        {
          return nullptr;
        }
        const auto *Bytes = static_cast<const std::byte *>(Source);
        std::vector<RuntimeValueRef> Fields;
        Fields.reserve(Struct.fieldTypes().size());
        for (std::size_t FieldIndex = 0; FieldIndex < Struct.fieldTypes().size(); ++FieldIndex)
        {
          RuntimeValueRef Field = loadNativeValue(*Struct.fieldTypes()[FieldIndex], Types, Bytes + (*Offsets)[FieldIndex], Values);
          if (Field == nullptr)
          {
            return nullptr;
          }
          Fields.push_back(Field);
        }
        return Values.aggregateValue(Struct, std::move(Fields));
      }
      case ir::TypeKind::Count:
        return nullptr;
      }
      return nullptr;
    }

    bool hasValidRuntimeShape(const RuntimeValue &Value, const ir::Type &ExpectedType, const core::TargetContext &Target, std::unordered_set<const RuntimeValue *> &ValidatedValues, std::unordered_set<const RuntimeValue *> &ActiveValues)
    {
      if (&Value.type() != &ExpectedType)
      {
        return false;
      }
      if (ValidatedValues.find(&Value) != ValidatedValues.end())
      {
        return true;
      }
      bool IsValid = false;
      switch (ExpectedType.kind())
      {
      case ir::TypeKind::Void:
        IsValid = Value.kind() == RuntimeValueKind::Void;
        break;
      case ir::TypeKind::Bool:
      case ir::TypeKind::Byte:
      case ir::TypeKind::I32:
      case ir::TypeKind::PointerSize:
      {
        const std::optional<std::uint64_t> Integer = Value.integer();
        IsValid = Value.kind() == RuntimeValueKind::Integer && Integer.has_value() && isValidRuntimeIntegerValue(ExpectedType, *Integer, Target);
        break;
      }
      case ir::TypeKind::F16:
      case ir::TypeKind::F32:
      case ir::TypeKind::F64:
      {
        const std::optional<std::uint64_t> Bits = Value.floatingPointBits();
        IsValid = Value.kind() == RuntimeValueKind::FloatingPoint && Bits.has_value() && isValidRuntimeFloatingPointValue(ExpectedType, *Bits);
        break;
      }
      case ir::TypeKind::BytePointer:
      case ir::TypeKind::ConstBytePointer:
      {
        const std::optional<std::size_t> Length = Value.byteLength();
        if (Value.kind() != RuntimeValueKind::Pointer || !Value.memoryAlive())
        {
          IsValid = false;
        }
        else if (!Length.has_value())
        {
          IsValid = Target.isNativeAbiCompatible() || Value.pointer() == nullptr;
        }
        else
        {
          const std::optional<std::uint64_t> Offset = runtimePointerByteOffset(Value);
          const void *Address = ExpectedType.kind() == ir::TypeKind::BytePointer ? Value.mutablePointer() : Value.pointer();
          IsValid = Offset.has_value() && *Offset <= *Length && *Length <= Target.maximumPointerSizeValue() && (*Length == 0 || Address != nullptr);
        }
        break;
      }
      case ir::TypeKind::ByteSlice:
      {
        const std::optional<std::size_t> Length = Value.byteLength();
        IsValid = Value.kind() == RuntimeValueKind::ByteSlice && Length.has_value() && *Length <= Target.maximumPointerSizeValue() && Value.memoryAlive() && (*Length == 0 || Value.mutablePointer() != nullptr);
        break;
      }
      case ir::TypeKind::ConstByteSlice:
      {
        const std::optional<std::size_t> Length = Value.byteLength();
        IsValid = Value.kind() == RuntimeValueKind::ByteSlice && Length.has_value() && *Length <= Target.maximumPointerSizeValue() && Value.memoryAlive() && (*Length == 0 || Value.pointer() != nullptr);
        break;
      }
      case ir::TypeKind::Struct:
        break;
      case ir::TypeKind::Count:
        return false;
      }
      if (ExpectedType.kind() != ir::TypeKind::Struct)
      {
        return IsValid;
      }
      if (Value.kind() != RuntimeValueKind::Aggregate)
      {
        return false;
      }
      const ir::StructType &Struct = static_cast<const ir::StructType &>(ExpectedType);
      if (Value.fieldCount() != Struct.fieldTypes().size() || !ActiveValues.insert(&Value).second)
      {
        return false;
      }
      for (std::size_t FieldIndex = 0; FieldIndex < Struct.fieldTypes().size(); ++FieldIndex)
      {
        const RuntimeValue *Field = Value.field(FieldIndex);
        if (Field == nullptr || !hasValidRuntimeShape(*Field, *Struct.fieldTypes()[FieldIndex], Target, ValidatedValues, ActiveValues))
        {
          ActiveValues.erase(&Value);
          return false;
        }
      }
      ActiveValues.erase(&Value);
      ValidatedValues.insert(&Value);
      return true;
    }
  } // namespace

  class ExecutionEngine::Impl : public ExternalFunctionInvoker
  {
    public:
      Impl(ExecutionContext &Context, const ir::Module &ModuleValue) : Context(Context), ModuleValue(ModuleValue), PreparedFunctions(ModuleValue.Functions.size())
      {
      }

      InitializationResult initialize()
      {
        InitializationResult Result;
        if (ModuleValue.context().compilationContext().targetContext() != Context.compilationContext().targetContext())
        {
          addFailure<core::DiagnosticKind::ExecutionTargetMismatch>(Result.Diagnostics);
          return Result;
        }
        if (Initialized)
        {
          Result.Succeeded = true;
          return Result;
        }

        ir::IRContext IRContext(Context.compilationContext());
        ir::VerificationResult Verification = ir::verify(IRContext, ModuleValue);
        if (!Verification.succeeded())
        {
          Result.Diagnostics = Verification.diagnostics();
          return Result;
        }
        for (std::size_t FunctionIndex = 0; FunctionIndex < ModuleValue.Functions.size(); ++FunctionIndex)
        {
          const ir::Function &FunctionValue = ModuleValue.Functions[FunctionIndex];
          if (FunctionValue.Kind != ir::FunctionKind::External)
          {
            continue;
          }

          auto Prepared = std::make_unique<PreparedFunction>();
          Prepared->ArgumentTypes.reserve(FunctionValue.ParameterTypes.size());
          bool HasUnsupportedType = false;
          for (const ir::Type *ParameterType : FunctionValue.ParameterTypes)
          {
            ffi_type *NativeParameterType = NativeTypes.get(*ParameterType);
            HasUnsupportedType = HasUnsupportedType || NativeParameterType == nullptr;
            Prepared->ArgumentTypes.push_back(NativeParameterType);
          }
          Prepared->ResultType = NativeTypes.get(*FunctionValue.ResultType);
          HasUnsupportedType = HasUnsupportedType || Prepared->ResultType == nullptr;
          if (HasUnsupportedType || ffi_prep_cif(&Prepared->Interface, FFI_DEFAULT_ABI, static_cast<unsigned int>(Prepared->ArgumentTypes.size()), Prepared->ResultType, Prepared->ArgumentTypes.data()) != FFI_OK)
          {
            addFailure<core::DiagnosticKind::ExternalFunctionSignatureUnsupported>(Result.Diagnostics, FunctionValue.Name);
            continue;
          }
          PreparedFunctions[FunctionIndex] = std::move(Prepared);
        }

        if (!Result.Diagnostics.empty())
        {
          PreparedFunctions.clear();
          PreparedFunctions.resize(ModuleValue.Functions.size());
          return Result;
        }
        Initialized = true;
        Result.Succeeded = true;
        return Result;
      }

      ExecutionResult execute(std::string_view EntryName, const std::vector<RuntimeValueRef> &Arguments)
      {
        ExecutionResult Result;
        InitializationResult Initialization = initialize();
        if (!Initialization.succeeded())
        {
          Result.Diagnostics = Initialization.diagnostics();
          return Result;
        }

        std::size_t EntryIndex = ModuleValue.Functions.size();
        for (std::size_t FunctionIndex = 0; FunctionIndex < ModuleValue.Functions.size(); ++FunctionIndex)
        {
          if (ModuleValue.Functions[FunctionIndex].Name == EntryName)
          {
            EntryIndex = FunctionIndex;
            break;
          }
        }
        if (EntryIndex == ModuleValue.Functions.size())
        {
          addFailure<core::DiagnosticKind::EntryFunctionNotFound>(Result.Diagnostics, EntryName);
          return Result;
        }
        const ir::Function &Entry = ModuleValue.Functions[EntryIndex];
        if (Entry.Kind != ir::FunctionKind::Definition)
        {
          addFailure<core::DiagnosticKind::EntryFunctionMustBeDefined>(Result.Diagnostics, Entry.Name);
          return Result;
        }
        if (Entry.ParameterTypes.size() != Arguments.size())
        {
          addFailure<core::DiagnosticKind::EntryArgumentCountMismatch>(Result.Diagnostics, Entry.Name, Entry.ParameterTypes.size(), Arguments.size());
          return Result;
        }
        std::unordered_set<const RuntimeValue *> ValidatedValues;
        std::unordered_set<const RuntimeValue *> ActiveValues;
        for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments.size(); ++ArgumentIndex)
        {
          if (Arguments[ArgumentIndex] == nullptr || !hasValidRuntimeShape(*Arguments[ArgumentIndex], *Entry.ParameterTypes[ArgumentIndex], Context.compilationContext().targetContext(), ValidatedValues, ActiveValues))
          {
            addFailure<core::DiagnosticKind::EntryArgumentInvalid>(Result.Diagnostics, Entry.Name, ArgumentIndex);
            return Result;
          }
        }

        FunctionExecutor Executor(Context, ModuleValue, *this, Result.Diagnostics);
        RuntimeValueRef ReturnValue = nullptr;
        if (!Executor.execute(EntryIndex, Arguments, ReturnValue))
        {
          return Result;
        }
        if (ReturnValue == nullptr)
        {
          addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>(Result.Diagnostics);
          return Result;
        }
        std::shared_ptr<RuntimeValueArena> ResultValues = std::make_shared<RuntimeValueArena>(Context.compilationContext().targetContext());
        RuntimeValueRef StableReturnValue = ResultValues->clone(*ReturnValue);
        if (StableReturnValue == nullptr)
        {
          addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>(Result.Diagnostics);
          return Result;
        }
        Result.ValueArena = std::move(ResultValues);
        Result.ReturnValue = StableReturnValue;
        return Result;
      }

    private:
      struct PreparedFunction
      {
          ffi_cif Interface{};
          std::vector<ffi_type *> ArgumentTypes;
          ffi_type *ResultType = nullptr;
          NativeFunctionAddress Address = nullptr;
      };

      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      void addFailure(std::vector<core::Diagnostic> &Diagnostics, ArgumentTypes &&...Arguments)
      {
        core::Diagnostic DiagnosticEntry = core::makeDiagnostic<Kind>({}, std::forward<ArgumentTypes>(Arguments)...);
        Context.diagnosticEngine().report(DiagnosticEntry);
        Diagnostics.push_back(std::move(DiagnosticEntry));
      }

      bool invokeExternal(std::size_t FunctionIndex, const std::vector<RuntimeValueRef> &Arguments, RuntimeValueArena &Values, RuntimeValueRef &Result, std::vector<core::Diagnostic> &Diagnostics) override
      {
        const ir::Function &FunctionValue = ModuleValue.Functions[FunctionIndex];
        if (!Context.compilationContext().targetContext().isNativeAbiCompatible())
        {
          addFailure<core::DiagnosticKind::ExternalFunctionTargetUnsupported>(Diagnostics, FunctionValue.Name);
          return false;
        }
        const std::unique_ptr<PreparedFunction> &Prepared = PreparedFunctions[FunctionIndex];
        if (!Prepared)
        {
          addFailure<core::DiagnosticKind::ExternalFunctionNotPrepared>(Diagnostics, FunctionValue.Name);
          return false;
        }

        if (Prepared->Address == nullptr)
        {
          Prepared->Address = Context.nativeSymbols().findAddress(FunctionValue.Name);
          if (Prepared->Address == nullptr)
          {
            addFailure<core::DiagnosticKind::ExternalFunctionNotFound>(Diagnostics, FunctionValue.Name);
            return false;
          }
        }

        std::vector<NativeCallSlot> ArgumentStorage;
        ArgumentStorage.reserve(Arguments.size());
        std::vector<void *> NativeArguments;
        NativeArguments.reserve(Arguments.size());
        for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments.size(); ++ArgumentIndex)
        {
          ffi_type *ArgumentType = Prepared->ArgumentTypes[ArgumentIndex];
          if (ArgumentType->alignment > alignof(std::max_align_t))
          {
            addFailure<core::DiagnosticKind::NativeArgumentOveraligned>(Diagnostics, FunctionValue.Name, ArgumentIndex, ArgumentType->alignment, alignof(std::max_align_t));
            return false;
          }
          ArgumentStorage.emplace_back(ArgumentType->size);
          void *ArgumentAddress = ArgumentStorage.back().data();
          if (Arguments[ArgumentIndex] == nullptr || !storeNativeValue(*Arguments[ArgumentIndex], NativeTypes, ArgumentAddress))
          {
            addFailure<core::DiagnosticKind::NativeArgumentMarshalFailed>(Diagnostics, FunctionValue.Name, ArgumentIndex);
            return false;
          }
          NativeArguments.push_back(ArgumentAddress);
        }

        std::optional<NativeCallSlot> ReturnStorage;
        void *ReturnAddress = nullptr;
        if (FunctionValue.ResultType->kind() != ir::TypeKind::Void)
        {
          if (Prepared->ResultType->alignment > alignof(std::max_align_t))
          {
            addFailure<core::DiagnosticKind::NativeResultOveraligned>(Diagnostics, FunctionValue.Name, Prepared->ResultType->alignment, alignof(std::max_align_t));
            return false;
          }
          ReturnStorage.emplace(Prepared->ResultType->size);
          ReturnAddress = ReturnStorage->data();
        }
        ffi_call(&Prepared->Interface, FFI_FN(Prepared->Address), ReturnAddress, NativeArguments.data());
        Result = loadNativeValue(*FunctionValue.ResultType, NativeTypes, ReturnStorage ? ReturnStorage->data() : nullptr, Values);
        if (Result == nullptr)
        {
          addFailure<core::DiagnosticKind::NativeResultUnmarshalFailed>(Diagnostics, FunctionValue.Name);
          return false;
        }
        return true;
      }

      ExecutionContext &Context;
      const ir::Module &ModuleValue;
      NativeTypeCache NativeTypes;
      std::vector<std::unique_ptr<PreparedFunction>> PreparedFunctions;
      bool Initialized = false;
  };

  bool InitializationResult::succeeded() const noexcept
  {
    return Succeeded && Diagnostics.empty();
  }

  const std::vector<core::Diagnostic> &InitializationResult::diagnostics() const noexcept
  {
    return Diagnostics;
  }

  ExecutionResult::ExecutionResult(ExecutionResult &&Other) noexcept : ValueArena(std::move(Other.ValueArena)), ReturnValue(Other.ReturnValue), Diagnostics(std::move(Other.Diagnostics))
  {
    Other.ReturnValue = nullptr;
  }

  ExecutionResult &ExecutionResult::operator=(ExecutionResult &&Other) noexcept
  {
    if (this != &Other)
    {
      ValueArena = std::move(Other.ValueArena);
      ReturnValue = Other.ReturnValue;
      Diagnostics = std::move(Other.Diagnostics);
      Other.ReturnValue = nullptr;
    }
    return *this;
  }

  bool ExecutionResult::succeeded() const noexcept
  {
    return ReturnValue != nullptr && Diagnostics.empty();
  }

  RuntimeValueRef ExecutionResult::returnValue() const & noexcept
  {
    return ReturnValue;
  }

  const std::vector<core::Diagnostic> &ExecutionResult::diagnostics() const noexcept
  {
    return Diagnostics;
  }

  ExecutionEngine::ExecutionEngine(ExecutionContext &Context, const ir::Module &ModuleValue) : Implementation(std::make_unique<Impl>(Context, ModuleValue))
  {
  }

  ExecutionEngine::~ExecutionEngine() = default;

  InitializationResult ExecutionEngine::initialize()
  {
    return Implementation->initialize();
  }

  ExecutionResult ExecutionEngine::execute(std::string_view EntryName, const std::vector<RuntimeValueRef> &Arguments)
  {
    return Implementation->execute(EntryName, Arguments);
  }
} // namespace ink::execution
