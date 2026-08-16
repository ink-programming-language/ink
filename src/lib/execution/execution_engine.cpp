#include "ink/execution/execution_engine.h"

#include "execution_frame.h"
#include "ink/ir/context.h"
#include "ink/ir/verifier.h"
#include "runtime.h"

#include <ffi.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    constexpr std::size_t MaximumCallDepth = 256;

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
          case ir::TypeKind::PointerSize:
            return sizeof(std::size_t) == sizeof(std::uint64_t) ? &ffi_type_uint64 : &ffi_type_uint32;
          case ir::TypeKind::ConstBytePointer:
            return &ffi_type_pointer;
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
      case ir::TypeKind::ConstBytePointer:
      {
        const void *NativeValue = Value.pointer();
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
      case ir::TypeKind::Count:
        return false;
      }
      return false;
    }

    std::optional<RuntimeValue> loadNativeValue(const ir::Type &TypeValue, NativeTypeCache &Types, const void *Source)
    {
      switch (TypeValue.kind())
      {
      case ir::TypeKind::Void:
        return RuntimeValue::voidValue(TypeValue);
      case ir::TypeKind::Bool:
      case ir::TypeKind::Byte:
      {
        std::uint8_t NativeValue = 0;
        std::memcpy(&NativeValue, Source, sizeof(NativeValue));
        return RuntimeValue::integerValue(TypeValue, NativeValue);
      }
      case ir::TypeKind::I32:
      {
        std::int32_t NativeValue = 0;
        std::memcpy(&NativeValue, Source, sizeof(NativeValue));
        return RuntimeValue::integerValue(TypeValue, static_cast<std::uint64_t>(static_cast<std::int64_t>(NativeValue)));
      }
      case ir::TypeKind::PointerSize:
      {
        std::size_t NativeValue = 0;
        std::memcpy(&NativeValue, Source, sizeof(NativeValue));
        return RuntimeValue::integerValue(TypeValue, NativeValue);
      }
      case ir::TypeKind::ConstBytePointer:
      {
        const void *NativeValue = nullptr;
        std::memcpy(&NativeValue, Source, sizeof(NativeValue));
        return RuntimeValue::pointerValue(TypeValue, NativeValue);
      }
      case ir::TypeKind::Struct:
      {
        const ir::StructType &Struct = static_cast<const ir::StructType &>(TypeValue);
        const std::vector<std::size_t> *Offsets = Types.offsets(Struct);
        if (Offsets == nullptr)
        {
          return std::nullopt;
        }
        const auto *Bytes = static_cast<const std::byte *>(Source);
        std::vector<RuntimeValue> Fields;
        Fields.reserve(Struct.fieldTypes().size());
        for (std::size_t FieldIndex = 0; FieldIndex < Struct.fieldTypes().size(); ++FieldIndex)
        {
          std::optional<RuntimeValue> Field = loadNativeValue(*Struct.fieldTypes()[FieldIndex], Types, Bytes + (*Offsets)[FieldIndex]);
          if (!Field.has_value())
          {
            return std::nullopt;
          }
          Fields.push_back(*Field);
        }
        return RuntimeValue::aggregateValue(Struct, std::move(Fields));
      }
      case ir::TypeKind::Count:
        return std::nullopt;
      }
      return std::nullopt;
    }

    bool hasValidRuntimeShape(const RuntimeValue &Value, const ir::Type &ExpectedType)
    {
      if (&Value.type() != &ExpectedType)
      {
        return false;
      }
      if (ExpectedType.kind() != ir::TypeKind::Struct)
      {
        return true;
      }
      const ir::StructType &Struct = static_cast<const ir::StructType &>(ExpectedType);
      if (Value.fieldCount() != Struct.fieldTypes().size())
      {
        return false;
      }
      for (std::size_t FieldIndex = 0; FieldIndex < Struct.fieldTypes().size(); ++FieldIndex)
      {
        const RuntimeValue *Field = Value.field(FieldIndex);
        if (Field == nullptr || !hasValidRuntimeShape(*Field, *Struct.fieldTypes()[FieldIndex]))
        {
          return false;
        }
      }
      return true;
    }
  } // namespace

  class RuntimeAggregateStorage
  {
    public:
      explicit RuntimeAggregateStorage(std::vector<RuntimeValue> Fields) : Fields(std::move(Fields))
      {
      }

      std::vector<RuntimeValue> Fields;
  };

  class ExecutionEngine::Impl
  {
    public:
      Impl(ExecutionContext &Context, const ir::Module &ModuleValue) : Context(Context), ModuleValue(ModuleValue), PreparedFunctions(ModuleValue.Functions.size())
      {
      }

      InitializationResult initialize()
      {
        InitializationResult Result;
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
        if (!registerRuntimeSymbols(Symbols))
        {
          addFailure<core::DiagnosticKind::RuntimeSymbolRegistrationFailed>(Result.Diagnostics);
          return Result;
        }

        for (std::size_t FunctionIndex = 0; FunctionIndex < ModuleValue.Functions.size(); ++FunctionIndex)
        {
          const ir::Function &FunctionValue = ModuleValue.Functions[FunctionIndex];
          if (FunctionValue.Kind != ir::FunctionKind::External)
          {
            continue;
          }
          if (!Symbols.resolveAndRegister(FunctionValue.Name))
          {
            addFailure<core::DiagnosticKind::ExternalFunctionNotFound>(Result.Diagnostics, FunctionValue.Name);
            continue;
          }

          auto Prepared = std::make_unique<PreparedFunction>();
          Prepared->Address = Symbols.findAddress(FunctionValue.Name);
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

      ExecutionResult execute(std::string_view EntryName, const std::vector<RuntimeValue> &Arguments)
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
        for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments.size(); ++ArgumentIndex)
        {
          if (!hasValidRuntimeShape(Arguments[ArgumentIndex], *Entry.ParameterTypes[ArgumentIndex]))
          {
            addFailure<core::DiagnosticKind::EntryArgumentInvalid>(Result.Diagnostics, Entry.Name, ArgumentIndex);
            return Result;
          }
        }

        executeFunction(EntryIndex, Arguments, 0, Result.ReturnValue, Result.Diagnostics);
        return Result;
      }

      NativeSymbolRegistry Symbols;

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

      std::optional<RuntimeValue> zeroValue(const ir::Type &TypeValue)
      {
        switch (TypeValue.kind())
        {
        case ir::TypeKind::Void:
          return RuntimeValue::voidValue(TypeValue);
        case ir::TypeKind::Bool:
        case ir::TypeKind::Byte:
        case ir::TypeKind::I32:
        case ir::TypeKind::PointerSize:
          return RuntimeValue::integerValue(TypeValue, 0);
        case ir::TypeKind::ConstBytePointer:
          return RuntimeValue::pointerValue(TypeValue, nullptr);
        case ir::TypeKind::Struct:
        {
          const ir::StructType &Struct = static_cast<const ir::StructType &>(TypeValue);
          std::vector<RuntimeValue> Fields;
          Fields.reserve(Struct.fieldTypes().size());
          for (const ir::Type *FieldType : Struct.fieldTypes())
          {
            std::optional<RuntimeValue> Field = zeroValue(*FieldType);
            if (!Field.has_value())
            {
              return std::nullopt;
            }
            Fields.push_back(*Field);
          }
          return RuntimeValue::aggregateValue(Struct, std::move(Fields));
        }
        case ir::TypeKind::Count:
          return std::nullopt;
        }
        return std::nullopt;
      }

      std::optional<RuntimeValue> evaluateValue(const ir::Value &Value, const ExecutionFrame &Frame, std::vector<core::Diagnostic> &Diagnostics)
      {
        if (Value.kind() == ir::ValueKind::IntegerConstant)
        {
          const std::int64_t Integer = static_cast<const ir::IntegerConstant &>(Value).value();
          return RuntimeValue::integerValue(Value.type(), static_cast<std::uint64_t>(Integer));
        }
        if (Value.kind() == ir::ValueKind::ValueOperand)
        {
          const ir::ValueId Id = static_cast<const ir::ValueOperand &>(Value).id();
          const RuntimeValue *Stored = Frame.find(Id);
          if (Stored == nullptr)
          {
            addFailure<core::DiagnosticKind::SsaValueUnavailableDuringExecution>(Diagnostics, Id.value());
            return std::nullopt;
          }
          return *Stored;
        }
        if (Value.kind() == ir::ValueKind::GlobalAddressOperand)
        {
          const ir::GlobalAddressOperand &Address = static_cast<const ir::GlobalAddressOperand &>(Value);
          const std::string &Data = ModuleValue.ByteConstants[Address.global().value()].Data;
          return RuntimeValue::pointerValue(Value.type(), Data.data() + Address.byteOffset());
        }
        if (Value.kind() == ir::ValueKind::ZeroInitializer)
        {
          std::optional<RuntimeValue> Result = zeroValue(Value.type());
          if (!Result.has_value())
          {
            addFailure<core::DiagnosticKind::ZeroInitializerConstructionFailed>(Diagnostics, ir::typeKindName(Value.type().kind()));
          }
          return Result;
        }
        addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>(Diagnostics);
        return std::nullopt;
      }

      bool callExternal(std::size_t FunctionIndex, const std::vector<RuntimeValue> &Arguments, std::optional<RuntimeValue> &Result, std::vector<core::Diagnostic> &Diagnostics)
      {
        const ir::Function &FunctionValue = ModuleValue.Functions[FunctionIndex];
        const std::unique_ptr<PreparedFunction> &Prepared = PreparedFunctions[FunctionIndex];
        if (!Prepared)
        {
          addFailure<core::DiagnosticKind::ExternalFunctionNotPrepared>(Diagnostics, FunctionValue.Name);
          return false;
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
          if (!storeNativeValue(Arguments[ArgumentIndex], NativeTypes, ArgumentAddress))
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
        Result = loadNativeValue(*FunctionValue.ResultType, NativeTypes, ReturnStorage ? ReturnStorage->data() : nullptr);
        if (!Result.has_value())
        {
          addFailure<core::DiagnosticKind::NativeResultUnmarshalFailed>(Diagnostics, FunctionValue.Name);
          return false;
        }
        return true;
      }

      bool executeFunction(std::size_t FunctionIndex, const std::vector<RuntimeValue> &Arguments, std::size_t Depth, std::optional<RuntimeValue> &Result, std::vector<core::Diagnostic> &Diagnostics)
      {
        const ir::Function &FunctionValue = ModuleValue.Functions[FunctionIndex];
        if (Depth >= MaximumCallDepth)
        {
          addFailure<core::DiagnosticKind::CallDepthLimitExceeded>(Diagnostics, FunctionValue.Name, MaximumCallDepth);
          return false;
        }
        if (FunctionValue.Kind == ir::FunctionKind::External)
        {
          return callExternal(FunctionIndex, Arguments, Result, Diagnostics);
        }
        if (FunctionValue.Blocks.size() != 1)
        {
          addFailure<core::DiagnosticKind::MultipleBasicBlocksUnsupported>(Diagnostics, FunctionValue.Name, FunctionValue.Blocks.size());
          return false;
        }

        ExecutionFrame Frame(Arguments);

        for (const std::unique_ptr<ir::Instruction> &InstructionPointer : FunctionValue.Blocks[0].Instructions)
        {
          if (InstructionPointer->kind() == ir::InstructionKind::Call)
          {
            const ir::CallInstruction &Call = static_cast<const ir::CallInstruction &>(*InstructionPointer);
            std::vector<RuntimeValue> CallArguments;
            CallArguments.reserve(Call.Arguments.size());
            for (const std::unique_ptr<ir::Value> &Argument : Call.Arguments)
            {
              std::optional<RuntimeValue> ArgumentValue = evaluateValue(*Argument, Frame, Diagnostics);
              if (!ArgumentValue.has_value())
              {
                return false;
              }
              CallArguments.push_back(*ArgumentValue);
            }

            std::optional<RuntimeValue> CallResult;
            if (!executeFunction(Call.Callee.value(), CallArguments, Depth + 1, CallResult, Diagnostics))
            {
              return false;
            }
            if (Call.Result.has_value())
            {
              if (!CallResult.has_value())
              {
                addFailure<core::DiagnosticKind::CallResultMissing>(Diagnostics, FunctionValue.Name);
                return false;
              }
              if (!Frame.define(*Call.Result, *CallResult))
              {
                addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>(Diagnostics, "call", FunctionValue.Name, Call.Result->value());
                return false;
              }
            }
            continue;
          }

          if (InstructionPointer->kind() == ir::InstructionKind::InsertValue)
          {
            const ir::InsertValueInstruction &Insert = static_cast<const ir::InsertValueInstruction &>(*InstructionPointer);
            std::optional<RuntimeValue> Aggregate = evaluateValue(*Insert.Aggregate, Frame, Diagnostics);
            std::optional<RuntimeValue> Element = evaluateValue(*Insert.Element, Frame, Diagnostics);
            if (!Aggregate.has_value() || !Element.has_value())
            {
              return false;
            }
            const ir::StructType &Struct = static_cast<const ir::StructType &>(*Insert.ResultType);
            std::vector<RuntimeValue> Fields;
            Fields.reserve(Struct.fieldTypes().size());
            for (std::size_t FieldIndex = 0; FieldIndex < Struct.fieldTypes().size(); ++FieldIndex)
            {
              const RuntimeValue *Field = Aggregate->field(FieldIndex);
              if (Field == nullptr)
              {
                addFailure<core::DiagnosticKind::InvalidRuntimeAggregate>(Diagnostics, "insertvalue", FunctionValue.Name);
                return false;
              }
              Fields.push_back(FieldIndex == Insert.FieldIndex ? *Element : *Field);
            }
            RuntimeValue InsertResult = RuntimeValue::aggregateValue(Struct, std::move(Fields));
            if (!Frame.define(Insert.Result, std::move(InsertResult)))
            {
              addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>(Diagnostics, "insertvalue", FunctionValue.Name, Insert.Result.value());
              return false;
            }
            continue;
          }

          if (InstructionPointer->kind() == ir::InstructionKind::ExtractValue)
          {
            const ir::ExtractValueInstruction &Extract = static_cast<const ir::ExtractValueInstruction &>(*InstructionPointer);
            std::optional<RuntimeValue> Aggregate = evaluateValue(*Extract.Aggregate, Frame, Diagnostics);
            if (!Aggregate.has_value())
            {
              return false;
            }
            const RuntimeValue *Field = Aggregate->field(Extract.FieldIndex);
            if (Field == nullptr)
            {
              addFailure<core::DiagnosticKind::InvalidRuntimeAggregate>(Diagnostics, "extractvalue", FunctionValue.Name);
              return false;
            }
            if (!Frame.define(Extract.Result, *Field))
            {
              addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>(Diagnostics, "extractvalue", FunctionValue.Name, Extract.Result.value());
              return false;
            }
            continue;
          }

          if (InstructionPointer->kind() == ir::InstructionKind::Return)
          {
            const ir::ReturnInstruction &Return = static_cast<const ir::ReturnInstruction &>(*InstructionPointer);
            if (!Return.ReturnValue)
            {
              Result = RuntimeValue::voidValue(*FunctionValue.ResultType);
              return true;
            }
            Result = evaluateValue(*Return.ReturnValue, Frame, Diagnostics);
            return Result.has_value();
          }

          addFailure<core::DiagnosticKind::UnsupportedInstructionDuringExecution>(Diagnostics, FunctionValue.Name, ir::instructionKindName(InstructionPointer->kind()));
          return false;
        }

        addFailure<core::DiagnosticKind::FunctionMissingReturnDuringExecution>(Diagnostics, FunctionValue.Name);
        return false;
      }

      ExecutionContext &Context;
      const ir::Module &ModuleValue;
      NativeTypeCache NativeTypes;
      std::vector<std::unique_ptr<PreparedFunction>> PreparedFunctions;
      bool Initialized = false;
  };

  RuntimeValue::RuntimeValue(const ir::Type &ValueType) noexcept : ValueType(&ValueType)
  {
  }

  RuntimeValue RuntimeValue::voidValue(const ir::Type &ValueType) noexcept
  {
    return RuntimeValue(ValueType);
  }

  RuntimeValue RuntimeValue::integerValue(const ir::Type &ValueType, std::uint64_t Value) noexcept
  {
    RuntimeValue Result(ValueType);
    Result.Integer = Value;
    return Result;
  }

  RuntimeValue RuntimeValue::pointerValue(const ir::Type &ValueType, const void *Value) noexcept
  {
    RuntimeValue Result(ValueType);
    Result.Pointer = Value;
    return Result;
  }

  RuntimeValue RuntimeValue::aggregateValue(const ir::StructType &ValueType, std::vector<RuntimeValue> Fields)
  {
    RuntimeValue Result(ValueType);
    Result.Aggregate = std::make_shared<RuntimeAggregateStorage>(std::move(Fields));
    return Result;
  }

  const ir::Type &RuntimeValue::type() const noexcept
  {
    return *ValueType;
  }

  std::optional<std::uint64_t> RuntimeValue::integer() const noexcept
  {
    const ir::TypeKind Kind = ValueType->kind();
    if (Kind == ir::TypeKind::Bool || Kind == ir::TypeKind::Byte || Kind == ir::TypeKind::I32 || Kind == ir::TypeKind::PointerSize)
    {
      return Integer;
    }
    return std::nullopt;
  }

  const void *RuntimeValue::pointer() const noexcept
  {
    return ValueType->kind() == ir::TypeKind::ConstBytePointer ? Pointer : nullptr;
  }

  std::size_t RuntimeValue::fieldCount() const noexcept
  {
    return Aggregate ? Aggregate->Fields.size() : 0;
  }

  const RuntimeValue *RuntimeValue::field(std::size_t FieldIndex) const noexcept
  {
    return Aggregate && FieldIndex < Aggregate->Fields.size() ? &Aggregate->Fields[FieldIndex] : nullptr;
  }

  bool InitializationResult::succeeded() const noexcept
  {
    return Succeeded && Diagnostics.empty();
  }

  const std::vector<core::Diagnostic> &InitializationResult::diagnostics() const noexcept
  {
    return Diagnostics;
  }

  bool ExecutionResult::succeeded() const noexcept
  {
    return ReturnValue.has_value() && Diagnostics.empty();
  }

  const std::optional<RuntimeValue> &ExecutionResult::returnValue() const noexcept
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

  NativeSymbolRegistry &ExecutionEngine::nativeSymbols() noexcept
  {
    return Implementation->Symbols;
  }

  const NativeSymbolRegistry &ExecutionEngine::nativeSymbols() const noexcept
  {
    return Implementation->Symbols;
  }

  InitializationResult ExecutionEngine::initialize()
  {
    return Implementation->initialize();
  }

  ExecutionResult ExecutionEngine::execute(std::string_view EntryName, const std::vector<RuntimeValue> &Arguments)
  {
    return Implementation->execute(EntryName, Arguments);
  }
} // namespace ink::execution
