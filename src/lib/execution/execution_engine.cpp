#include "ink/execution/execution_engine.h"

#include "function_executor.h"
#include "ink/execution/module_loader.h"
#include "ink/ir/context.h"
#include "ink/ir/verifier.h"
#include "native_abi.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
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

    ir::ModuleId effectiveModuleId(const ir::Module &ModuleValue) noexcept
    {
      return ModuleValue.Id.valid() ? ModuleValue.Id : ir::ModuleId{0};
    }

    class SingleModuleProvider final : public ModuleProvider
    {
    public:
      explicit SingleModuleProvider(const ir::Module &ModuleValue)
          : ModuleValue(ModuleValue),
            Id(effectiveModuleId(ModuleValue))
      {
      }

      ModuleProvisionResult provideModule(ir::ModuleId Module) noexcept override
      {
        if (Module != Id)
        {
          return ModuleProvisionResult::notFound();
        }
        return ModuleProvisionResult::found(std::shared_ptr<const ir::Module>(&ModuleValue, [](const ir::Module *)
        {
        }));
      }

      ir::ModuleId id() const noexcept
      {
        return Id;
      }

    private:
      const ir::Module &ModuleValue;
      ir::ModuleId Id;
    };

    void appendUniqueDiagnostics(std::vector<core::Diagnostic> &Destination, const std::vector<core::Diagnostic> &Source)
    {
      for (const core::Diagnostic &DiagnosticEntry : Source)
      {
        if (std::find(Destination.begin(), Destination.end(), DiagnosticEntry) == Destination.end())
        {
          Destination.push_back(DiagnosticEntry);
        }
      }
    }
  } // namespace

  class ExecutionEngine::Impl final : public ModuleLifecycle, public ModuleExecutionRuntime
  {
  public:
    Impl(ExecutionContext &Context, const ir::Module &ModuleValue)
        : Context(Context),
          OwnedProvider(std::make_unique<SingleModuleProvider>(ModuleValue)),
          Provider(OwnedProvider.get()),
          EntryModule(static_cast<SingleModuleProvider &>(*Provider).id()),
          Loader(*Provider, *this, Context.compilationContext().targetContext())
    {
    }

    Impl(ExecutionContext &Context, ModuleProvider &Provider, ir::ModuleId EntryModule)
        : Context(Context),
          Provider(&Provider),
          EntryModule(EntryModule),
          Loader(*this->Provider, *this, Context.compilationContext().targetContext())
    {
    }

    ~Impl() override = default;

    ShutdownResult shutdown()
    {
      ShutdownResult Result;
      const std::vector<ModuleLoadError> Errors = Loader.shutdown();
      for (const ModuleLoadError &Error : Errors)
      {
        const std::size_t PreviousDiagnosticCount = Result.Diagnostics.size();
        if (Error.Module.valid())
        {
          appendUniqueDiagnostics(Result.Diagnostics, moduleDiagnostics(Error.Module));
        }
        if (Result.Diagnostics.size() == PreviousDiagnosticCount)
        {
          addLoadFailure(Result.Diagnostics, Error);
        }
      }
      Result.Succeeded = Errors.empty();
      return Result;
    }

    InitializationResult initialize()
    {
      InitializationResult Result;
      const ModuleLoadResult Load = Loader.loadModule(EntryModule);
      if (!Load.succeeded())
      {
        if (Load.instance() != nullptr)
        {
          appendUniqueDiagnostics(Result.Diagnostics, moduleDiagnostics(Load.instance()->id()));
        }
        if (Result.Diagnostics.empty())
        {
          addLoadFailure(Result.Diagnostics, Load.error());
        }
        return Result;
      }
      EntryInstance = Load.instance();
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

      const std::optional<ir::FunctionId> Entry = prepareEntryInvocation(*EntryInstance, EntryName, Arguments, Result.Diagnostics);
      if (!Entry.has_value())
      {
        return Result;
      }

      FunctionExecutor Executor(Context, *this, *EntryInstance, Result.Diagnostics);
      RuntimeValueRef ReturnValue = nullptr;
      if (!Executor.execute(*Entry, Arguments, ReturnValue))
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
    struct RuntimeModuleData
    {
      std::shared_ptr<NativeCallAdapter> NativeCalls;
      std::vector<core::Diagnostic> Diagnostics;
    };

    template <core::DiagnosticKind Kind, typename... ArgumentTypes>
    void addFailure(std::vector<core::Diagnostic> &Diagnostics, ArgumentTypes &&...Arguments)
    {
      core::Diagnostic DiagnosticEntry = core::makeDiagnostic<Kind>({}, std::forward<ArgumentTypes>(Arguments)...);
      Context.diagnosticEngine().report(DiagnosticEntry);
      Diagnostics.push_back(std::move(DiagnosticEntry));
    }

    ModuleLoadError prepare(ModuleInstance &Instance) noexcept override
    {
      std::vector<core::Diagnostic> Diagnostics;
      const ir::Module &Definition = Instance.definition();
      if (Definition.context().compilationContext().targetContext() != Context.compilationContext().targetContext())
      {
        addFailure<core::DiagnosticKind::ExecutionTargetMismatch>(Diagnostics);
        recordModuleDiagnostics(Instance.id(), Diagnostics);
        return ModuleLoadError::failure(ModuleLoadErrorKind::PreparationFailed, Instance.id());
      }

      bool ContextMatches = true;
      {
        const std::lock_guard<std::mutex> Lock(RuntimeMutex);
        if (SharedIRContext == nullptr)
        {
          SharedIRContext = &Definition.context();
        }
        else
        {
          ContextMatches = SharedIRContext == &Definition.context();
        }
      }
      if (!ContextMatches)
      {
        addFailure<core::DiagnosticKind::ModuleIrContextMismatch>(Diagnostics, Instance.id().value());
        recordModuleDiagnostics(Instance.id(), Diagnostics);
        return ModuleLoadError::failure(ModuleLoadErrorKind::PreparationFailed, Instance.id());
      }

      ir::IRContext DiagnosticContext(Context.compilationContext());
      const ir::VerificationResult Verification = ir::verify(DiagnosticContext, Definition);
      if (!Verification.succeeded())
      {
        Diagnostics = Verification.diagnostics();
        recordModuleDiagnostics(Instance.id(), Diagnostics);
        return ModuleLoadError::failure(ModuleLoadErrorKind::PreparationFailed, Instance.id());
      }

      std::shared_ptr<NativeCallAdapter> NativeCalls = std::make_shared<NativeCallAdapter>(Context, Definition);
      if (!NativeCalls->initialize(Diagnostics))
      {
        recordModuleData(Instance.id(), std::move(NativeCalls), Diagnostics);
        return ModuleLoadError::failure(ModuleLoadErrorKind::PreparationFailed, Instance.id());
      }
      recordModuleData(Instance.id(), std::move(NativeCalls), {});
      return {};
    }

    ModuleLoadError initialize(ModuleLoader &, ModuleInstance &Instance) noexcept override
    {
      const ir::Module &Definition = Instance.definition();
      if (!Definition.Initializer.has_value())
      {
        return {};
      }
      std::vector<core::Diagnostic> Diagnostics;
      FunctionExecutor Executor(Context, *this, Instance, Diagnostics);
      RuntimeValueRef Result = nullptr;
      if (!Executor.execute(*Definition.Initializer, {}, Result))
      {
        recordModuleDiagnostics(Instance.id(), Diagnostics);
        return ModuleLoadError::failure(ModuleLoadErrorKind::InitializationFailed, Instance.id());
      }
      return {};
    }

    ModuleLoadError finalize(ModuleLoader &, ModuleInstance &Instance) noexcept override
    {
      const ir::Module &Definition = Instance.definition();
      if (!Definition.Finalizer.has_value())
      {
        return {};
      }
      std::vector<core::Diagnostic> Diagnostics;
      FunctionExecutor Executor(Context, *this, Instance, Diagnostics);
      RuntimeValueRef Result = nullptr;
      if (!Executor.execute(*Definition.Finalizer, {}, Result))
      {
        recordModuleDiagnostics(Instance.id(), Diagnostics);
        return ModuleLoadError::failure(ModuleLoadErrorKind::FinalizationFailed, Instance.id());
      }
      return {};
    }

    bool importModule(ModuleInstance &Importer, ir::ModuleId Target, std::vector<core::Diagnostic> &Diagnostics) override
    {
      const ModuleLoadResult Load = Loader.importModule(Importer, Target);
      if (Load.succeeded())
      {
        return true;
      }
      if (Load.instance() != nullptr)
      {
        appendUniqueDiagnostics(Diagnostics, moduleDiagnostics(Load.instance()->id()));
      }
      if (Diagnostics.empty())
      {
        addLoadFailure(Diagnostics, Load.error());
      }
      return false;
    }

    ModuleInstance *resolveReferencedModule(ModuleInstance &Importer, ir::ModuleId Target, std::vector<core::Diagnostic> &Diagnostics) override
    {
      if (!Target.valid() || Target == Importer.id())
      {
        return &Importer;
      }
      const std::vector<ir::ModuleId> Dependencies = Importer.activeDependencies();
      if (std::find(Dependencies.begin(), Dependencies.end(), Target) == Dependencies.end())
      {
        addFailure<core::DiagnosticKind::ModuleReferenceUnavailable>(Diagnostics, Target.value());
        return nullptr;
      }
      const std::shared_ptr<ModuleInstance> Instance = Loader.findModule(Target);
      if (Instance == nullptr || Instance->state() != ModuleState::Ready)
      {
        addFailure<core::DiagnosticKind::ModuleReferenceUnavailable>(Diagnostics, Target.value());
        return nullptr;
      }
      return Instance.get();
    }

    ExternalFunctionInvoker *externalInvoker(ModuleInstance &Module) noexcept override
    {
      const std::lock_guard<std::mutex> Lock(RuntimeMutex);
      const auto Found = RuntimeModules.find(Module.id().value());
      if (Found == RuntimeModules.end() || Found->second->NativeCalls == nullptr)
      {
        return nullptr;
      }
      return Found->second->NativeCalls.get();
    }

    void addLoadFailure(std::vector<core::Diagnostic> &Diagnostics, const ModuleLoadError &Error)
    {
      if (!Error.Module.valid())
      {
        addFailure<core::DiagnosticKind::ExecutionFailed>(Diagnostics);
        return;
      }
      switch (Error.Kind)
      {
      case ModuleLoadErrorKind::ModuleNotFound:
        addFailure<core::DiagnosticKind::ModuleNotFound>(Diagnostics, Error.Module.value());
        return;
      case ModuleLoadErrorKind::ProviderFailure:
        addFailure<core::DiagnosticKind::ModuleProviderFailed>(Diagnostics, Error.Module.value());
        return;
      case ModuleLoadErrorKind::ModuleIdentityMismatch:
        if (Error.RelatedModule.valid())
        {
          addFailure<core::DiagnosticKind::ModuleIdentityMismatch>(Diagnostics, Error.Module.value(), Error.RelatedModule.value());
          return;
        }
        break;
      case ModuleLoadErrorKind::CircularImport:
        if (Error.RelatedModule.valid())
        {
          addFailure<core::DiagnosticKind::ModuleImportCycle>(Diagnostics, Error.Module.value(), Error.RelatedModule.value());
          return;
        }
        break;
      case ModuleLoadErrorKind::ImportDepthLimitExceeded:
        addFailure<core::DiagnosticKind::ModuleImportDepthLimitExceeded>(Diagnostics, Error.Module.value(), MaximumModuleImportDepth);
        return;
      case ModuleLoadErrorKind::None:
      case ModuleLoadErrorKind::InvalidGlobalStorage:
      case ModuleLoadErrorKind::PreparationFailed:
      case ModuleLoadErrorKind::InitializationFailed:
      case ModuleLoadErrorKind::FinalizationFailed:
      case ModuleLoadErrorKind::InvalidImporter:
      case ModuleLoadErrorKind::InvalidState:
      case ModuleLoadErrorKind::LoaderStopped:
      case ModuleLoadErrorKind::ShutdownDuringInitialization:
      case ModuleLoadErrorKind::ShutdownDuringFinalization:
        break;
      }
      addFailure<core::DiagnosticKind::ModuleLoadFailed>(Diagnostics, Error.Module.value());
    }

    void recordModuleData(ir::ModuleId Module, std::shared_ptr<NativeCallAdapter> NativeCalls, std::vector<core::Diagnostic> Diagnostics)
    {
      const std::lock_guard<std::mutex> Lock(RuntimeMutex);
      std::shared_ptr<RuntimeModuleData> &Data = RuntimeModules[Module.value()];
      if (Data == nullptr)
      {
        Data = std::make_shared<RuntimeModuleData>();
      }
      Data->NativeCalls = std::move(NativeCalls);
      Data->Diagnostics = std::move(Diagnostics);
    }

    void recordModuleDiagnostics(ir::ModuleId Module, const std::vector<core::Diagnostic> &Diagnostics)
    {
      const std::lock_guard<std::mutex> Lock(RuntimeMutex);
      std::shared_ptr<RuntimeModuleData> &Data = RuntimeModules[Module.value()];
      if (Data == nullptr)
      {
        Data = std::make_shared<RuntimeModuleData>();
      }
      Data->Diagnostics = Diagnostics;
    }

    std::vector<core::Diagnostic> moduleDiagnostics(ir::ModuleId Module) const
    {
      const std::lock_guard<std::mutex> Lock(RuntimeMutex);
      const auto Found = RuntimeModules.find(Module.value());
      return Found == RuntimeModules.end() ? std::vector<core::Diagnostic>{} : Found->second->Diagnostics;
    }

    std::optional<ir::FunctionId> prepareEntryInvocation(ModuleInstance &Module, std::string_view EntryName, const std::vector<RuntimeValueRef> &Arguments, std::vector<core::Diagnostic> &Diagnostics)
    {
      const ir::Module &Definition = Module.definition();
      std::size_t EntryIndex = Definition.Functions.size();
      for (std::size_t FunctionIndex = 0; FunctionIndex < Definition.Functions.size(); ++FunctionIndex)
      {
        if (Definition.Functions[FunctionIndex].Name == EntryName)
        {
          EntryIndex = FunctionIndex;
          break;
        }
      }
      if (EntryIndex == Definition.Functions.size())
      {
        addFailure<core::DiagnosticKind::EntryFunctionNotFound>(Diagnostics, EntryName);
        return std::nullopt;
      }

      const ir::FunctionId EntryId{EntryIndex};
      const ir::Function &Entry = Definition.Functions[EntryIndex];
      if ((Definition.Initializer.has_value() && *Definition.Initializer == EntryId) || (Definition.Finalizer.has_value() && *Definition.Finalizer == EntryId))
      {
        addFailure<core::DiagnosticKind::EntryFunctionIsModuleLifecycle>(Diagnostics, Entry.Name);
        return std::nullopt;
      }
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
      return EntryId;
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
    std::unique_ptr<ModuleProvider> OwnedProvider;
    ModuleProvider *Provider;
    ir::ModuleId EntryModule;
    mutable std::mutex RuntimeMutex;
    const ir::IRContext *SharedIRContext = nullptr;
    std::unordered_map<std::size_t, std::shared_ptr<RuntimeModuleData>> RuntimeModules;
    std::shared_ptr<ModuleInstance> EntryInstance;
    ModuleLoader Loader;
  };

  bool InitializationResult::succeeded() const noexcept
  {
    return Succeeded && Diagnostics.empty();
  }

  const std::vector<core::Diagnostic> &InitializationResult::diagnostics() const noexcept
  {
    return Diagnostics;
  }

  ExecutionResult::ExecutionResult(ExecutionResult &&Other) noexcept
      : ValueArena(std::move(Other.ValueArena)),
        ReturnValue(Other.ReturnValue),
        Diagnostics(std::move(Other.Diagnostics))
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

  bool ShutdownResult::succeeded() const noexcept
  {
    return Succeeded && Diagnostics.empty();
  }

  const std::vector<core::Diagnostic> &ShutdownResult::diagnostics() const noexcept
  {
    return Diagnostics;
  }

  ExecutionEngine::ExecutionEngine(ExecutionContext &Context, const ir::Module &ModuleValue)
      : Implementation(std::make_unique<Impl>(Context, ModuleValue))
  {
  }

  ExecutionEngine::ExecutionEngine(ExecutionContext &Context, ModuleProvider &Provider, ir::ModuleId EntryModule)
      : Implementation(std::make_unique<Impl>(Context, Provider, EntryModule))
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

  ShutdownResult ExecutionEngine::shutdown()
  {
    return Implementation->shutdown();
  }
} // namespace ink::execution
