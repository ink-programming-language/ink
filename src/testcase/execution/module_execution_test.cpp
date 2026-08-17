#include "ink/execution/execution_engine.h"
#include "ink/execution/module/compiling_module_provider.h"
#include "ink/execution/module/module_loader.h"
#include "ink/ir/model/context.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
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

    std::string moduleName(std::size_t Value)
    {
      return "module.m" + std::to_string(Value);
    }

    std::size_t moduleValue(std::string_view Name)
    {
      constexpr std::string_view Prefix = "module.m";
      if (Name.size() <= Prefix.size() || Name.substr(0, Prefix.size()) != Prefix)
      {
        return 0;
      }
      std::size_t Value = 0;
      const char *Begin = Name.data() + Prefix.size();
      const char *End = Name.data() + Name.size();
      const std::from_chars_result Parsed = std::from_chars(Begin, End, Value);
      return Parsed.ec == std::errc{} && Parsed.ptr == End ? Value : 0;
    }

    class TestModuleProvider final : public ModuleProvider
    {
      public:
        void addModule(std::shared_ptr<const ir::Module> ModuleValue)
        {
          Modules.push_back(std::move(ModuleValue));
        }

        ir::IRContext &irContext() noexcept override
        {
          return Modules.front()->context();
        }

        ModuleProvisionResult provideModule(const ir::Name &ModuleName) noexcept override
        {
          ++RequestCounts[ModuleName.str()];
          const auto Found = std::find_if(Modules.begin(), Modules.end(), [ModuleName](const std::shared_ptr<const ir::Module> &ModuleValue)
                                          {
                                            return ModuleValue != nullptr && ModuleValue->Name.has_value() && *ModuleValue->Name == ModuleName;
                                          });
          return Found == Modules.end() ? ModuleProvisionResult::notFound() : ModuleProvisionResult::found(*Found);
        }

        std::size_t requestCount(std::string_view ModuleName) const
        {
          const auto Found = RequestCounts.find(std::string(ModuleName));
          return Found == RequestCounts.end() ? 0 : Found->second;
        }

      private:
        std::vector<std::shared_ptr<const ir::Module>> Modules;
        std::unordered_map<std::string, std::size_t> RequestCounts;
    };

    class FixedModuleProvider final : public ModuleProvider
    {
      public:
        explicit FixedModuleProvider(std::shared_ptr<const ir::Module> ModuleValue)
            : ModuleValue(std::move(ModuleValue))
        {
        }

        ModuleProvisionResult provideModule(const ir::Name &) noexcept override
        {
          return ModuleProvisionResult::found(ModuleValue);
        }

        ir::IRContext &irContext() noexcept override
        {
          return ModuleValue->context();
        }

      private:
        std::shared_ptr<const ir::Module> ModuleValue;
    };

    class MismatchedContextModuleProvider final : public ModuleProvider
    {
      public:
        MismatchedContextModuleProvider(ir::IRContext &DeclaredContext, std::shared_ptr<const ir::Module> ModuleValue)
            : DeclaredContext(DeclaredContext),
              ModuleValue(std::move(ModuleValue))
        {
        }

        ir::IRContext &irContext() noexcept override
        {
          return DeclaredContext;
        }

        ModuleProvisionResult provideModule(const ir::Name &) noexcept override
        {
          return ModuleProvisionResult::found(ModuleValue);
        }

      private:
        ir::IRContext &DeclaredContext;
        std::shared_ptr<const ir::Module> ModuleValue;
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

        ir::IRContext &irContext() noexcept override
        {
          return ModuleValue->context();
        }

        ModuleProvisionResult provideModule(const ir::Name &) noexcept override
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

        ir::IRContext &irContext() noexcept override
        {
          return ModuleValue->context();
        }

        ModuleProvisionResult provideModule(const ir::Name &) noexcept override
        {
          const ModuleLoadResult Nested = Loader->loadModule(moduleName(2));
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
        explicit FailingModuleProvider(ir::IRContext &Context) noexcept
            : Context(Context)
        {
        }

        ir::IRContext &irContext() noexcept override
        {
          return Context;
        }

        ModuleProvisionResult provideModule(const ir::Name &) noexcept override
        {
          return ModuleProvisionResult::failure();
        }

      private:
        ir::IRContext &Context;
    };

    class TextModuleCompiler final : public ir::ModuleCompiler
    {
      public:
        void addModule(std::string Name, std::string Text)
        {
          Modules.emplace(std::move(Name), std::move(Text));
        }

        void addCompilationDependency(std::string ModuleName, std::string Dependency)
        {
          Dependencies[std::move(ModuleName)].push_back(std::move(Dependency));
        }

        ir::ModuleCompilationResult compileModule(ir::CompilationSession &Session, const ir::Name &ModuleName) noexcept override
        {
          ++CompilationCounts[ModuleName.str()];
          const auto Found = Modules.find(ModuleName.str());
          if (Found == Modules.end())
          {
            return ir::ModuleCompilationResult::notFound();
          }
          const auto ModuleDependencies = Dependencies.find(ModuleName.str());
          if (ModuleDependencies != Dependencies.end())
          {
            for (const std::string &Dependency : ModuleDependencies->second)
            {
              ir::ModuleCompilationResult CompiledDependency = Session.getOrCompileModule(Dependency);
              if (CompiledDependency.Status != ir::ModuleCompilationStatus::Found)
              {
                return ir::ModuleCompilationResult::failure(std::move(CompiledDependency.Diagnostics));
              }
            }
          }
          ir::DeserializeResult Parsed = ir::deserialize(Session.irContext(), Found->second);
          if (!Parsed.succeeded())
          {
            return ir::ModuleCompilationResult::failure(Parsed.diagnostics());
          }
          return ir::ModuleCompilationResult::found(std::make_shared<ir::Module>(std::move(*Parsed.module())));
        }

        std::size_t compilationCount(std::string_view ModuleName) const
        {
          const auto Found = CompilationCounts.find(std::string(ModuleName));
          return Found == CompilationCounts.end() ? 0 : Found->second;
        }

      private:
        std::unordered_map<std::string, std::string> Modules;
        std::unordered_map<std::string, std::vector<std::string>> Dependencies;
        std::unordered_map<std::string, std::size_t> CompilationCounts;
    };

    class RecordingModuleLifecycle final : public ModuleLifecycle
    {
      public:
        void setImports(std::string Module, std::vector<std::string> Imports)
        {
          Behaviors[std::move(Module)].Imports = std::move(Imports);
        }

        void setInitializationFailure(std::string Module, bool Fails)
        {
          Behaviors[std::move(Module)].FailsInitialization = Fails;
        }

        std::size_t initializationCount(std::string_view Module) const
        {
          const auto Found = InitializationCounts.find(std::string(Module));
          return Found == InitializationCounts.end() ? 0 : Found->second;
        }

        const std::vector<std::string> &initializationOrder() const noexcept
        {
          return InitializationOrder;
        }

        const std::vector<std::string> &finalizationOrder() const noexcept
        {
          return FinalizationOrder;
        }

        ModuleLoadError initialize(ModuleLoader &Loader, ModuleInstance &Instance) noexcept override
        {
          ++InitializationCounts[std::string(Instance.name())];
          InitializationOrder.push_back(std::string(Instance.name()));
          const Behavior BehaviorValue = behavior(Instance.name());
          for (const std::string &Target : BehaviorValue.Imports)
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
          FinalizationOrder.push_back(std::string(Instance.name()));
          return {};
        }

      private:
        struct Behavior
        {
            std::vector<std::string> Imports;
            bool FailsInitialization = false;
        };

        Behavior behavior(std::string_view Module) const
        {
          const auto Found = Behaviors.find(std::string(Module));
          return Found == Behaviors.end() ? Behavior{} : Found->second;
        }

        std::unordered_map<std::string, Behavior> Behaviors;
        std::unordered_map<std::string, std::size_t> InitializationCounts;
        std::vector<std::string> InitializationOrder;
        std::vector<std::string> FinalizationOrder;
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

        ModuleId FinalizedModule;
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
          const std::size_t CurrentModuleValue = moduleValue(Instance.name());
          FurthestInitializedModuleValue = std::max(FurthestInitializedModuleValue, CurrentModuleValue);
          if (CurrentModuleValue >= LastModuleValue)
          {
            return {};
          }
          const ModuleLoadResult Imported = Loader.importModule(Instance, moduleName(CurrentModuleValue + 1));
          return Imported.succeeded() ? ModuleLoadError{} : Imported.error();
        }

        std::size_t LastModuleValue;
        std::size_t FurthestInitializedModuleValue = 0;
    };

    std::shared_ptr<ir::Module> makeModuleDefinition(ir::IRContext &Context, std::string Name)
    {
      auto Result = std::make_shared<ir::Module>(Context);
      Result->Name = std::move(Name);
      return Result;
    }

    std::vector<std::string> moduleNames(const ModuleLoader &Loader, const std::vector<ModuleId> &Ids)
    {
      std::vector<std::string> Result;
      Result.reserve(Ids.size());
      for (const ModuleId Id : Ids)
      {
        Result.push_back(Loader.moduleName(Id).str());
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
        "module module.counter\n"
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

    // Verifies that ModuleId is a strongly typed execution-only handle with an explicit invalid state.
    TEST(ModuleLoaderTest, EncapsulatesRuntimeModuleIds)
    {
      constexpr ModuleId First{0};
      constexpr ModuleId Second{1};

      static_assert(!std::is_convertible_v<std::size_t, ModuleId>);
      static_assert(!ModuleId{}.valid());
      static_assert(First.valid());
      static_assert(First != Second);
    }

    // Verifies that one loader assigns one reusable runtime ID per canonical name and distinct IDs to distinct names.
    TEST(ModuleLoaderTest, AssignsRuntimeIdsByCanonicalName)
    {
      ModuleExecutionTestContext Context;
      TestModuleProvider Provider;
      ModuleLifecycle Lifecycle;
      const std::string FirstName = moduleName(1);
      const std::string SecondName = moduleName(2);
      Provider.addModule(makeModuleDefinition(Context.IR, FirstName));
      Provider.addModule(makeModuleDefinition(Context.IR, SecondName));
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());

      const ModuleLoadResult First = Loader.loadModule(FirstName);
      const ModuleLoadResult Repeated = Loader.loadModule(FirstName);
      const ModuleLoadResult Second = Loader.loadModule(SecondName);

      ASSERT_TRUE(First.succeeded());
      ASSERT_TRUE(Repeated.succeeded());
      ASSERT_TRUE(Second.succeeded());
      EXPECT_EQ(First.instance()->id(), Repeated.instance()->id());
      EXPECT_NE(First.instance()->id(), Second.instance()->id());
      EXPECT_EQ(First.instance()->name(), FirstName);
      EXPECT_EQ(Loader.moduleName(Second.instance()->id()), SecondName);
    }

    // Verifies that provider identity validation compares canonical names rather than serialized numeric IDs.
    TEST(ModuleLoaderTest, RejectsProviderCanonicalNameMismatch)
    {
      ModuleExecutionTestContext Context;
      const std::string RequestedName = moduleName(1);
      const std::string ActualName = moduleName(2);
      FixedModuleProvider Provider(makeModuleDefinition(Context.IR, ActualName));
      ModuleLifecycle Lifecycle;
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());

      const ModuleLoadResult Loaded = Loader.loadModule(RequestedName);

      ASSERT_FALSE(Loaded.succeeded());
      EXPECT_EQ(Loaded.error().Kind, ModuleLoadErrorKind::ModuleIdentityMismatch);
      EXPECT_EQ(Loader.moduleName(Loaded.error().Module), RequestedName);
      EXPECT_EQ(Loader.moduleName(Loaded.error().RelatedModule), ActualName);
    }

    // Verifies that a dynamically reached diamond dependency is initialized once and repeated imports reuse its Ready instance.
    TEST(ModuleLoaderTest, InitializesDiamondDependenciesExactlyOnce)
    {
      ModuleExecutionTestContext Context;
      TestModuleProvider Provider;
      RecordingModuleLifecycle Lifecycle;
      const std::string Root = moduleName(1);
      const std::string Left = moduleName(2);
      const std::string Right = moduleName(3);
      const std::string Shared = moduleName(4);
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
      EXPECT_EQ(Lifecycle.initializationOrder(), (std::vector<std::string>{Root, Left, Shared, Right}));
      ASSERT_NE(Loaded.instance(), nullptr);
      EXPECT_EQ(moduleNames(Loader, Loaded.instance()->activeDependencies()), (std::vector<std::string>{Left, Right}));
    }

    // Verifies that an indirect A-to-B-to-A import cycle fails both instances without running either initializer a second time on retry.
    TEST(ModuleLoaderTest, RejectsAndCachesIndirectImportCycle)
    {
      ModuleExecutionTestContext Context;
      TestModuleProvider Provider;
      RecordingModuleLifecycle Lifecycle;
      const std::string First = moduleName(1);
      const std::string Second = moduleName(2);
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
      EXPECT_EQ(Lifecycle.initializationOrder(), (std::vector<std::string>{First, Second}));
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
      const std::string Target = moduleName(7);
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
      FailingModuleProvider Provider(Context.IR);
      ModuleLifecycle Lifecycle;
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());

      const ModuleLoadResult Loaded = Loader.loadModule(moduleName(9));

      ASSERT_FALSE(Loaded.succeeded());
      EXPECT_EQ(Loaded.error().Kind, ModuleLoadErrorKind::ProviderFailure);
      EXPECT_EQ(Loader.moduleName(Loaded.error().Module), moduleName(9));
      EXPECT_EQ(Loaded.instance(), nullptr);
    }

    // Verifies that shutdown finalizes a loaded dependency chain from importer to dependency and remains idempotent on repetition.
    TEST(ModuleLoaderTest, FinalizesDependenciesInReverseInitializationOrder)
    {
      ModuleExecutionTestContext Context;
      TestModuleProvider Provider;
      RecordingModuleLifecycle Lifecycle;
      const std::string Root = moduleName(1);
      const std::string Middle = moduleName(2);
      const std::string Leaf = moduleName(3);
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
      EXPECT_EQ(Lifecycle.finalizationOrder(), (std::vector<std::string>{Root, Middle, Leaf}));
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
      const std::string Target = moduleName(1);
      Provider.addModule(makeModuleDefinition(Context.IR, Target));
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());
      ASSERT_TRUE(Loader.loadModule(Target).succeeded());

      const std::vector<ModuleLoadError> ShutdownErrors = Loader.shutdown();

      EXPECT_TRUE(ShutdownErrors.empty());
      EXPECT_EQ(Loader.moduleName(Lifecycle.FinalizedModule), Target);
      ASSERT_EQ(Lifecycle.ReentrantShutdownErrors.size(), 1U);
      EXPECT_EQ(Lifecycle.ReentrantShutdownErrors[0].Kind, ModuleLoadErrorKind::ShutdownDuringFinalization);
      EXPECT_EQ(Loader.moduleName(Lifecycle.ReentrantShutdownErrors[0].Module), Target);
      ASSERT_NE(Loader.findModule(Target), nullptr);
      EXPECT_EQ(Loader.findModule(Target)->state(), ModuleState::Stopped);
    }

    // Verifies that a provider can synchronously attempt shutdown during image resolution without deadlocking the active load operation.
    TEST(ModuleLoaderTest, RejectsReentrantShutdownDuringProviderResolution)
    {
      ModuleExecutionTestContext Context;
      const std::string Target = moduleName(1);
      ReentrantShutdownModuleProvider Provider(makeModuleDefinition(Context.IR, Target));
      ModuleLifecycle Lifecycle;
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());
      Provider.attachLoader(Loader);

      const ModuleLoadResult Loaded = Loader.loadModule(Target);

      ASSERT_TRUE(Loaded.succeeded());
      ASSERT_EQ(Provider.ReentrantShutdownErrors.size(), 1U);
      EXPECT_EQ(Provider.ReentrantShutdownErrors[0].Kind, ModuleLoadErrorKind::ShutdownDuringInitialization);
      EXPECT_EQ(Loader.moduleName(Provider.ReentrantShutdownErrors[0].Module), Target);
      EXPECT_TRUE(Loader.shutdown().empty());
    }

    // Verifies that a provider callback cannot reenter the same loader and create resolver wait cycles.
    TEST(ModuleLoaderTest, RejectsReentrantLoadDuringProviderResolution)
    {
      ModuleExecutionTestContext Context;
      const std::string Target = moduleName(1);
      ReentrantLoadModuleProvider Provider(makeModuleDefinition(Context.IR, Target));
      ModuleLifecycle Lifecycle;
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());
      Provider.attachLoader(Loader);

      const ModuleLoadResult Loaded = Loader.loadModule(Target);

      ASSERT_TRUE(Loaded.succeeded());
      EXPECT_FALSE(Provider.NestedLoadSucceeded);
      EXPECT_EQ(Provider.NestedLoadError.Kind, ModuleLoadErrorKind::ProviderFailure);
      EXPECT_EQ(Loader.moduleName(Provider.NestedLoadError.Module), moduleName(2));
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
        Provider.addModule(makeModuleDefinition(Context.IR, moduleName(ModuleValue)));
      }
      ModuleLoader Loader(Provider, Lifecycle, Context.Compilation.targetContext());

      const ModuleLoadResult Loaded = Loader.loadModule(moduleName(FirstModuleValue));

      ASSERT_FALSE(Loaded.succeeded());
      EXPECT_EQ(Loaded.error().Kind, ModuleLoadErrorKind::ImportDepthLimitExceeded);
      EXPECT_EQ(Loader.moduleName(Loaded.error().Module), moduleName(ModuleCount));
      EXPECT_EQ(Loader.moduleName(Loaded.error().RelatedModule), moduleName(ModuleCount - 1));
      EXPECT_EQ(Lifecycle.FurthestInitializedModuleValue, MaximumModuleImportDepth);
      EXPECT_EQ(Provider.requestCount(moduleName(ModuleCount)), 1U);
    }

    // Verifies that a conditional initializer import runs at its reached instruction position while the untaken path leaves the dependency uninitialized.
    TEST(ModuleExecutionTest, ExecutesOnlyReachedImportAtInstructionPosition)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
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
          "  import module.m2\n"
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
          "module module.m2\n"
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
        ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));
        ASSERT_TRUE(Engine.initialize().succeeded());
      }
      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{10, 40}));
      EXPECT_EQ(Provider.requestCount(moduleName(2)), 0U);

      RecordedModuleEvents.clear();
      ModuleImportSelector = 1;
      {
        ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));
        ASSERT_TRUE(Engine.initialize().succeeded());
      }
      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{10, 20, 30}));
      EXPECT_EQ(Provider.requestCount(moduleName(2)), 1U);
    }

    // Verifies that repeatedly executing one import from an initializer loop initializes the target module exactly once.
    TEST(ModuleExecutionTest, InitializesLoopImportedModuleExactlyOnce)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
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
          "  import module.m2\n"
          "  %2 = add i32 %0, i32 1\n"
          "  br loop\n"
          "exit:\n"
          "  ret void\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module module.m2\n"
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
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));

      const InitializationResult Result = Engine.initialize();

      ASSERT_TRUE(Result.succeeded());
      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{77}));
      EXPECT_EQ(Provider.requestCount(moduleName(2)), 1U);
    }

    // Verifies that execution reports an indirect initializer import cycle, stops both initializers at the cycle, and caches the failed root state.
    TEST(ModuleExecutionTest, RejectsAndCachesInitializerImportCycle)
    {
      ModuleExecutionTestContext Context;
      const std::string FirstText =
          "inkir 1\n"
          "module module.m1\n"
          "initializer @init\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  call void @record_module_event(i32 1)\n"
          "  import module.m2\n"
          "  call void @record_module_event(i32 3)\n"
          "  ret void\n"
          "}\n";
      const std::string SecondText =
          "inkir 1\n"
          "module module.m2\n"
          "initializer @init\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  call void @record_module_event(i32 2)\n"
          "  import module.m1\n"
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
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));

      const InitializationResult Initial = Engine.initialize();
      const InitializationResult Repeated = Engine.initialize();

      ASSERT_FALSE(Initial.succeeded());
      ASSERT_FALSE(Repeated.succeeded());
      ASSERT_EQ(Initial.diagnostics().size(), 1U);
      EXPECT_EQ(Initial.diagnostics()[0].Kind, core::DiagnosticKind::ModuleImportCycle);
      EXPECT_EQ(Repeated.diagnostics(), Initial.diagnostics());
      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{1, 2}));
    }

    // Verifies that imported functions and globals bind by name and that global reads and writes reach the dependency's storage.
    TEST(ModuleExecutionTest, ResolvesImportedFunctionAndGlobal)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
          "initializer @init\n"
          "\n"
          "declare import global mutable i32 @dependency.answer from module module.m2, symbol @answer\n"
          "\n"
          "declare import i32 @dependency.read_answer() from module module.m2, symbol @read_answer\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import module.m2\n"
          "  %0 = call i32 @dependency.read_answer()\n"
          "  ret void\n"
          "}\n"
          "\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  store i32 43, byte* @dependency.answer\n"
          "  %0 = call i32 @dependency.read_answer()\n"
          "  %1 = load i32, const byte* @dependency.answer\n"
          "  %2 = add i32 %0, i32 %1\n"
          "  ret i32 %2\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module module.m2\n"
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
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));

      const ExecutionResult Result = Engine.execute("main");

      expectIntegerResult(Result, 86);
    }

    // Verifies that module initialization fails when an imported function names no function in the imported dependency.
    TEST(ModuleExecutionTest, RejectsMissingImportedFunctionTarget)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
          "initializer @init\n"
          "\n"
          "declare import i32 @dependency.missing() from module module.m2, symbol @missing\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import module.m2\n"
          "  ret void\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module module.m2\n"
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
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));

      const InitializationResult Result = Engine.initialize();

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ImportedFunctionNotFound);
      EXPECT_EQ(Provider.requestCount(moduleName(2)), 1U);
    }

    // Verifies that module initialization rejects an imported declaration whose signature differs from the resolved dependency function.
    TEST(ModuleExecutionTest, RejectsImportedFunctionSignatureMismatch)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
          "initializer @init\n"
          "\n"
          "declare import i32 @dependency.answer(i32) from module module.m2, symbol @answer\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import module.m2\n"
          "  ret void\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module module.m2\n"
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
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));

      const InitializationResult Result = Engine.initialize();

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ImportedFunctionSignatureMismatch);
    }

    // Verifies that binding rejects an imported global declaration whose value type differs from the resolved target.
    TEST(ModuleExecutionTest, RejectsImportedGlobalTypeMismatch)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
          "initializer @init\n"
          "\n"
          "declare import global constant byte @dependency.answer from module module.m2, symbol @answer\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import module.m2\n"
          "  ret void\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module module.m2\n"
          "\n"
          "@answer = global mutable i32\n";
      const std::shared_ptr<ir::Module> Root = parseModule(Context.IR, RootText);
      const std::shared_ptr<ir::Module> Dependency = parseModule(Context.IR, DependencyText);
      ASSERT_NE(Root, nullptr);
      ASSERT_NE(Dependency, nullptr);
      TestModuleProvider Provider;
      Provider.addModule(Root);
      Provider.addModule(Dependency);
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));

      const InitializationResult Result = Engine.initialize();

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ImportedGlobalTypeMismatch);
    }

    // Verifies that binding fails when an imported global names no global in the imported dependency.
    TEST(ModuleExecutionTest, RejectsMissingImportedGlobalTarget)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
          "initializer @init\n"
          "\n"
          "declare import global constant i32 @dependency.missing from module module.m2, symbol @missing\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import module.m2\n"
          "  ret void\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module module.m2\n"
          "\n"
          "@answer = global constant i32\n";
      const std::shared_ptr<ir::Module> Root = parseModule(Context.IR, RootText);
      const std::shared_ptr<ir::Module> Dependency = parseModule(Context.IR, DependencyText);
      ASSERT_NE(Root, nullptr);
      ASSERT_NE(Dependency, nullptr);
      TestModuleProvider Provider;
      Provider.addModule(Root);
      Provider.addModule(Dependency);
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));

      const InitializationResult Result = Engine.initialize();

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ImportedGlobalNotFound);
    }

    // Verifies that a mutable imported capability cannot bind to constant dependency storage.
    TEST(ModuleExecutionTest, RejectsImportedGlobalMutabilityEscalation)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
          "initializer @init\n"
          "\n"
          "declare import global mutable i32 @dependency.answer from module module.m2, symbol @answer\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import module.m2\n"
          "  ret void\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module module.m2\n"
          "\n"
          "@answer = global constant i32\n";
      const std::shared_ptr<ir::Module> Root = parseModule(Context.IR, RootText);
      const std::shared_ptr<ir::Module> Dependency = parseModule(Context.IR, DependencyText);
      ASSERT_NE(Root, nullptr);
      ASSERT_NE(Dependency, nullptr);
      TestModuleProvider Provider;
      Provider.addModule(Root);
      Provider.addModule(Dependency);
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));

      const InitializationResult Result = Engine.initialize();

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ImportedGlobalMutabilityMismatch);
    }

    // Verifies that imported globals can re-export another imported global and resolve through the complete alias chain.
    TEST(ModuleExecutionTest, ResolvesImportedGlobalAliasChain)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
          "initializer @init\n"
          "\n"
          "declare import global constant i32 @dependency.answer from module module.m2, symbol @answer\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import module.m2\n"
          "  ret void\n"
          "}\n"
          "\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  %0 = load i32, const byte* @dependency.answer\n"
          "  ret i32 %0\n"
          "}\n";
      const std::string MiddleText =
          "inkir 1\n"
          "module module.m2\n"
          "initializer @init\n"
          "\n"
          "declare import global constant i32 @answer from module module.m3, symbol @answer\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import module.m3\n"
          "  ret void\n"
          "}\n";
      const std::string LeafText =
          "inkir 1\n"
          "module module.m3\n"
          "initializer @init\n"
          "\n"
          "@answer = global constant i32\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  store i32 42, byte* @answer\n"
          "  ret void\n"
          "}\n";
      const std::shared_ptr<ir::Module> Root = parseModule(Context.IR, RootText);
      const std::shared_ptr<ir::Module> Middle = parseModule(Context.IR, MiddleText);
      const std::shared_ptr<ir::Module> Leaf = parseModule(Context.IR, LeafText);
      ASSERT_NE(Root, nullptr);
      ASSERT_NE(Middle, nullptr);
      ASSERT_NE(Leaf, nullptr);
      TestModuleProvider Provider;
      Provider.addModule(Root);
      Provider.addModule(Middle);
      Provider.addModule(Leaf);
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));

      const ExecutionResult Result = Engine.execute("main");

      expectIntegerResult(Result, 42);
    }

    // Verifies that an ordinary function can dynamically load a previously unseen module and bind its imported function before use.
    TEST(ModuleExecutionTest, LoadsAndBindsModuleFromOrdinaryFunction)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
          "\n"
          "declare import i32 @dependency.answer() from module module.m2, symbol @answer\n"
          "\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  import module.m2\n"
          "  %0 = call i32 @dependency.answer()\n"
          "  ret i32 %0\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module module.m2\n"
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
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));

      const InitializationResult Initialized = Engine.initialize();
      EXPECT_TRUE(Initialized.succeeded());
      EXPECT_EQ(Provider.requestCount(moduleName(2)), 0U);
      const ExecutionResult First = Engine.execute("main");
      const ExecutionResult Repeated = Engine.execute("main");

      expectIntegerResult(First, 42);
      expectIntegerResult(Repeated, 42);
      EXPECT_EQ(Provider.requestCount(moduleName(2)), 1U);
    }

    // Verifies that a compilation session materializes missing modules in its shared IR context and caches compiled images across runtime requests.
    TEST(ModuleExecutionTest, CompilesMissingModuleOnDynamicImport)
    {
      core::CompilationContext Compilation;
      ExecutionContext Execution(Compilation);
      TextModuleCompiler Compiler;
      Compiler.addModule(moduleName(1),
                         "inkir 1\n"
                         "module module.m1\n"
                         "\n"
                         "declare import i32 @dependency.answer() from module module.m2, symbol @answer\n"
                         "\n"
                         "define i32 @main() {\n"
                         "entry:\n"
                         "  import module.m2\n"
                         "  %0 = call i32 @dependency.answer()\n"
                         "  ret i32 %0\n"
                         "}\n");
      Compiler.addModule(moduleName(2),
                         "inkir 1\n"
                         "module module.m2\n"
                         "\n"
                         "define i32 @answer() {\n"
                         "entry:\n"
                         "  ret i32 42\n"
                         "}\n");
      ir::CompilationSession Session(Compilation, Compiler);
      CompilingModuleProvider Provider(Session);
      ExecutionEngine Engine(Execution, Provider, moduleName(1));

      const InitializationResult Initialized = Engine.initialize();
      EXPECT_TRUE(Initialized.succeeded());
      EXPECT_EQ(Compiler.compilationCount(moduleName(1)), 1U);
      EXPECT_EQ(Compiler.compilationCount(moduleName(2)), 0U);
      const ExecutionResult First = Engine.execute("main");
      const ExecutionResult Repeated = Engine.execute("main");

      expectIntegerResult(First, 42);
      expectIntegerResult(Repeated, 42);
      EXPECT_EQ(Compiler.compilationCount(moduleName(1)), 1U);
      EXPECT_EQ(Compiler.compilationCount(moduleName(2)), 1U);
    }

    // Verifies that recursive compilation uses one session context, detects an active dependency cycle, and caches the failed graph.
    TEST(CompilationSessionTest, RejectsAndCachesRecursiveModuleCycle)
    {
      core::CompilationContext Compilation;
      TextModuleCompiler Compiler;
      Compiler.addModule(moduleName(1), "inkir 1\nmodule module.m1\n");
      Compiler.addModule(moduleName(2), "inkir 1\nmodule module.m2\n");
      Compiler.addCompilationDependency(moduleName(1), moduleName(2));
      Compiler.addCompilationDependency(moduleName(2), moduleName(1));
      ir::CompilationSession Session(Compilation, Compiler);

      const ir::ModuleCompilationResult First = Session.getOrCompileModule(moduleName(1));
      const ir::ModuleCompilationResult Repeated = Session.getOrCompileModule(moduleName(1));

      EXPECT_EQ(First.Status, ir::ModuleCompilationStatus::Failed);
      EXPECT_EQ(Repeated.Status, ir::ModuleCompilationStatus::Failed);
      EXPECT_EQ(Compiler.compilationCount(moduleName(1)), 1U);
      EXPECT_EQ(Compiler.compilationCount(moduleName(2)), 1U);
    }

    // Verifies that the host can load an arbitrary canonical module name and receives a stable runtime module handle on repetition.
    TEST(ModuleExecutionTest, LoadsModuleByRuntimeHostName)
    {
      ModuleExecutionTestContext Context;
      TestModuleProvider Provider;
      Provider.addModule(makeModuleDefinition(Context.IR, moduleName(1)));
      Provider.addModule(makeModuleDefinition(Context.IR, moduleName(2)));
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));
      ASSERT_TRUE(Engine.initialize().succeeded());

      const DynamicModuleLoadResult First = Engine.loadModule(moduleName(2));
      const DynamicModuleLoadResult Repeated = Engine.loadModule(moduleName(2));

      ASSERT_TRUE(First.succeeded());
      ASSERT_TRUE(Repeated.succeeded());
      EXPECT_EQ(First.moduleId(), Repeated.moduleId());
      EXPECT_EQ(Provider.requestCount(moduleName(2)), 1U);
    }

    // Verifies that a dependency loaded after its importer is Ready is still finalized after the importer.
    TEST(ModuleExecutionTest, FinalizesRuntimeImportedDependencyAfterImporter)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
          "finalizer @fini\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "\n"
          "define void @main() {\n"
          "entry:\n"
          "  import module.m2\n"
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
          "module module.m2\n"
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
      ASSERT_TRUE(Context.Execution.nativeSymbols().registerSymbol("record_module_event", reinterpret_cast<NativeFunctionAddress>(&recordModuleEvent)));
      TestModuleProvider Provider;
      Provider.addModule(Root);
      Provider.addModule(Dependency);
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));
      RecordedModuleEvents.clear();
      ASSERT_TRUE(Engine.execute("main").succeeded());

      const ShutdownResult Result = Engine.shutdown();

      EXPECT_TRUE(Result.succeeded());
      EXPECT_EQ(RecordedModuleEvents, (std::vector<std::int32_t>{1, 2}));
    }

    // Verifies that an engine rejects a provider image built in a different IR context than the provider declared.
    TEST(ModuleExecutionTest, RejectsProviderIrContextMismatch)
    {
      ModuleExecutionTestContext Context;
      ir::IRContext OtherIR(Context.Compilation);
      MismatchedContextModuleProvider Provider(Context.IR, makeModuleDefinition(OtherIR, moduleName(1)));
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));

      const InitializationResult Result = Engine.initialize();

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ModuleIrContextMismatch);
    }

    // Verifies that compiler diagnostics from a failed on-demand module compilation are returned without being replaced by a generic provider error.
    TEST(ModuleExecutionTest, PreservesDynamicCompilationDiagnostics)
    {
      core::CompilationContext Compilation;
      ExecutionContext Execution(Compilation);
      TextModuleCompiler Compiler;
      Compiler.addModule(moduleName(1), "not valid InkIR");
      ir::CompilationSession Session(Compilation, Compiler);
      CompilingModuleProvider Provider(Session);
      ExecutionEngine Engine(Execution, Provider, moduleName(1));

      const InitializationResult Result = Engine.initialize();

      ASSERT_FALSE(Result.succeeded());
      ASSERT_FALSE(Result.diagnostics().empty());
      EXPECT_NE(Result.diagnostics()[0].Kind, core::DiagnosticKind::ModuleProviderFailed);
      EXPECT_EQ(Compiler.compilationCount(moduleName(1)), 1U);
    }

    // Verifies that an imported function remains unbound until its ordinary-function dynamic import executes.
    TEST(ModuleExecutionTest, RejectsImportedFunctionBeforeDynamicImport)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
          "\n"
          "declare import i32 @dependency.answer() from module module.m2, symbol @answer\n"
          "\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  %0 = call i32 @dependency.answer()\n"
          "  ret i32 %0\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module module.m2\n"
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
      ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ImportedFunctionNotBound);
      EXPECT_EQ(Provider.requestCount(moduleName(2)), 0U);
    }

    // Verifies that a dependency initializer failure stays sticky in one Engine even after its missing native symbol becomes available, while a fresh Engine can initialize it.
    TEST(ModuleExecutionTest, CachesFailedDependencyInitializationPerEngine)
    {
      ModuleExecutionTestContext Context;
      const std::string RootText =
          "inkir 1\n"
          "module module.m1\n"
          "initializer @init\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import module.m2\n"
          "  ret void\n"
          "}\n";
      const std::string DependencyText =
          "inkir 1\n"
          "module module.m2\n"
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
      ExecutionEngine FailedEngine(Context.Execution, Provider, moduleName(1));

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
      ExecutionEngine FreshEngine(Context.Execution, Provider, moduleName(1));
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
          "module module.m1\n"
          "initializer @init\n"
          "finalizer @fini\n"
          "\n"
          "declare extern \"C\" void @record_module_event(i32) [sideeffect]\n"
          "\n"
          "define void @init() {\n"
          "entry:\n"
          "  import module.m2\n"
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
          "module module.m2\n"
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
        ExecutionEngine Engine(Context.Execution, Provider, moduleName(1));
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
          "module module.m1\n"
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
          "module module.m1\n"
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
          "module module.m1\n"
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
          "module module.m1\n"
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
          "module module.m1\n"
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
          "module module.m1\n"
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
          "module module.m1\n"
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
          "module module.m1\n"
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
          "module module.m1\n"
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
