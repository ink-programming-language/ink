#include "ink/execution/execution_engine.h"
#include "ink/execution/module_loader.h"
#include "ink/ir/context.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    struct ModuleExecutionTestContext
    {
      core::CompilationContext Compilation;
      ir::IRContext IR{Compilation};
      ExecutionContext Execution{Compilation};
    };

    std::vector<std::int32_t> RecordedModuleEvents;
    std::int32_t ModuleImportSelector = 0;
    std::size_t LateModuleInitializerCallCount = 0;

    extern "C" void recordModuleEvent(std::int32_t Event)
    {
      RecordedModuleEvents.push_back(Event);
    }

    extern "C" std::int32_t selectModuleImport()
    {
      return ModuleImportSelector;
    }

    extern "C" void lateModuleInitializer()
    {
      ++LateModuleInitializerCallCount;
    }

    extern "C" const std::uint8_t *identityModulePointer(const std::uint8_t *Value)
    {
      return Value;
    }

    extern "C" const std::uint8_t *advanceModulePointer(const std::uint8_t *Value)
    {
      return Value + 1;
    }

    extern "C" std::uint8_t *downgradeModulePointer(const std::uint8_t *Value)
    {
      return const_cast<std::uint8_t *>(Value);
    }

    class TestModuleProvider final : public ModuleProvider
    {
    public:
      void addModule(std::shared_ptr<const ir::Module> ModuleValue)
      {
        Modules.push_back(std::move(ModuleValue));
      }

      ModuleProvisionResult provideModule(ir::ModuleId Module) noexcept override
      {
        ++RequestCounts[Module.value()];
        const auto Found = std::find_if(Modules.begin(), Modules.end(), [Module](const std::shared_ptr<const ir::Module> &ModuleValue)
        {
          return ModuleValue != nullptr && ModuleValue->Id == Module;
        });
        return Found == Modules.end() ? ModuleProvisionResult::notFound() : ModuleProvisionResult::found(*Found);
      }

      std::size_t requestCount(ir::ModuleId Module) const noexcept
      {
        const auto Found = RequestCounts.find(Module.value());
        return Found == RequestCounts.end() ? 0 : Found->second;
      }

    private:
      std::vector<std::shared_ptr<const ir::Module>> Modules;
      std::unordered_map<std::size_t, std::size_t> RequestCounts;
    };

    class ReentrantShutdownModuleProvider final : public ModuleProvider
    {
    public:
      explicit ReentrantShutdownModuleProvider(std::shared_ptr<const ir::Module> ModuleValue)
          : ModuleValue(std::move(ModuleValue))
      {
      }

      void attachLoader(ModuleLoader &LoaderValue) noexcept
      {
        Loader = &LoaderValue;
      }

      ModuleProvisionResult provideModule(ir::ModuleId) noexcept override
      {
        ReentrantShutdownErrors = Loader->shutdown();
        return ModuleProvisionResult::found(ModuleValue);
      }

      std::vector<ModuleLoadError> ReentrantShutdownErrors;

    private:
      std::shared_ptr<const ir::Module> ModuleValue;
      ModuleLoader *Loader = nullptr;
    };

    class ReentrantLoadModuleProvider final : public ModuleProvider
    {
    public:
      explicit ReentrantLoadModuleProvider(std::shared_ptr<const ir::Module> ModuleValue)
          : ModuleValue(std::move(ModuleValue))
      {
      }

      void attachLoader(ModuleLoader &LoaderValue) noexcept
      {
        Loader = &LoaderValue;
      }

      ModuleProvisionResult provideModule(ir::ModuleId) noexcept override
      {
        const ModuleLoadResult Nested = Loader->loadModule(ir::ModuleId{2});
        NestedLoadSucceeded = Nested.succeeded();
        NestedLoadError = Nested.error();
        return ModuleProvisionResult::found(ModuleValue);
      }

      bool NestedLoadSucceeded = false;
      ModuleLoadError NestedLoadError;

    private:
      std::shared_ptr<const ir::Module> ModuleValue;
      ModuleLoader *Loader = nullptr;
    };

    class FailingModuleProvider final : public ModuleProvider
    {
    public:
      ModuleProvisionResult provideModule(ir::ModuleId) noexcept override
      {
        return ModuleProvisionResult::failure();
      }
    };

    class RecordingModuleLifecycle final : public ModuleLifecycle
    {
    public:
      void setImports(ir::ModuleId Module, std::vector<ir::ModuleId> Imports)
      {
        Behaviors[Module.value()].Imports = std::move(Imports);
      }

      void setInitializationFailure(ir::ModuleId Module, bool Fails)
      {
        Behaviors[Module.value()].FailsInitialization = Fails;
      }

      std::size_t initializationCount(ir::ModuleId Module) const noexcept
      {
        const auto Found = InitializationCounts.find(Module.value());
        return Found == InitializationCounts.end() ? 0 : Found->second;
      }

      const std::vector<ir::ModuleId> &initializationOrder() const noexcept
      {
        return InitializationOrder;
      }

      const std::vector<ir::ModuleId> &finalizationOrder() const noexcept
      {
        return FinalizationOrder;
      }

      ModuleLoadError initialize(ModuleLoader &Loader, ModuleInstance &Instance) noexcept override
      {
        ++InitializationCounts[Instance.id().value()];
        InitializationOrder.push_back(Instance.id());
        const Behavior BehaviorValue = behavior(Instance.id());
        for (const ir::ModuleId Target : BehaviorValue.Imports)
        {
          const ModuleLoadResult Imported = Loader.importModule(Instance, Target);
          if (!Imported.succeeded())
          {
            return Imported.error();
          }
        }
        return BehaviorValue.FailsInitialization ? ModuleLoadError::failure(ModuleLoadErrorKind::InitializationFailed, Instance.id()) : ModuleLoadError{};
      }

      ModuleLoadError finalize(ModuleLoader &, ModuleInstance &Instance) noexcept override
      {
        FinalizationOrder.push_back(Instance.id());
        return {};
      }

    private:
      struct Behavior
      {
        std::vector<ir::ModuleId> Imports;
        bool FailsInitialization = false;
      };

      Behavior behavior(ir::ModuleId Module) const
      {
        const auto Found = Behaviors.find(Module.value());
        return Found == Behaviors.end() ? Behavior{} : Found->second;
      }

      std::unordered_map<std::size_t, Behavior> Behaviors;
      std::unordered_map<std::size_t, std::size_t> InitializationCounts;
      std::vector<ir::ModuleId> InitializationOrder;
      std::vector<ir::ModuleId> FinalizationOrder;
    };

    class ReentrantShutdownModuleLifecycle final : public ModuleLifecycle
    {
    public:
      ModuleLoadError finalize(ModuleLoader &Loader, ModuleInstance &Instance) noexcept override
      {
        FinalizedModule = Instance.id();
        ReentrantShutdownErrors = Loader.shutdown();
        return {};
      }

      ir::ModuleId FinalizedModule;
      std::vector<ModuleLoadError> ReentrantShutdownErrors;
    };

    class ImportChainModuleLifecycle final : public ModuleLifecycle
    {
    public:
      explicit ImportChainModuleLifecycle(std::size_t LastModuleValue)
          : LastModuleValue(LastModuleValue)
      {
      }

      ModuleLoadError initialize(ModuleLoader &Loader, ModuleInstance &Instance) noexcept override
      {
        FurthestInitializedModuleValue = std::max(FurthestInitializedModuleValue, Instance.id().value());
        if (Instance.id().value() >= LastModuleValue)
        {
          return {};
        }
        const ModuleLoadResult Imported = Loader.importModule(Instance, ir::ModuleId{Instance.id().value() + 1});
        return Imported.succeeded() ? ModuleLoadError{} : Imported.error();
      }

      std::size_t LastModuleValue;
      std::size_t FurthestInitializedModuleValue = 0;
    };

    std::shared_ptr<ir::Module> makeModuleDefinition(ir::IRContext &Context, ir::ModuleId Id)
    {
      auto Result = std::make_shared<ir::Module>(Context);
      Result->Id = Id;
      return Result;
    }

    std::vector<std::size_t> moduleIdValues(const std::vector<ir::ModuleId> &Ids)
    {
      std::vector<std::size_t> Result;
      Result.reserve(Ids.size());
      for (const ir::ModuleId Id : Ids)
      {
        Result.push_back(Id.value());
      }
      return Result;
    }

    std::shared_ptr<ir::Module> parseModule(ir::IRContext &Context, const std::string &Text)
    {
      ir::DeserializeResult Parsed = ir::deserialize(Context, Text);
      if (!Parsed.succeeded() || !Parsed.module().has_value())
      {
        ADD_FAILURE() << "expected module execution test InkIR to deserialize";
        return nullptr;
      }
      return std::make_shared<ir::Module>(std::move(*Parsed.module()));
    }

    void expectIntegerResult(const ExecutionResult &Result, std::uint64_t Expected)
    {
      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      const std::optional<std::uint64_t> Value = Result.returnValue()->integer();
      ASSERT_TRUE(Value.has_value());
      EXPECT_EQ(*Value, Expected);
    }

    const std::string CounterModuleText =
        "inkir 1\n"
        "module 1\n"
        "initializer @init\n"
        "\n"
        "@counter = global mutable i32\n"
        "\n"
        "define void @init() {\n"
        "entry:\n"
        "  store i32 40, byte* @counter\n"
        "  ret void\n"
        "}\n"
        "\n"
        "define i32 @increment() {\n"
        "entry:\n"
        "  %0 = load i32, byte* @counter\n"
        "  %1 = add i32 %0, i32 1\n"
        "  store i32 %1, byte* @counter\n"
        "  ret i32 %1\n"
        "}\n";

    // Verifies that a dynamically reached diamond dependency is initialized once and repeated imports reuse its Ready instance.
    TEST(ModuleLoaderTest, InitializesDiamondDependenciesExactlyOnce)
    {
      ModuleExecutionTestContext Context;
      TestModuleProvider Provider;
      RecordingModuleLifecycle Lifecycle;
      constexpr ir::ModuleId Root{1};
      constexpr ir::ModuleId Left{2};
      constexpr ir::ModuleId Right{3};
      constexpr ir::ModuleId Shared{4};
      Provider.addModule(makeModuleDefinition(Context.IR, Root));
      Provider.addModule(makeModuleDefinition(Context.IR, Left));
      Provider.addModule(makeModuleDefinition(Context.IR, Right));
      Provider.addModule(makeModuleDefinition(Context.IR, Shared));
      Lifecycle.setImports(Root, {Left, Right, Left});
      Lifecycle.setImports(Left, {Shared});
      Lifecycle.setImports(Right, {Shared});
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());

      const ModuleLoadResult Loaded = Loader.loadModule(Root);
      const ModuleLoadResult Repeated = Loader.loadModule(Shared);

      ASSERT_TRUE(Loaded.succeeded());
      ASSERT_TRUE(Repeated.succeeded());
      EXPECT_EQ(Lifecycle.initializationCount(Root), 1U);
      EXPECT_EQ(Lifecycle.initializationCount(Left), 1U);
      EXPECT_EQ(Lifecycle.initializationCount(Right), 1U);
      EXPECT_EQ(Lifecycle.initializationCount(Shared), 1U);
      EXPECT_EQ(Provider.requestCount(Shared), 1U);
      EXPECT_EQ(moduleIdValues(Lifecycle.initializationOrder()), (std::vector<std::size_t>{1, 2, 4, 3}));
      ASSERT_NE(Loaded.instance(), nullptr);
      EXPECT_EQ(moduleIdValues(Loaded.instance()->activeDependencies()), (std::vector<std::size_t>{2, 3}));
    }

    // Verifies that an indirect A-to-B-to-A import cycle fails both instances without running either initializer a second time on retry.
    TEST(ModuleLoaderTest, RejectsAndCachesIndirectImportCycle)
    {
      ModuleExecutionTestContext Context;
      TestModuleProvider Provider;
      RecordingModuleLifecycle Lifecycle;
      constexpr ir::ModuleId First{1};
      constexpr ir::ModuleId Second{2};
      Provider.addModule(makeModuleDefinition(Context.IR, First));
      Provider.addModule(makeModuleDefinition(Context.IR, Second));
      Lifecycle.setImports(First, {Second});
      Lifecycle.setImports(Second, {First});
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());

      const ModuleLoadResult Initial = Loader.loadModule(First);
      const ModuleLoadResult Repeated = Loader.loadModule(First);

      ASSERT_FALSE(Initial.succeeded());
      ASSERT_FALSE(Repeated.succeeded());
      EXPECT_EQ(Initial.error().Kind, ModuleLoadErrorKind::CircularImport);
      EXPECT_EQ(Repeated.error().Kind, ModuleLoadErrorKind::CircularImport);
      EXPECT_EQ(Lifecycle.initializationCount(First), 1U);
      EXPECT_EQ(Lifecycle.initializationCount(Second), 1U);
      EXPECT_EQ(moduleIdValues(Lifecycle.initializationOrder()), (std::vector<std::size_t>{1, 2}));
      ASSERT_NE(Loader.findModule(First), nullptr);
      ASSERT_NE(Loader.findModule(Second), nullptr);
      EXPECT_EQ(Loader.findModule(First)->state(), ModuleState::Failed);
      EXPECT_EQ(Loader.findModule(Second)->state(), ModuleState::Failed);
    }

    // Verifies that a failed initializer remains sticky even when the lifecycle callback is changed to succeed before a repeated load.
    TEST(ModuleLoaderTest, CachesFailedInitialization)
    {
      ModuleExecutionTestContext Context;
      TestModuleProvider Provider;
      RecordingModuleLifecycle Lifecycle;
      constexpr ir::ModuleId Target{7};
      Provider.addModule(makeModuleDefinition(Context.IR, Target));
      Lifecycle.setInitializationFailure(Target, true);
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());

      const ModuleLoadResult Initial = Loader.loadModule(Target);
      Lifecycle.setInitializationFailure(Target, false);
      const ModuleLoadResult Repeated = Loader.loadModule(Target);

      ASSERT_FALSE(Initial.succeeded());
      ASSERT_FALSE(Repeated.succeeded());
      EXPECT_EQ(Initial.error().Kind, ModuleLoadErrorKind::InitializationFailed);
      EXPECT_EQ(Repeated.error().Kind, ModuleLoadErrorKind::InitializationFailed);
      EXPECT_EQ(Lifecycle.initializationCount(Target), 1U);
      EXPECT_EQ(Provider.requestCount(Target), 1U);
      ASSERT_NE(Repeated.instance(), nullptr);
      EXPECT_EQ(Repeated.instance()->state(), ModuleState::Failed);
    }

    // Verifies that a provider reports an operational failure explicitly instead of conflating it with a missing module.
    TEST(ModuleLoaderTest, ReportsExplicitProviderFailure)
    {
      ModuleExecutionTestContext Context;
      FailingModuleProvider Provider;
      ModuleLifecycle Lifecycle;
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());

      const ModuleLoadResult Loaded = Loader.loadModule(ir::ModuleId{9});

      ASSERT_FALSE(Loaded.succeeded());
      EXPECT_EQ(Loaded.error().Kind, ModuleLoadErrorKind::ProviderFailure);
      EXPECT_EQ(Loaded.error().Module, ir::ModuleId{9});
      EXPECT_EQ(Loaded.instance(), nullptr);
    }

    // Verifies that shutdown finalizes a loaded dependency chain from importer to dependency and remains idempotent on repetition.
    TEST(ModuleLoaderTest, FinalizesDependenciesInReverseInitializationOrder)
    {
      ModuleExecutionTestContext Context;
      TestModuleProvider Provider;
      RecordingModuleLifecycle Lifecycle;
      constexpr ir::ModuleId Root{1};
      constexpr ir::ModuleId Middle{2};
      constexpr ir::ModuleId Leaf{3};
      Provider.addModule(makeModuleDefinition(Context.IR, Root));
      Provider.addModule(makeModuleDefinition(Context.IR, Middle));
      Provider.addModule(makeModuleDefinition(Context.IR, Leaf));
      Lifecycle.setImports(Root, {Middle});
      Lifecycle.setImports(Middle, {Leaf});
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());
      ASSERT_TRUE(Loader.loadModule(Root).succeeded());

      const std::vector<ModuleLoadError> InitialShutdown = Loader.shutdown();
      const std::vector<ModuleLoadError> RepeatedShutdown = Loader.shutdown();

      EXPECT_TRUE(InitialShutdown.empty());
      EXPECT_TRUE(RepeatedShutdown.empty());
      EXPECT_EQ(moduleIdValues(Lifecycle.finalizationOrder()), (std::vector<std::size_t>{1, 2, 3}));
      ASSERT_NE(Loader.findModule(Root), nullptr);
      ASSERT_NE(Loader.findModule(Middle), nullptr);
      ASSERT_NE(Loader.findModule(Leaf), nullptr);
      EXPECT_EQ(Loader.findModule(Root)->state(), ModuleState::Stopped);
      EXPECT_EQ(Loader.findModule(Middle)->state(), ModuleState::Stopped);
      EXPECT_EQ(Loader.findModule(Leaf)->state(), ModuleState::Stopped);
    }

    // Verifies that a finalizer can synchronously attempt shutdown on its own loader without deadlocking and receives the dedicated reentrancy error.
    TEST(ModuleLoaderTest, RejectsReentrantShutdownDuringFinalization)
    {
      ModuleExecutionTestContext Context;
      TestModuleProvider Provider;
      ReentrantShutdownModuleLifecycle Lifecycle;
      constexpr ir::ModuleId Target{1};
      Provider.addModule(makeModuleDefinition(Context.IR, Target));
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());
      ASSERT_TRUE(Loader.loadModule(Target).succeeded());

      const std::vector<ModuleLoadError> ShutdownErrors = Loader.shutdown();

      EXPECT_TRUE(ShutdownErrors.empty());
      EXPECT_EQ(Lifecycle.FinalizedModule, Target);
      ASSERT_EQ(Lifecycle.ReentrantShutdownErrors.size(), 1U);
      EXPECT_EQ(Lifecycle.ReentrantShutdownErrors[0].Kind, ModuleLoadErrorKind::ShutdownDuringFinalization);
      EXPECT_EQ(Lifecycle.ReentrantShutdownErrors[0].Module, Target);
      ASSERT_NE(Loader.findModule(Target), nullptr);
      EXPECT_EQ(Loader.findModule(Target)->state(), ModuleState::Stopped);
    }

    // Verifies that a provider can synchronously attempt shutdown during image resolution without deadlocking the active load operation.
    TEST(ModuleLoaderTest, RejectsReentrantShutdownDuringProviderResolution)
    {
      ModuleExecutionTestContext Context;
      constexpr ir::ModuleId Target{1};
      ReentrantShutdownModuleProvider Provider(makeModuleDefinition(Context.IR, Target));
      ModuleLifecycle Lifecycle;
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());
      Provider.attachLoader(Loader);

      const ModuleLoadResult Loaded = Loader.loadModule(Target);

      ASSERT_TRUE(Loaded.succeeded());
      ASSERT_EQ(Provider.ReentrantShutdownErrors.size(), 1U);
      EXPECT_EQ(Provider.ReentrantShutdownErrors[0].Kind, ModuleLoadErrorKind::ShutdownDuringInitialization);
      EXPECT_EQ(Provider.ReentrantShutdownErrors[0].Module, Target);
      EXPECT_TRUE(Loader.shutdown().empty());
    }

    // Verifies that a provider callback cannot reenter the same loader and create resolver wait cycles.
    TEST(ModuleLoaderTest, RejectsReentrantLoadDuringProviderResolution)
    {
      ModuleExecutionTestContext Context;
      constexpr ir::ModuleId Target{1};
      ReentrantLoadModuleProvider Provider(makeModuleDefinition(Context.IR, Target));
      ModuleLifecycle Lifecycle;
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());
      Provider.attachLoader(Loader);

      const ModuleLoadResult Loaded = Loader.loadModule(Target);

      ASSERT_TRUE(Loaded.succeeded());
      EXPECT_FALSE(Provider.NestedLoadSucceeded);
      EXPECT_EQ(Provider.NestedLoadError.Kind, ModuleLoadErrorKind::ProviderFailure);
      EXPECT_EQ(Provider.NestedLoadError.Module, ir::ModuleId{2});
      EXPECT_TRUE(Loader.shutdown().empty());
    }

    // Verifies that a dynamically reached import chain may fill the supported depth but rejects the next module before its initializer runs.
    TEST(ModuleLoaderTest, RejectsImportChainBeyondMaximumDepth)
    {
      ModuleExecutionTestContext Context;
      TestModuleProvider Provider;
      constexpr std::size_t FirstModuleValue = 1;
      constexpr std::size_t ModuleCount = MaximumModuleImportDepth + 1;
      ImportChainModuleLifecycle Lifecycle(ModuleCount);
      for (std::size_t ModuleValue = FirstModuleValue; ModuleValue <= ModuleCount; ++ModuleValue)
      {
        Provider.addModule(makeModuleDefinition(Context.IR, ir::ModuleId{ModuleValue}));
      }
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());

      const ModuleLoadResult Loaded = Loader.loadModule(ir::ModuleId{FirstModuleValue});

      ASSERT_FALSE(Loaded.succeeded());
      EXPECT_EQ(Loaded.error().Kind, ModuleLoadErrorKind::ImportDepthLimitExceeded);
      EXPECT_EQ(Loaded.error().Module, ir::ModuleId{ModuleCount});
      EXPECT_EQ(Loaded.error().RelatedModule, ir::ModuleId{ModuleCount - 1});
      EXPECT_EQ(Lifecycle.FurthestInitializedModuleValue, MaximumModuleImportDepth);
      EXPECT_EQ(Provider.requestCount(ir::ModuleId{ModuleCount}), 1U);
    }

    // Verifies that a conditional initializer import runs at its reached instruction position while the untaken path leaves the dependency uninitialized.
    TEST(ModuleExecutionTest, ExecutesOnlyReachedImportAtInstructionPosition)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module 1\n"
          "initializer @init\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "declare extern \"C\" i32 @select_module_import() [sideeffect]\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  call void @record_module_event(i32 10)\n"
          "  %0 = call i32 @select_module_import()\n"
          "  %1 = icmp ne i32 %0, i32 0\n"
          "  condbr bool %1, load, skip\n"
          "load:\n"
          "  import 2\n"
          "  call void @record_module_event(i32 30)\n"
          "  br exit\n"
          "skip:\n"
          "  call void @record_module_event(i32 40)\n"
          "  br exit\n"
          "exit:\n"
          "  ret void\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module 2\n"
          "initializer @init\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  call void @record_module_event(i32 20)\n"
          "  ret void\n"
          "}\n";
      const std::shared_ptr<ir::Module> Root = parseModule(Context.IR, RootText);
      const std::shared_ptr<ir::Module> Dependency = parseModule(Context.IR, DependencyText);
      ASSERT_NE(Root, nullptr);
      ASSERT_NE(Dependency, nullptr);
      TestModuleProvider Provider;
      Provider.addModule(Root);
      Provider.addModule(Dependency);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("record_module_event", reinterpret_cast<NativeFunctionAddress>(&recordModuleEvent)));
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("select_module_import", reinterpret_cast<NativeFunctionAddress>(&selectModuleImport)));

      RecordedModuleEvents.clear();
      ModuleImportSelector = 0;
      {
        ExecutionEngine Engine(Context.Execution, Provider, ir::ModuleId{1});
        ASSERT_TRUE(Engine.initialize().succeeded());
      }
      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{10, 40}));
      EXPECT_EQ(Provider.requestCount(ir::ModuleId{2}), 0U);

      RecordedModuleEvents.clear();
      ModuleImportSelector = 1;
      {
        ExecutionEngine Engine(Context.Execution, Provider, ir::ModuleId{1});
        ASSERT_TRUE(Engine.initialize().succeeded());
      }
      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{10, 20, 30}));
      EXPECT_EQ(Provider.requestCount(ir::ModuleId{2}), 1U);
    }

    // Verifies that repeatedly executing one import from an initializer loop initializes the target module exactly once.
    TEST(ModuleExecutionTest, InitializesLoopImportedModuleExactlyOnce)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module 1\n"
          "initializer @init\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  br loop\n"
          "loop:\n"
          "  %0 = phi i32 [0, entry], [%2, body]\n"
          "  %1 = icmp lt i32 %0, i32 3\n"
          "  condbr bool %1, body, exit\n"
          "body:\n"
          "  import 2\n"
          "  %2 = add i32 %0, i32 1\n"
          "  br loop\n"
          "exit:\n"
          "  ret void\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module 2\n"
          "initializer @init\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  call void @record_module_event(i32 77)\n"
          "  ret void\n"
          "}\n";
      const std::shared_ptr<ir::Module> Root = parseModule(Context.IR, RootText);
      const std::shared_ptr<ir::Module> Dependency = parseModule(Context.IR, DependencyText);
      ASSERT_NE(Root, nullptr);
      ASSERT_NE(Dependency, nullptr);
      TestModuleProvider Provider;
      Provider.addModule(Root);
      Provider.addModule(Dependency);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("record_module_event", reinterpret_cast<NativeFunctionAddress>(&recordModuleEvent)));
      RecordedModuleEvents.clear();
      ExecutionEngine Engine(Context.Execution, Provider, ir::ModuleId{1});

      const InitializationResult Result = Engine.initialize();

      ASSERT_TRUE(Result.succeeded());
      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{77}));
      EXPECT_EQ(Provider.requestCount(ir::ModuleId{2}), 1U);
    }

    // Verifies that execution reports an indirect initializer import cycle, stops both initializers at the cycle, and caches the failed root state.
    TEST(ModuleExecutionTest, RejectsAndCachesInitializerImportCycle)
    {
      ModuleExecutionTestContext Context;
      const std::string FirstText =
          "inkir 1\n"
          "module 1\n"
          "initializer @init\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  call void @record_module_event(i32 1)\n"
          "  import 2\n"
          "  call void @record_module_event(i32 3)\n"
          "  ret void\n"
          "}\n";
      const std::string SecondText =
          "inkir 1\n"
          "module 2\n"
          "initializer @init\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  call void @record_module_event(i32 2)\n"
          "  import 1\n"
          "  call void @record_module_event(i32 4)\n"
          "  ret void\n"
          "}\n";
      const std::shared_ptr<ir::Module> First = parseModule(Context.IR, FirstText);
      const std::shared_ptr<ir::Module> Second = parseModule(Context.IR, SecondText);
      ASSERT_NE(First, nullptr);
      ASSERT_NE(Second, nullptr);
      TestModuleProvider Provider;
      Provider.addModule(First);
      Provider.addModule(Second);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("record_module_event", reinterpret_cast<NativeFunctionAddress>(&recordModuleEvent)));
      RecordedModuleEvents.clear();
      ExecutionEngine Engine(Context.Execution, Provider, ir::ModuleId{1});

      const InitializationResult Initial = Engine.initialize();
      const InitializationResult Repeated = Engine.initialize();

      ASSERT_FALSE(Initial.succeeded());
      ASSERT_FALSE(Repeated.succeeded());
      ASSERT_EQ(Initial.diagnostics().size(), 1U);
      EXPECT_EQ(Initial.diagnostics()[0].Kind, core::DiagnosticKind::ModuleImportCycle);
      EXPECT_EQ(Repeated.diagnostics(), Initial.diagnostics());
      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{1, 2}));
    }

    // Verifies that a qualified FunctionRef executes ordinary Ink code in a Ready dependency and that a qualified GlobalRef reaches the same module instance.
    TEST(ModuleExecutionTest, ResolvesQualifiedFunctionAndGlobalReferences)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module 1\n"
          "initializer @init\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import 2\n"
          "  ret void\n"
          "}\n"
          "\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  %0 = call i32 module(2, 1)()\n"
          "  %1 = load i32, const byte* global(2, 0)\n"
          "  %2 = add i32 %0, i32 %1\n"
          "  ret i32 %2\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module 2\n"
          "initializer @init\n"
          "\n"
          "@answer = global mutable i32\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  store i32 42, byte* @answer\n"
          "  ret void\n"
          "}\n"
          "\n"
          "define i32 @read_answer() {\n"
          "entry:\n"
          "  %0 = load i32, const byte* @answer\n"
          "  ret i32 %0\n"
          "}\n";
      const std::shared_ptr<ir::Module> Root = parseModule(Context.IR, RootText);
      const std::shared_ptr<ir::Module> Dependency = parseModule(Context.IR, DependencyText);
      ASSERT_NE(Root, nullptr);
      ASSERT_NE(Dependency, nullptr);
      TestModuleProvider Provider;
      Provider.addModule(Root);
      Provider.addModule(Dependency);
      ExecutionEngine Engine(Context.Execution, Provider, ir::ModuleId{1});

      const ExecutionResult Result = Engine.execute("main");

      expectIntegerResult(Result, 84);
    }

    // Verifies that importing a dependency does not permit a qualified GlobalRef to be loaded using a type different from the declared global type.
    TEST(ModuleExecutionTest, RejectsWrongTypedQualifiedGlobalLoad)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module 1\n"
          "initializer @init\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import 2\n"
          "  ret void\n"
          "}\n"
          "\n"
          "define byte @main() {\n"
          "entry:\n"
          "  %0 = load byte, const byte* global(2, 0)\n"
          "  ret byte %0\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module 2\n"
          "\n"
          "@answer = global mutable i32\n";
      const std::shared_ptr<ir::Module> Root = parseModule(Context.IR, RootText);
      const std::shared_ptr<ir::Module> Dependency = parseModule(Context.IR, DependencyText);
      ASSERT_NE(Root, nullptr);
      ASSERT_NE(Dependency, nullptr);
      TestModuleProvider Provider;
      Provider.addModule(Root);
      Provider.addModule(Dependency);
      ExecutionEngine Engine(Context.Execution, Provider, ir::ModuleId{1});

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ModuleGlobalReferenceInvalid);
    }

    // Verifies that a qualified function reference cannot implicitly load or access a module that the caller did not import.
    TEST(ModuleExecutionTest, RejectsQualifiedReferenceWithoutImport)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module 1\n"
          "\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  %0 = call i32 module(2, 0)()\n"
          "  ret i32 %0\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module 2\n"
          "\n"
          "define i32 @answer() {\n"
          "entry:\n"
          "  ret i32 42\n"
          "}\n";
      const std::shared_ptr<ir::Module> Root = parseModule(Context.IR, RootText);
      const std::shared_ptr<ir::Module> Dependency = parseModule(Context.IR, DependencyText);
      ASSERT_NE(Root, nullptr);
      ASSERT_NE(Dependency, nullptr);
      TestModuleProvider Provider;
      Provider.addModule(Root);
      Provider.addModule(Dependency);
      ExecutionEngine Engine(Context.Execution, Provider, ir::ModuleId{1});

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ModuleReferenceUnavailable);
      EXPECT_EQ(Provider.requestCount(ir::ModuleId{2}), 0U);
    }

    // Verifies that a dependency initializer failure stays sticky in one Engine even after its missing native symbol becomes available, while a fresh Engine can initialize it.
    TEST(ModuleExecutionTest, CachesFailedDependencyInitializationPerEngine)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module 1\n"
          "initializer @init\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import 2\n"
          "  ret void\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module 2\n"
          "initializer @init\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "declare extern \"C\" void @late_module_initializer() [sideeffect]\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  call void @record_module_event(i32 9)\n"
          "  call void @late_module_initializer()\n"
          "  ret void\n"
          "}\n";
      const std::shared_ptr<ir::Module> Root = parseModule(Context.IR, RootText);
      const std::shared_ptr<ir::Module> Dependency = parseModule(Context.IR, DependencyText);
      ASSERT_NE(Root, nullptr);
      ASSERT_NE(Dependency, nullptr);
      TestModuleProvider Provider;
      Provider.addModule(Root);
      Provider.addModule(Dependency);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("record_module_event", reinterpret_cast<NativeFunctionAddress>(&recordModuleEvent)));
      RecordedModuleEvents.clear();
      LateModuleInitializerCallCount = 0;
      ExecutionEngine FailedEngine(Context.Execution, Provider, ir::ModuleId{1});

      const InitializationResult Initial = FailedEngine.initialize();
      ASSERT_FALSE(Initial.succeeded());
      ASSERT_EQ(Initial.diagnostics().size(), 1U);
      EXPECT_EQ(Initial.diagnostics()[0].Kind, core::DiagnosticKind::ExternalFunctionNotFound);
      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{9}));
      EXPECT_EQ(LateModuleInitializerCallCount, 0U);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("late_module_initializer", reinterpret_cast<NativeFunctionAddress>(&lateModuleInitializer)));
      const InitializationResult Repeated = FailedEngine.initialize();

      ASSERT_FALSE(Repeated.succeeded());
      EXPECT_EQ(Repeated.diagnostics(), Initial.diagnostics());
      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{9}));
      EXPECT_EQ(LateModuleInitializerCallCount, 0U);
      ExecutionEngine FreshEngine(Context.Execution, Provider, ir::ModuleId{1});
      const InitializationResult Fresh = FreshEngine.initialize();
      ASSERT_TRUE(Fresh.succeeded());
      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{9, 9}));
      EXPECT_EQ(LateModuleInitializerCallCount, 1U);
    }

    // Verifies that explicit Engine shutdown runs the importer finalizer before its dependency and remains idempotent for destruction.
    TEST(ModuleExecutionTest, FinalizesImportedModulesInDependencySafeOrder)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module 1\n"
          "initializer @init\n"
          "finalizer @fini\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import 2\n"
          "  ret void\n"
          "}\n"
          "\n"
          "define void @fini() {\n"
          "entry:\n"
          "  call void @record_module_event(i32 1)\n"
          "  ret void\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module 2\n"
          "finalizer @fini\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "\n"
          "define void @fini() {\n"
          "entry:\n"
          "  call void @record_module_event(i32 2)\n"
          "  ret void\n"
          "}\n";
      const std::shared_ptr<ir::Module> Root = parseModule(Context.IR, RootText);
      const std::shared_ptr<ir::Module> Dependency = parseModule(Context.IR, DependencyText);
      ASSERT_NE(Root, nullptr);
      ASSERT_NE(Dependency, nullptr);
      TestModuleProvider Provider;
      Provider.addModule(Root);
      Provider.addModule(Dependency);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("record_module_event", reinterpret_cast<NativeFunctionAddress>(&recordModuleEvent)));
      RecordedModuleEvents.clear();

      {
        ExecutionEngine Engine(Context.Execution, Provider, ir::ModuleId{1});
        ASSERT_TRUE(Engine.initialize().succeeded());
        const ShutdownResult Shutdown = Engine.shutdown();
        const ShutdownResult Repeated = Engine.shutdown();
        EXPECT_TRUE(Shutdown.succeeded());
        EXPECT_TRUE(Repeated.succeeded());
      }

      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{1, 2}));
    }

    // Verifies that Engine destruction runs a module finalizer while the execution runtime state used by that finalizer is still alive.
    TEST(ModuleExecutionTest, FinalizesModuleDuringEngineDestruction)
    {
      ModuleExecutionTestContext Context;
      const std::string ModuleText =
          "inkir 1\n"
          "module 1\n"
          "finalizer @fini\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "\n"
          "define void @fini() {\n"
          "entry:\n"
          "  call void @record_module_event(i32 5)\n"
          "  ret void\n"
          "}\n";
      const std::shared_ptr<ir::Module> ModuleValue = parseModule(Context.IR, ModuleText);
      ASSERT_NE(ModuleValue, nullptr);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("record_module_event", reinterpret_cast<NativeFunctionAddress>(&recordModuleEvent)));
      RecordedModuleEvents.clear();

      {
        ExecutionEngine Engine(Context.Execution, *ModuleValue);
        ASSERT_TRUE(Engine.initialize().succeeded());
      }

      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{5}));
    }

    // Verifies that explicit shutdown returns the diagnostic produced by a failing module finalizer.
    TEST(ModuleExecutionTest, ReportsFinalizerFailureFromShutdown)
    {
      ModuleExecutionTestContext Context;
      const std::string ModuleText =
          "inkir 1\n"
          "module 1\n"
          "finalizer @fini\n"
          "\n"
          "declare extern \"C\" void @missing_finalizer_symbol() [sideeffect]\n"
          "\n"
          "define void @fini() {\n"
          "entry:\n"
          "  call void @missing_finalizer_symbol()\n"
          "  ret void\n"
          "}\n";
      const std::shared_ptr<ir::Module> ModuleValue = parseModule(Context.IR, ModuleText);
      ASSERT_NE(ModuleValue, nullptr);
      ExecutionEngine Engine(Context.Execution, *ModuleValue);
      ASSERT_TRUE(Engine.initialize().succeeded());

      const ShutdownResult Result = Engine.shutdown();

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ExternalFunctionNotFound);
    }

    // Verifies that module global storage reserves a padded struct's full stride so initializer store and entry load preserve both fields.
    TEST(ModuleExecutionTest, StoresAndLoadsPaddedStructGlobal)
    {
      ModuleExecutionTestContext Context;
      const std::string ModuleText =
          "inkir 1\n"
          "module 1\n"
          "initializer @init\n"
          "\n"
          "%Pair = type {i32, byte}\n"
          "\n"
          "@pair = global mutable %Pair\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  store %Pair {i32 16909060, byte 7}, byte* @pair\n"
          "  ret void\n"
          "}\n"
          "\n"
          "define %Pair @main() {\n"
          "entry:\n"
          "  %0 = load %Pair, const byte* @pair\n"
          "  ret %Pair %0\n"
          "}\n";
      const std::shared_ptr<ir::Module> ModuleValue = parseModule(Context.IR, ModuleText);
      ASSERT_NE(ModuleValue, nullptr);
      ExecutionEngine Engine(Context.Execution, *ModuleValue);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(Result.returnValue()->kind(), RuntimeValueKind::Aggregate);
      ASSERT_EQ(Result.returnValue()->fieldCount(), 2U);
      ASSERT_NE(Result.returnValue()->field(0), nullptr);
      ASSERT_NE(Result.returnValue()->field(1), nullptr);
      EXPECT_EQ(Result.returnValue()->field(0)->integer(), 16909060U);
      EXPECT_EQ(Result.returnValue()->field(1)->integer(), 7U);
    }

    // Verifies that ExecutionResult keeps returned byte-constant and global pointer values queryable after shutdown while their module-owned memory becomes dead.
    TEST(ModuleExecutionTest, InvalidatesReturnedModulePointersAfterShutdown)
    {
      ModuleExecutionTestContext Context;
      const std::string ModuleText =
          "inkir 1\n"
          "module 1\n"
          "\n"
          "@data = private constant [3 x byte] c\"ink\"\n"
          "@value = global mutable i32\n"
          "\n"
          "define const byte* @constant_pointer() {\n"
          "entry:\n"
          "  ret const byte* @data[0]\n"
          "}\n"
          "\n"
          "define byte* @global_pointer() {\n"
          "entry:\n"
          "  ret byte* @value\n"
          "}\n";
      const std::shared_ptr<ir::Module> ModuleValue = parseModule(Context.IR, ModuleText);
      ASSERT_NE(ModuleValue, nullptr);
      ExecutionEngine Engine(Context.Execution, *ModuleValue);

      const ExecutionResult ConstantPointer = Engine.execute("constant_pointer");
      const ExecutionResult GlobalPointer = Engine.execute("global_pointer");
      ASSERT_TRUE(ConstantPointer.succeeded());
      ASSERT_TRUE(GlobalPointer.succeeded());
      ASSERT_NE(ConstantPointer.returnValue(), nullptr);
      ASSERT_NE(GlobalPointer.returnValue(), nullptr);
      EXPECT_TRUE(ConstantPointer.returnValue()->memoryAlive());
      EXPECT_TRUE(GlobalPointer.returnValue()->memoryAlive());

      const ShutdownResult Shutdown = Engine.shutdown();

      EXPECT_TRUE(Shutdown.succeeded());
      EXPECT_EQ(ConstantPointer.returnValue()->kind(), RuntimeValueKind::Pointer);
      EXPECT_EQ(GlobalPointer.returnValue()->kind(), RuntimeValueKind::Pointer);
      EXPECT_FALSE(ConstantPointer.returnValue()->memoryAlive());
      EXPECT_FALSE(GlobalPointer.returnValue()->memoryAlive());
      EXPECT_EQ(ConstantPointer.returnValue()->pointer(), nullptr);
      EXPECT_EQ(GlobalPointer.returnValue()->pointer(), nullptr);
    }

    // Verifies that a module pointer passed from one ExecutionResult into a later entry and returned through an extern keeps provenance until shutdown invalidates both results.
    TEST(ModuleExecutionTest, PreservesModulePointerProvenanceAcrossExecuteArgument)
    {
      ModuleExecutionTestContext Context;
      const std::string ModuleText =
          "inkir 1\n"
          "module 1\n"
          "\n"
          "@data = private constant [1 x byte] c\"x\"\n"
          "\n"
          "declare extern \"C\" const byte* @identity_module_pointer(const byte*)\n"
          "\n"
          "define const byte* @source() {\n"
          "entry:\n"
          "  ret const byte* @data[0]\n"
          "}\n"
          "\n"
          "define const byte* @forward(const byte* %0) {\n"
          "entry:\n"
          "  %1 = call const byte* @identity_module_pointer(const byte* %0)\n"
          "  ret const byte* %1\n"
          "}\n";
      const std::shared_ptr<ir::Module> ModuleValue = parseModule(Context.IR, ModuleText);
      ASSERT_NE(ModuleValue, nullptr);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("identity_module_pointer", reinterpret_cast<NativeFunctionAddress>(&identityModulePointer)));
      ExecutionEngine Engine(Context.Execution, *ModuleValue);

      const ExecutionResult Source = Engine.execute("source");
      ASSERT_TRUE(Source.succeeded());
      ASSERT_NE(Source.returnValue(), nullptr);
      const ExecutionResult Forwarded = Engine.execute("forward", {Source.returnValue()});
      ASSERT_TRUE(Forwarded.succeeded());
      ASSERT_NE(Forwarded.returnValue(), nullptr);
      EXPECT_TRUE(Source.returnValue()->memoryAlive());
      EXPECT_TRUE(Forwarded.returnValue()->memoryAlive());

      const ShutdownResult Shutdown = Engine.shutdown();

      EXPECT_TRUE(Shutdown.succeeded());
      EXPECT_FALSE(Source.returnValue()->memoryAlive());
      EXPECT_FALSE(Forwarded.returnValue()->memoryAlive());
      EXPECT_EQ(Source.returnValue()->pointer(), nullptr);
      EXPECT_EQ(Forwarded.returnValue()->pointer(), nullptr);
    }

    // Verifies that native pointer re-association preserves an interior module-backing offset and invalidates it on shutdown.
    TEST(ModuleExecutionTest, PreservesInteriorModulePointerProvenanceFromNativeResult)
    {
      ModuleExecutionTestContext Context;
      const std::string ModuleText =
          "inkir 1\n"
          "module 1\n"
          "\n"
          "@data = private constant [2 x byte] c\"xy\"\n"
          "\n"
          "declare extern \"C\" const byte* @advance_module_pointer(const byte*)\n"
          "\n"
          "define const byte* @main() {\n"
          "entry:\n"
          "  %0 = call const byte* @advance_module_pointer(const byte* @data[0])\n"
          "  ret const byte* %0\n"
          "}\n";
      const std::shared_ptr<ir::Module> ModuleValue = parseModule(Context.IR, ModuleText);
      ASSERT_NE(ModuleValue, nullptr);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("advance_module_pointer", reinterpret_cast<NativeFunctionAddress>(&advanceModulePointer)));
      ExecutionEngine Engine(Context.Execution, *ModuleValue);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      ASSERT_NE(Result.returnValue()->pointer(), nullptr);
      EXPECT_EQ(*static_cast<const char *>(Result.returnValue()->pointer()), 'y');
      EXPECT_EQ(runtimePointerByteOffset(*Result.returnValue()), 1U);
      EXPECT_TRUE(Engine.shutdown().succeeded());
      EXPECT_FALSE(Result.returnValue()->memoryAlive());
      EXPECT_EQ(Result.returnValue()->pointer(), nullptr);
    }

    // Verifies that a native result cannot downgrade a managed module byte constant from const byte* to byte*.
    TEST(ModuleExecutionTest, RejectsMutableNativeAliasOfModuleByteConstant)
    {
      ModuleExecutionTestContext Context;
      const std::string ModuleText =
          "inkir 1\n"
          "module 1\n"
          "\n"
          "@data = private constant [1 x byte] c\"x\"\n"
          "\n"
          "declare extern \"C\" byte* @downgrade_module_pointer(const byte*)\n"
          "\n"
          "define byte* @main() {\n"
          "entry:\n"
          "  %0 = call byte* @downgrade_module_pointer(const byte* @data[0])\n"
          "  ret byte* %0\n"
          "}\n";
      const std::shared_ptr<ir::Module> ModuleValue = parseModule(Context.IR, ModuleText);
      ASSERT_NE(ModuleValue, nullptr);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("downgrade_module_pointer", reinterpret_cast<NativeFunctionAddress>(&downgradeModulePointer)));
      ExecutionEngine Engine(Context.Execution, *ModuleValue);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::NativeResultUnmarshalFailed);
    }

    // Verifies that an immutable global is writable by its initializer but sealed before native code can return a mutable alias.
    TEST(ModuleExecutionTest, SealsImmutableGlobalAfterInitialization)
    {
      ModuleExecutionTestContext Context;
      const std::string ModuleText =
          "inkir 1\n"
          "module 1\n"
          "initializer @init\n"
          "\n"
          "@value = global constant i32\n"
          "\n"
          "declare extern \"C\" byte* @downgrade_immutable_global(const byte*)\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  store i32 42, byte* @value\n"
          "  ret void\n"
          "}\n"
          "\n"
          "define byte* @main() {\n"
          "entry:\n"
          "  %0 = call byte* @downgrade_immutable_global(const byte* @value)\n"
          "  ret byte* %0\n"
          "}\n";
      const std::shared_ptr<ir::Module> ModuleValue = parseModule(Context.IR, ModuleText);
      ASSERT_NE(ModuleValue, nullptr);
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("downgrade_immutable_global", reinterpret_cast<NativeFunctionAddress>(&downgradeModulePointer)));
      ExecutionEngine Engine(Context.Execution, *ModuleValue);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::NativeResultUnmarshalFailed);
    }

    // Verifies that a pointer derived from a reached inline string is owned by its ModuleInstance and becomes dead without dangling after shutdown.
    TEST(ModuleExecutionTest, InvalidatesReturnedInlineStringPointerAfterShutdown)
    {
      ModuleExecutionTestContext Context;
      const std::string ModuleText =
          "inkir 1\n"
          "module 1\n"
          "\n"
          "define const byte* @main() {\n"
          "entry:\n"
          "  %0 = slice.data const byte* const byte[] c\"ink\"\n"
          "  ret const byte* %0\n"
          "}\n";
      const std::shared_ptr<ir::Module> ModuleValue = parseModule(Context.IR, ModuleText);
      ASSERT_NE(ModuleValue, nullptr);
      ExecutionEngine Engine(Context.Execution, *ModuleValue);

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(Result.returnValue()->kind(), RuntimeValueKind::Pointer);
      EXPECT_EQ(Result.returnValue()->byteLength(), 3U);
      ASSERT_NE(Result.returnValue()->pointer(), nullptr);
      EXPECT_EQ(std::string(static_cast<const char *>(Result.returnValue()->pointer()), 3), "ink");
      EXPECT_TRUE(Engine.shutdown().succeeded());
      EXPECT_FALSE(Result.returnValue()->memoryAlive());
      EXPECT_EQ(Result.returnValue()->pointer(), nullptr);
    }

    // Verifies that initializer-created global storage survives repeated entry execution and is not reset by repeated Engine initialization.
    TEST(ModuleExecutionTest, PersistsGlobalStateAcrossExecuteCalls)
    {
      ModuleExecutionTestContext Context;
      const std::shared_ptr<ir::Module> ModuleValue = parseModule(Context.IR, CounterModuleText);
      ASSERT_NE(ModuleValue, nullptr);
      ExecutionEngine Engine(Context.Execution, *ModuleValue);

      const InitializationResult Initialization = Engine.initialize();
      const ExecutionResult First = Engine.execute("increment");
      const InitializationResult RepeatedInitialization = Engine.initialize();
      const ExecutionResult Second = Engine.execute("increment");

      ASSERT_TRUE(Initialization.succeeded());
      ASSERT_TRUE(RepeatedInitialization.succeeded());
      expectIntegerResult(First, 41);
      expectIntegerResult(Second, 42);
    }

    // Verifies that two Engines sharing one immutable module definition own independent initializer state and global storage.
    TEST(ModuleExecutionTest, IsolatesGlobalStateBetweenEngines)
    {
      ModuleExecutionTestContext Context;
      const std::shared_ptr<ir::Module> ModuleValue = parseModule(Context.IR, CounterModuleText);
      ASSERT_NE(ModuleValue, nullptr);
      ExecutionEngine FirstEngine(Context.Execution, *ModuleValue);
      ExecutionEngine SecondEngine(Context.Execution, *ModuleValue);

      const ExecutionResult FirstEngineInitial = FirstEngine.execute("increment");
      const ExecutionResult SecondEngineInitial = SecondEngine.execute("increment");
      const ExecutionResult FirstEngineRepeated = FirstEngine.execute("increment");

      expectIntegerResult(FirstEngineInitial, 41);
      expectIntegerResult(SecondEngineInitial, 41);
      expectIntegerResult(FirstEngineRepeated, 42);
    }
  } // namespace
} // namespace ink::execution
