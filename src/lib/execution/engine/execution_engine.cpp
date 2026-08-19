#include "ink/execution/execution_engine.h"

#include "engine/function_executor.h"
#include "ink/execution/module/module_loader.h"
#include "ink/ir/analysis/verifier.h"
#include "ink/ir/model/context.h"
#include "native/native_abi.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    struct ModuleLoadFailureKey
    {
        ModuleLoadErrorKind Kind;
        ModuleId Module;
        ModuleId RelatedModule;

        bool operator==(const ModuleLoadFailureKey &Other) const noexcept
        {
          return Kind == Other.Kind && Module == Other.Module && RelatedModule == Other.RelatedModule;
        }
    };

    struct ModuleLoadFailureKeyHash
    {
        std::size_t operator()(const ModuleLoadFailureKey &Key) const noexcept
        {
          std::size_t Result = static_cast<std::size_t>(Key.Kind);
          Result ^= Key.Module.value() + 0x9E3779B9U + (Result << 6U) + (Result >> 2U);
          Result ^= Key.RelatedModule.value() + 0x9E3779B9U + (Result << 6U) + (Result >> 2U);
          return Result;
        }
    };

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
      if (Value.fieldCount() != Struct.fieldCount() || !ActiveValues.insert(&Value).second)
      {
        return false;
      }
      for (std::size_t FieldIndex = 0; FieldIndex < Struct.fieldCount(); ++FieldIndex)
      {
        const RuntimeValue *Field = Value.field(FieldIndex);
        if (Field == nullptr || !hasValidRuntimeShape(*Field, *Struct.fieldType(FieldIndex), Target, ValidatedValues, ActiveValues))
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
      if (FunctionValue.parameterCount() != Arguments.size())
      {
        return {RuntimeArgumentValidationKind::CountMismatch};
      }
      std::unordered_set<const RuntimeValue *> ValidatedValues;
      std::unordered_set<const RuntimeValue *> ActiveValues;
      for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments.size(); ++ArgumentIndex)
      {
        if (Arguments[ArgumentIndex] == nullptr || !hasValidRuntimeShape(*Arguments[ArgumentIndex], *FunctionValue.parameterType(ArgumentIndex), Target, ValidatedValues, ActiveValues))
        {
          return {RuntimeArgumentValidationKind::InvalidArgument, ArgumentIndex};
        }
      }
      return {RuntimeArgumentValidationKind::Valid};
    }

    bool sameFunctionSignature(const ir::Function &Left, const ir::Function &Right)
    {
      if (Left.ResultType != Right.ResultType || Left.parameterCount() != Right.parameterCount())
      {
        return false;
      }
      for (std::size_t ParameterIndex = 0; ParameterIndex < Left.parameterCount(); ++ParameterIndex)
      {
        if (Left.parameterType(ParameterIndex) != Right.parameterType(ParameterIndex))
        {
          return false;
        }
      }
      return true;
    }

    ir::Name effectiveModuleName(const ir::Module &ModuleValue)
    {
      return ModuleValue.Name.has_value() ? *ModuleValue.Name : ir::Name("ink.entry");
    }

    class SingleModuleProvider final : public ModuleProvider
    {
      public:
        explicit SingleModuleProvider(const ir::Module &ModuleValue)
            : ModuleValue(ModuleValue),
              Name(effectiveModuleName(ModuleValue))
        {
        }

        ModuleProvisionResult provideModule(const ir::Name &ModuleName) noexcept override
        {
          if (ModuleName != Name)
          {
            return ModuleProvisionResult::notFound();
          }
          return ModuleProvisionResult::found(std::shared_ptr<const ir::Module>(&ModuleValue, [](const ir::Module *)
                                                                                {
                                                                                }));
        }

        ir::IRContext &irContext() noexcept override
        {
          return ModuleValue.context();
        }

        const ir::Name &name() const noexcept
        {
          return Name;
        }

      private:
        const ir::Module &ModuleValue;
        ir::Name Name;
    };

  } // namespace

  class ExecutionEngine::Impl final : public ModuleLifecycle, public ModuleExecutionRuntime
  {
    public:
      Impl(ExecutionContext &Context, const ir::Module &ModuleValue)
          : Context(Context),
            OwnedProvider(std::make_unique<SingleModuleProvider>(ModuleValue)),
            Provider(OwnedProvider.get()),
            IRContext(ModuleValue.context()),
            EntryModule(static_cast<SingleModuleProvider &>(*Provider).name()),
            Loader(*Provider, *this, Context.compilationContext().targetContext())
      {
      }

      Impl(ExecutionContext &Context, ModuleProvider &Provider, ir::Name EntryModule)
          : Context(Context),
            Provider(&Provider),
            IRContext(Provider.irContext()),
            EntryModule(std::move(EntryModule)),
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
          if (!Error.DiagnosticReported)
          {
            addLoadFailure(Error);
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
          if (!Load.error().DiagnosticReported)
          {
            addLoadFailure(Load.error());
          }
          return Result;
        }
        EntryInstance = Load.instance();
        Result.Succeeded = true;
        return Result;
      }

      DynamicModuleLoadResult loadModule(const ir::Name &ModuleName)
      {
        DynamicModuleLoadResult Result;
        const ModuleLoadResult Load = Loader.loadModule(ModuleName);
        if (!Load.succeeded())
        {
          if (!Load.error().DiagnosticReported)
          {
            addLoadFailure(Load.error());
          }
          return Result;
        }
        Result.Module = Load.instance()->id();
        return Result;
      }

      ExecutionResult execute(const ir::Name &EntryName, const std::vector<RuntimeValueRef> &Arguments)
      {
        ExecutionResult Result;
        InitializationResult Initialization = initialize();
        if (!Initialization.succeeded())
        {
          return Result;
        }

        const std::optional<ir::FunctionId> Entry = prepareEntryInvocation(*EntryInstance, EntryName, Arguments);
        if (!Entry.has_value())
        {
          return Result;
        }

        FunctionExecutor Executor(Context, *this, *EntryInstance);
        RuntimeValueRef ReturnValue = nullptr;
        if (!Executor.execute(*Entry, Arguments, ReturnValue))
        {
          return Result;
        }
        if (!prepareReturnValue(ReturnValue, Result))
        {
          return Result;
        }
        Result.Succeeded = true;
        return Result;
      }

    private:
      struct ImportedFunctionBinding
      {
          ModuleId Module;
          ir::FunctionId Function;
      };

      struct ImportedGlobalBinding
      {
          ModuleId Module;
          ir::GlobalId Global;
      };

      struct RuntimeModuleData
      {
          std::shared_ptr<NativeCallAdapter> NativeCalls;
          std::vector<std::optional<ImportedFunctionBinding>> ImportedFunctions;
          std::vector<std::optional<ImportedGlobalBinding>> ImportedGlobals;
      };

      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      void addFailure(ArgumentTypes &&...Arguments)
      {
        core::Diagnostic DiagnosticEntry = core::makeDiagnostic<Kind>({}, std::forward<ArgumentTypes>(Arguments)...);
        Context.diagnosticEngine().report(DiagnosticEntry);
      }

      ModuleLoadError prepare(ModuleInstance &Instance) noexcept override
      {
        const ir::Module &Definition = Instance.definition();
        if (Definition.context().compilationContext().targetContext() != Context.compilationContext().targetContext())
        {
          addFailure<core::DiagnosticKind::ExecutionTargetMismatch>();
          return ModuleLoadError::failure(ModuleLoadErrorKind::PreparationFailed, Instance.id(), {}, true);
        }

        if (&Definition.context() != &IRContext)
        {
          addFailure<core::DiagnosticKind::ModuleIrContextMismatch>(Instance.name());
          return ModuleLoadError::failure(ModuleLoadErrorKind::PreparationFailed, Instance.id(), {}, true);
        }

        ir::IRContext DiagnosticContext(Context.compilationContext());
        const ir::VerificationResult Verification = ir::verify(DiagnosticContext, Definition);
        if (!Verification.succeeded())
        {
          return ModuleLoadError::failure(ModuleLoadErrorKind::PreparationFailed, Instance.id(), {}, true);
        }

        std::shared_ptr<NativeCallAdapter> NativeCalls = std::make_shared<NativeCallAdapter>(Context, Definition);
        if (!NativeCalls->initialize())
        {
          recordModuleData(Instance.id(), Definition.Functions.size(), Definition.Globals.size(), std::move(NativeCalls));
          return ModuleLoadError::failure(ModuleLoadErrorKind::PreparationFailed, Instance.id(), {}, true);
        }
        recordModuleData(Instance.id(), Definition.Functions.size(), Definition.Globals.size(), std::move(NativeCalls));
        return {};
      }

      ModuleLoadError initialize(ModuleLoader &, ModuleInstance &Instance) noexcept override
      {
        const ir::Module &Definition = Instance.definition();
        if (Definition.Initializer.has_value())
        {
          FunctionExecutor Executor(Context, *this, Instance);
          RuntimeValueRef Result = nullptr;
          if (!Executor.execute(*Definition.Initializer, {}, Result))
          {
            return ModuleLoadError::failure(ModuleLoadErrorKind::InitializationFailed, Instance.id(), {}, true);
          }
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
        FunctionExecutor Executor(Context, *this, Instance);
        RuntimeValueRef Result = nullptr;
        if (!Executor.execute(*Definition.Finalizer, {}, Result))
        {
          return ModuleLoadError::failure(ModuleLoadErrorKind::FinalizationFailed, Instance.id(), {}, true);
        }
        return {};
      }

      bool importModule(ModuleInstance &Importer, const ir::Name &Target) override
      {
        const ModuleLoadResult Load = Loader.importModule(Importer, Target);
        if (Load.succeeded())
        {
          return bindImportedSymbols(Importer, Target, Load.instance()->id());
        }
        if (!Load.error().DiagnosticReported)
        {
          addLoadFailure(Load.error());
        }
        return false;
      }

      ModuleInstance *resolveReferencedModule(ModuleInstance &Importer, ModuleId Target) override
      {
        if (!Target.valid() || Target == Importer.id())
        {
          return &Importer;
        }
        const std::vector<ModuleId> Dependencies = Importer.activeDependencies();
        if (std::find(Dependencies.begin(), Dependencies.end(), Target) == Dependencies.end())
        {
          addFailure<core::DiagnosticKind::ModuleReferenceUnavailable>(Loader.moduleName(Target));
          return nullptr;
        }
        const std::shared_ptr<ModuleInstance> Instance = Loader.findModule(Target);
        if (Instance == nullptr || Instance->state() != ModuleState::Ready)
        {
          addFailure<core::DiagnosticKind::ModuleReferenceUnavailable>(Loader.moduleName(Target));
          return nullptr;
        }
        return Instance.get();
      }

      bool resolveImportedFunction(ModuleInstance &Importer, ir::FunctionId Import, ModuleInstance *&TargetModule, ir::FunctionId &TargetFunction) override
      {
        std::optional<ImportedFunctionBinding> Binding;
        {
          const std::lock_guard<std::mutex> Lock(RuntimeMutex);
          const auto Found = RuntimeModules.find(Importer.id().value());
          if (Found != RuntimeModules.end() && Found->second != nullptr && Import.valid() && Import.value() < Found->second->ImportedFunctions.size())
          {
            Binding = Found->second->ImportedFunctions[Import.value()];
          }
        }
        if (!Binding.has_value())
        {
          const ir::Module &Definition = Importer.definition();
          const ir::Name FunctionName = Import.valid() && Import.value() < Definition.Functions.size() ? Definition.Functions[Import.value()].Name : ir::Name{};
          addFailure<core::DiagnosticKind::ImportedFunctionNotBound>(FunctionName, Importer.name());
          return false;
        }
        TargetModule = resolveReferencedModule(Importer, Binding->Module);
        TargetFunction = Binding->Function;
        return TargetModule != nullptr;
      }

      bool resolveImportedGlobal(ModuleInstance &Importer, ir::GlobalId Import, ModuleInstance *&TargetModule, ir::GlobalId &TargetGlobal) override
      {
        std::optional<ImportedGlobalBinding> Binding;
        {
          const std::lock_guard<std::mutex> Lock(RuntimeMutex);
          const auto Found = RuntimeModules.find(Importer.id().value());
          if (Found != RuntimeModules.end() && Found->second != nullptr && Import.valid() && Import.value() < Found->second->ImportedGlobals.size())
          {
            Binding = Found->second->ImportedGlobals[Import.value()];
          }
        }
        if (!Binding.has_value())
        {
          const ir::Module &Definition = Importer.definition();
          const ir::Name GlobalName = Import.valid() && Import.value() < Definition.Globals.size() ? Definition.Globals[Import.value()].Name : ir::Name{};
          addFailure<core::DiagnosticKind::ImportedGlobalNotBound>(GlobalName, Importer.name());
          return false;
        }
        TargetModule = resolveReferencedModule(Importer, Binding->Module);
        TargetGlobal = Binding->Global;
        return TargetModule != nullptr;
      }

      bool bindImportedFunctions(ModuleInstance &Importer, const ir::Name &TargetName, ModuleId Target)
      {
        ModuleInstance *TargetInstance = resolveReferencedModule(Importer, Target);
        if (TargetInstance == nullptr)
        {
          return false;
        }
        const ir::Module &ImporterDefinition = Importer.definition();
        const ir::Module &TargetDefinition = TargetInstance->definition();
        std::vector<std::pair<std::size_t, ImportedFunctionBinding>> Bindings;
        for (std::size_t FunctionIndex = 0; FunctionIndex < ImporterDefinition.Functions.size(); ++FunctionIndex)
        {
          const ir::Function &ImportedFunction = ImporterDefinition.Functions[FunctionIndex];
          if (ImportedFunction.Kind != ir::FunctionKind::Imported || !ImportedFunction.Import.has_value() || ImportedFunction.Import->Module != TargetName)
          {
            continue;
          }
          const std::optional<ir::FunctionId> TargetFunction = TargetDefinition.findFunction(ImportedFunction.Import->Symbol);
          if (!TargetFunction.has_value() || (TargetDefinition.Initializer.has_value() && *TargetDefinition.Initializer == *TargetFunction) || (TargetDefinition.Finalizer.has_value() && *TargetDefinition.Finalizer == *TargetFunction))
          {
            addFailure<core::DiagnosticKind::ImportedFunctionNotFound>(ImportedFunction.Name, ImportedFunction.Import->Symbol, TargetName);
            return false;
          }
          if (!sameFunctionSignature(ImportedFunction, TargetDefinition.Functions[TargetFunction->value()]))
          {
            addFailure<core::DiagnosticKind::ImportedFunctionSignatureMismatch>(ImportedFunction.Name, ImportedFunction.Import->Symbol, TargetName);
            return false;
          }
          Bindings.push_back({FunctionIndex, {Target, *TargetFunction}});
        }
        bool Bound = false;
        {
          const std::lock_guard<std::mutex> Lock(RuntimeMutex);
          const auto Found = RuntimeModules.find(Importer.id().value());
          if (Found != RuntimeModules.end() && Found->second != nullptr && Found->second->ImportedFunctions.size() == ImporterDefinition.Functions.size())
          {
            for (const auto &Binding : Bindings)
            {
              Found->second->ImportedFunctions[Binding.first] = Binding.second;
            }
            Bound = true;
          }
        }
        if (!Bound)
        {
          addFailure<core::DiagnosticKind::ModuleLoadFailed>(Importer.name());
        }
        return Bound;
      }

      bool bindImportedGlobals(ModuleInstance &Importer, const ir::Name &TargetName, ModuleId Target)
      {
        ModuleInstance *TargetInstance = resolveReferencedModule(Importer, Target);
        if (TargetInstance == nullptr)
        {
          return false;
        }
        const ir::Module &ImporterDefinition = Importer.definition();
        const ir::Module &TargetDefinition = TargetInstance->definition();
        std::vector<std::pair<std::size_t, ImportedGlobalBinding>> Bindings;
        for (std::size_t GlobalIndex = 0; GlobalIndex < ImporterDefinition.Globals.size(); ++GlobalIndex)
        {
          const ir::GlobalVariable &ImportedGlobal = ImporterDefinition.Globals[GlobalIndex];
          if (ImportedGlobal.Kind != ir::GlobalVariableKind::Imported || !ImportedGlobal.Import.has_value() || ImportedGlobal.Import->Module != TargetName)
          {
            continue;
          }
          const std::optional<ir::GlobalId> TargetGlobalId = TargetDefinition.findGlobal(ImportedGlobal.Import->Symbol);
          if (!TargetGlobalId.has_value())
          {
            addFailure<core::DiagnosticKind::ImportedGlobalNotFound>(ImportedGlobal.Name, ImportedGlobal.Import->Symbol, TargetName);
            return false;
          }
          const ir::GlobalVariable &TargetGlobal = TargetDefinition.Globals[TargetGlobalId->value()];
          if (ImportedGlobal.ValueType != TargetGlobal.ValueType)
          {
            addFailure<core::DiagnosticKind::ImportedGlobalTypeMismatch>(ImportedGlobal.Name, ImportedGlobal.Import->Symbol, TargetName);
            return false;
          }
          if (ImportedGlobal.Mutable && !TargetGlobal.Mutable)
          {
            addFailure<core::DiagnosticKind::ImportedGlobalMutabilityMismatch>(ImportedGlobal.Name, ImportedGlobal.Import->Symbol, TargetName);
            return false;
          }
          Bindings.push_back({GlobalIndex, {Target, *TargetGlobalId}});
        }
        bool Bound = false;
        {
          const std::lock_guard<std::mutex> Lock(RuntimeMutex);
          const auto Found = RuntimeModules.find(Importer.id().value());
          if (Found != RuntimeModules.end() && Found->second != nullptr && Found->second->ImportedGlobals.size() == ImporterDefinition.Globals.size())
          {
            for (const auto &Binding : Bindings)
            {
              Found->second->ImportedGlobals[Binding.first] = Binding.second;
            }
            Bound = true;
          }
        }
        if (!Bound)
        {
          addFailure<core::DiagnosticKind::ModuleLoadFailed>(Importer.name());
        }
        return Bound;
      }

      bool bindImportedSymbols(ModuleInstance &Importer, const ir::Name &TargetName, ModuleId Target)
      {
        return bindImportedFunctions(Importer, TargetName, Target) && bindImportedGlobals(Importer, TargetName, Target);
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

      void addLoadFailure(const ModuleLoadError &Error)
      {
        {
          const std::lock_guard<std::mutex> Lock(ReportedLoadFailuresMutex);
          if (!ReportedLoadFailures.insert({Error.Kind, Error.Module, Error.RelatedModule}).second)
          {
            return;
          }
        }
        if (!Error.Module.valid())
        {
          addFailure<core::DiagnosticKind::ExecutionFailed>();
          return;
        }
        switch (Error.Kind)
        {
        case ModuleLoadErrorKind::ModuleNotFound:
          addFailure<core::DiagnosticKind::ModuleNotFound>(Loader.moduleName(Error.Module));
          return;
        case ModuleLoadErrorKind::ProviderFailure:
          addFailure<core::DiagnosticKind::ModuleProviderFailed>(Loader.moduleName(Error.Module));
          return;
        case ModuleLoadErrorKind::ModuleIdentityMismatch:
          if (Error.RelatedModule.valid())
          {
            addFailure<core::DiagnosticKind::ModuleIdentityMismatch>(Loader.moduleName(Error.Module), Loader.moduleName(Error.RelatedModule));
            return;
          }
          break;
        case ModuleLoadErrorKind::CircularImport:
          if (Error.RelatedModule.valid())
          {
            addFailure<core::DiagnosticKind::ModuleImportCycle>(Loader.moduleName(Error.Module), Loader.moduleName(Error.RelatedModule));
            return;
          }
          break;
        case ModuleLoadErrorKind::ImportDepthLimitExceeded:
          addFailure<core::DiagnosticKind::ModuleImportDepthLimitExceeded>(Loader.moduleName(Error.Module), MaximumModuleImportDepth);
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
        addFailure<core::DiagnosticKind::ModuleLoadFailed>(Loader.moduleName(Error.Module));
      }

      void recordModuleData(ModuleId Module, std::size_t FunctionCount, std::size_t GlobalCount, std::shared_ptr<NativeCallAdapter> NativeCalls)
      {
        const std::lock_guard<std::mutex> Lock(RuntimeMutex);
        std::shared_ptr<RuntimeModuleData> &Data = RuntimeModules[Module.value()];
        if (Data == nullptr)
        {
          Data = std::make_shared<RuntimeModuleData>();
        }
        Data->NativeCalls = std::move(NativeCalls);
        Data->ImportedFunctions.resize(FunctionCount);
        Data->ImportedGlobals.resize(GlobalCount);
      }

      std::optional<ir::FunctionId> prepareEntryInvocation(ModuleInstance &Module, const ir::Name &EntryName, const std::vector<RuntimeValueRef> &Arguments)
      {
        const ir::Module &Definition = Module.definition();
        const std::optional<ir::FunctionId> EntryId = Definition.findFunction(EntryName);
        if (!EntryId.has_value())
        {
          addFailure<core::DiagnosticKind::EntryFunctionNotFound>(EntryName);
          return std::nullopt;
        }

        const ir::Function &Entry = Definition.Functions[EntryId->value()];
        if ((Definition.Initializer.has_value() && *Definition.Initializer == *EntryId) || (Definition.Finalizer.has_value() && *Definition.Finalizer == *EntryId))
        {
          addFailure<core::DiagnosticKind::EntryFunctionIsModuleLifecycle>(Entry.Name);
          return std::nullopt;
        }
        if (Entry.Kind != ir::FunctionKind::Definition)
        {
          addFailure<core::DiagnosticKind::EntryFunctionMustBeDefined>(Entry.Name);
          return std::nullopt;
        }

        const RuntimeArgumentValidationResult ArgumentValidation = validateRuntimeArguments(Entry, Arguments, Context.compilationContext().targetContext());
        if (ArgumentValidation.Kind == RuntimeArgumentValidationKind::CountMismatch)
        {
          addFailure<core::DiagnosticKind::EntryArgumentCountMismatch>(Entry.Name, Entry.parameterCount(), Arguments.size());
          return std::nullopt;
        }
        if (ArgumentValidation.Kind == RuntimeArgumentValidationKind::InvalidArgument)
        {
          addFailure<core::DiagnosticKind::EntryArgumentInvalid>(Entry.Name, ArgumentValidation.ArgumentIndex);
          return std::nullopt;
        }
        return *EntryId;
      }

      bool prepareReturnValue(RuntimeValueRef ReturnValue, ExecutionResult &Result)
      {
        if (ReturnValue == nullptr)
        {
          addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>();
          return false;
        }
        std::shared_ptr<RuntimeValueArena> ResultValues = std::make_shared<RuntimeValueArena>(Context.compilationContext().targetContext());
        RuntimeValueRef StableReturnValue = ResultValues->clone(*ReturnValue);
        if (StableReturnValue == nullptr)
        {
          addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>();
          return false;
        }
        Result.ValueArena = std::move(ResultValues);
        Result.ReturnValue = StableReturnValue;
        return true;
      }

      ExecutionContext &Context;
      std::unique_ptr<ModuleProvider> OwnedProvider;
      ModuleProvider *Provider;
      ir::IRContext &IRContext;
      ir::Name EntryModule;
      mutable std::mutex RuntimeMutex;
      std::mutex ReportedLoadFailuresMutex;
      std::unordered_set<ModuleLoadFailureKey, ModuleLoadFailureKeyHash> ReportedLoadFailures;
      std::unordered_map<std::size_t, std::shared_ptr<RuntimeModuleData>> RuntimeModules;
      std::shared_ptr<ModuleInstance> EntryInstance;
      ModuleLoader Loader;
  };

  bool InitializationResult::succeeded() const noexcept
  {
    return Succeeded;
  }

  ExecutionResult::ExecutionResult(ExecutionResult &&Other) noexcept
      : Succeeded(Other.Succeeded),
        ValueArena(std::move(Other.ValueArena)),
        ReturnValue(Other.ReturnValue)
  {
    Other.Succeeded = false;
    Other.ReturnValue = nullptr;
  }

  ExecutionResult &ExecutionResult::operator=(ExecutionResult &&Other) noexcept
  {
    if (this != &Other)
    {
      Succeeded = Other.Succeeded;
      ValueArena = std::move(Other.ValueArena);
      ReturnValue = Other.ReturnValue;
      Other.Succeeded = false;
      Other.ReturnValue = nullptr;
    }
    return *this;
  }

  bool ExecutionResult::succeeded() const noexcept
  {
    return Succeeded;
  }

  RuntimeValueRef ExecutionResult::returnValue() const & noexcept
  {
    return ReturnValue;
  }

  bool ShutdownResult::succeeded() const noexcept
  {
    return Succeeded;
  }

  ExecutionEngine::ExecutionEngine(ExecutionContext &Context, const ir::Module &ModuleValue)
      : Implementation(std::make_unique<Impl>(Context, ModuleValue))
  {
  }

  ExecutionEngine::ExecutionEngine(ExecutionContext &Context, ModuleProvider &Provider, ir::Name EntryModule)
      : Implementation(std::make_unique<Impl>(Context, Provider, std::move(EntryModule)))
  {
  }

  ExecutionEngine::~ExecutionEngine() = default;

  InitializationResult ExecutionEngine::initialize()
  {
    return Implementation->initialize();
  }

  DynamicModuleLoadResult ExecutionEngine::loadModule(const ir::Name &ModuleName)
  {
    return Implementation->loadModule(ModuleName);
  }

  ExecutionResult ExecutionEngine::execute(const ir::Name &EntryName, const std::vector<RuntimeValueRef> &Arguments)
  {
    return Implementation->execute(EntryName, Arguments);
  }

  ShutdownResult ExecutionEngine::shutdown()
  {
    return Implementation->shutdown();
  }
} // namespace ink::execution
