#include "ink/execution/execution_engine.h"

#include "function_executor.h"
#include "ink/ir/context.h"
#include "ink/ir/verifier.h"
#include "native_abi.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
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

    enum class RuntimeArgumentValidationKind
    {
      Valid,
      CountMismatch,
      InvalidArgument,
    };

    struct RuntimeArgumentValidationResult
    {
      RuntimeArgumentValidationKind Kind;
      std::size_t ArgumentIndex = 0;
    };

    RuntimeArgumentValidationResult validateRuntimeArguments(const ir::Function &FunctionValue, const std::vector<RuntimeValueRef> &Arguments, const core::TargetContext &Target)
    {
      if (FunctionValue.ParameterTypes.size() != Arguments.size())
      {
        return {RuntimeArgumentValidationKind::CountMismatch};
      }
      std::unordered_set<const RuntimeValue *> ValidatedValues;
      std::unordered_set<const RuntimeValue *> ActiveValues;
      for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments.size(); ++ArgumentIndex)
      {
        if (Arguments[ArgumentIndex] == nullptr || !hasValidRuntimeShape(*Arguments[ArgumentIndex], *FunctionValue.ParameterTypes[ArgumentIndex], Target, ValidatedValues, ActiveValues))
        {
          return {RuntimeArgumentValidationKind::InvalidArgument, ArgumentIndex};
        }
      }
      return {RuntimeArgumentValidationKind::Valid};
    }
  } // namespace

  class ExecutionEngine::Impl
  {
    public:
      Impl(ExecutionContext &Context, const ir::Module &ModuleValue) : Context(Context), ModuleValue(ModuleValue), NativeCalls(Context, ModuleValue)
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
        if (!NativeCalls.initialize(Result.Diagnostics))
        {
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

        const std::optional<std::size_t> EntryIndex = prepareEntryInvocation(EntryName, Arguments, Result.Diagnostics);
        if (!EntryIndex.has_value())
        {
          return Result;
        }

        FunctionExecutor Executor(Context, ModuleValue, NativeCalls, Result.Diagnostics);
        RuntimeValueRef ReturnValue = nullptr;
        if (!Executor.execute(*EntryIndex, Arguments, ReturnValue))
        {
          return Result;
        }
        if (!prepareReturnValue(ReturnValue, Result))
        {
          return Result;
        }
        return Result;
      }

    private:
      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      void addFailure(std::vector<core::Diagnostic> &Diagnostics, ArgumentTypes &&...Arguments)
      {
        core::Diagnostic DiagnosticEntry = core::makeDiagnostic<Kind>({}, std::forward<ArgumentTypes>(Arguments)...);
        Context.diagnosticEngine().report(DiagnosticEntry);
        Diagnostics.push_back(std::move(DiagnosticEntry));
      }

      std::optional<std::size_t> prepareEntryInvocation(std::string_view EntryName, const std::vector<RuntimeValueRef> &Arguments, std::vector<core::Diagnostic> &Diagnostics)
      {
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
          addFailure<core::DiagnosticKind::EntryFunctionNotFound>(Diagnostics, EntryName);
          return std::nullopt;
        }

        const ir::Function &Entry = ModuleValue.Functions[EntryIndex];
        if (Entry.Kind != ir::FunctionKind::Definition)
        {
          addFailure<core::DiagnosticKind::EntryFunctionMustBeDefined>(Diagnostics, Entry.Name);
          return std::nullopt;
        }

        const RuntimeArgumentValidationResult ArgumentValidation = validateRuntimeArguments(Entry, Arguments, Context.compilationContext().targetContext());
        if (ArgumentValidation.Kind == RuntimeArgumentValidationKind::CountMismatch)
        {
          addFailure<core::DiagnosticKind::EntryArgumentCountMismatch>(Diagnostics, Entry.Name, Entry.ParameterTypes.size(), Arguments.size());
          return std::nullopt;
        }
        if (ArgumentValidation.Kind == RuntimeArgumentValidationKind::InvalidArgument)
        {
          addFailure<core::DiagnosticKind::EntryArgumentInvalid>(Diagnostics, Entry.Name, ArgumentValidation.ArgumentIndex);
          return std::nullopt;
        }
        return EntryIndex;
      }

      bool prepareReturnValue(RuntimeValueRef ReturnValue, ExecutionResult &Result)
      {
        if (ReturnValue == nullptr)
        {
          addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>(Result.Diagnostics);
          return false;
        }
        std::shared_ptr<RuntimeValueArena> ResultValues = std::make_shared<RuntimeValueArena>(Context.compilationContext().targetContext());
        RuntimeValueRef StableReturnValue = ResultValues->clone(*ReturnValue);
        if (StableReturnValue == nullptr)
        {
          addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>(Result.Diagnostics);
          return false;
        }
        Result.ValueArena = std::move(ResultValues);
        Result.ReturnValue = StableReturnValue;
        return true;
      }

      ExecutionContext &Context;
      const ir::Module &ModuleValue;
      NativeCallAdapter NativeCalls;
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
