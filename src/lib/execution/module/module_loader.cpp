#include "ink/execution/module/module_loader.h"

#include "ink/ir/model/module.h"

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    struct LoadFrame
    {
        const ModuleLoader *Loader;
        ModuleId Module;
        const LoadFrame *Previous;
    };

    thread_local const LoadFrame *CurrentLoadFrame = nullptr;

    struct OperationFrame
    {
        const ModuleLoader *Loader;
        ModuleId Module;
        const OperationFrame *Previous;
    };

    thread_local const OperationFrame *CurrentOperationFrame = nullptr;

    struct ProviderFrame
    {
        const ModuleLoader *Loader;
        const ProviderFrame *Previous;
    };

    thread_local const ProviderFrame *CurrentProviderFrame = nullptr;

    struct FinalizationFrame
    {
        const ModuleLoader *Loader;
        ModuleId Module;
        const FinalizationFrame *Previous;
    };

    thread_local const FinalizationFrame *CurrentFinalizationFrame = nullptr;

    bool loadChainContains(const ModuleLoader &Loader, ModuleId Module) noexcept
    {
      for (const LoadFrame *Frame = CurrentLoadFrame; Frame != nullptr; Frame = Frame->Previous)
      {
        if (Frame->Loader == &Loader && Frame->Module == Module)
        {
          return true;
        }
      }
      return false;
    }

    bool operationChainContains(const ModuleLoader &Loader) noexcept
    {
      for (const OperationFrame *Frame = CurrentOperationFrame; Frame != nullptr; Frame = Frame->Previous)
      {
        if (Frame->Loader == &Loader)
        {
          return true;
        }
      }
      return false;
    }

    ModuleId currentOperationModule(const ModuleLoader &Loader) noexcept
    {
      for (const OperationFrame *Frame = CurrentOperationFrame; Frame != nullptr; Frame = Frame->Previous)
      {
        if (Frame->Loader == &Loader)
        {
          return Frame->Module;
        }
      }
      return {};
    }

    bool providerCallbackContains(const ModuleLoader &Loader) noexcept
    {
      for (const ProviderFrame *Frame = CurrentProviderFrame; Frame != nullptr; Frame = Frame->Previous)
      {
        if (Frame->Loader == &Loader)
        {
          return true;
        }
      }
      return false;
    }

    std::size_t loadChainDepth(const ModuleLoader &Loader) noexcept
    {
      std::size_t Depth = 0;
      for (const LoadFrame *Frame = CurrentLoadFrame; Frame != nullptr; Frame = Frame->Previous)
      {
        if (Frame->Loader == &Loader)
        {
          ++Depth;
        }
      }
      return Depth;
    }

    ModuleId currentLoadingModule(const ModuleLoader &Loader) noexcept
    {
      for (const LoadFrame *Frame = CurrentLoadFrame; Frame != nullptr; Frame = Frame->Previous)
      {
        if (Frame->Loader == &Loader)
        {
          return Frame->Module;
        }
      }
      return {};
    }

    ModuleId currentFinalizingModule(const ModuleLoader &Loader) noexcept
    {
      for (const FinalizationFrame *Frame = CurrentFinalizationFrame; Frame != nullptr; Frame = Frame->Previous)
      {
        if (Frame->Loader == &Loader)
        {
          return Frame->Module;
        }
      }
      return {};
    }

    class LoadFrameScope
    {
      public:
        LoadFrameScope(const ModuleLoader &Loader, ModuleId Module) noexcept
            : Frame{&Loader, Module, CurrentLoadFrame}
        {
          CurrentLoadFrame = &Frame;
        }

        ~LoadFrameScope()
        {
          CurrentLoadFrame = Frame.Previous;
        }

        LoadFrameScope(const LoadFrameScope &) = delete;
        LoadFrameScope &operator=(const LoadFrameScope &) = delete;

      private:
        LoadFrame Frame;
    };

    class OperationFrameScope
    {
      public:
        OperationFrameScope(const ModuleLoader &Loader, ModuleId Module) noexcept
            : Frame{&Loader, Module, CurrentOperationFrame}
        {
          CurrentOperationFrame = &Frame;
        }

        ~OperationFrameScope()
        {
          CurrentOperationFrame = Frame.Previous;
        }

        OperationFrameScope(const OperationFrameScope &) = delete;
        OperationFrameScope &operator=(const OperationFrameScope &) = delete;

      private:
        OperationFrame Frame;
    };

    class ProviderFrameScope
    {
      public:
        explicit ProviderFrameScope(const ModuleLoader &Loader) noexcept
            : Frame{&Loader, CurrentProviderFrame}
        {
          CurrentProviderFrame = &Frame;
        }

        ~ProviderFrameScope()
        {
          CurrentProviderFrame = Frame.Previous;
        }

        ProviderFrameScope(const ProviderFrameScope &) = delete;
        ProviderFrameScope &operator=(const ProviderFrameScope &) = delete;

      private:
        ProviderFrame Frame;
    };

    class FinalizationFrameScope
    {
      public:
        FinalizationFrameScope(const ModuleLoader &Loader, ModuleId Module) noexcept
            : Frame{&Loader, Module, CurrentFinalizationFrame}
        {
          CurrentFinalizationFrame = &Frame;
        }

        ~FinalizationFrameScope()
        {
          CurrentFinalizationFrame = Frame.Previous;
        }

        FinalizationFrameScope(const FinalizationFrameScope &) = delete;
        FinalizationFrameScope &operator=(const FinalizationFrameScope &) = delete;

      private:
        FinalizationFrame Frame;
    };

    struct RegistryEntry
    {
        ModuleId Id;
        ir::Name Name;
        std::mutex Mutex;
        std::condition_variable Resolved;
        bool Resolving = false;
        std::thread::id ResolverThread;
        std::shared_ptr<ModuleInstance> Instance;
        ModuleLoadError Failure;
    };

    struct WaitEdge
    {
        std::uint64_t Token;
        ModuleId Importer;
        ModuleId Target;
    };
  } // namespace

  class ModuleLoader::Impl
  {
    public:
      Impl(ModuleProvider &Provider, ModuleLifecycle &Lifecycle, core::TargetContext TargetValue)
          : Provider(Provider),
            Lifecycle(Lifecycle),
            Target(TargetValue)
      {
      }

      std::shared_ptr<RegistryEntry> getOrCreateEntry(const ir::Name &Name)
      {
        const std::lock_guard<std::mutex> Lock(Mutex);
        const auto Existing = Registry.find(Name);
        if (Existing != Registry.end())
        {
          return Existing->second;
        }
        const ModuleId Id{EntriesById.size()};
        std::shared_ptr<RegistryEntry> Entry = std::make_shared<RegistryEntry>();
        Entry->Id = Id;
        Entry->Name = Name;
        Registry.emplace(Name, Entry);
        EntriesById.push_back(Entry);
        return Entry;
      }

      std::shared_ptr<RegistryEntry> findEntry(const ir::Name &Name) const
      {
        const std::lock_guard<std::mutex> Lock(Mutex);
        const auto Found = Registry.find(Name);
        return Found == Registry.end() ? nullptr : Found->second;
      }

      std::shared_ptr<RegistryEntry> findEntry(ModuleId Id) const
      {
        const std::lock_guard<std::mutex> Lock(Mutex);
        return !Id.valid() || Id.value() >= EntriesById.size() ? nullptr : EntriesById[Id.value()];
      }

      bool waitPathExists(ModuleId Start, ModuleId Destination) const
      {
        std::vector<ModuleId> Pending = {Start};
        std::unordered_set<std::size_t> Visited;
        while (!Pending.empty())
        {
          const ModuleId Current = Pending.back();
          Pending.pop_back();
          if (Current == Destination)
          {
            return true;
          }
          if (!Visited.insert(Current.value()).second)
          {
            continue;
          }
          for (const WaitEdge &Edge : WaitEdges)
          {
            if (Edge.Importer == Current)
            {
              Pending.push_back(Edge.Target);
            }
          }
        }
        return false;
      }

      std::pair<std::uint64_t, bool> addWaitEdge(ModuleId Importer, ModuleId TargetModule)
      {
        const std::lock_guard<std::mutex> Lock(Mutex);
        if (waitPathExists(TargetModule, Importer))
        {
          return {0, true};
        }
        const std::uint64_t Token = NextWaitEdgeToken++;
        WaitEdges.push_back({Token, Importer, TargetModule});
        return {Token, false};
      }

      void removeWaitEdge(std::uint64_t Token) noexcept
      {
        const std::lock_guard<std::mutex> Lock(Mutex);
        const auto Edge = std::find_if(WaitEdges.begin(), WaitEdges.end(), [Token](const WaitEdge &Value)
                                       {
                                         return Value.Token == Token;
                                       });
        if (Edge != WaitEdges.end())
        {
          WaitEdges.erase(Edge);
        }
      }

      ModuleProvider &Provider;
      ModuleLifecycle &Lifecycle;
      core::TargetContext Target;
      mutable std::mutex Mutex;
      std::condition_variable OperationsChanged;
      std::condition_variable ShutdownCompleted;
      std::unordered_map<ir::Name, std::shared_ptr<RegistryEntry>> Registry;
      std::vector<std::shared_ptr<RegistryEntry>> EntriesById;
      std::vector<std::shared_ptr<ModuleInstance>> ReadyOrder;
      std::vector<WaitEdge> WaitEdges;
      std::vector<ModuleLoadError> ShutdownErrors;
      std::uint64_t NextWaitEdgeToken = 1;
      std::size_t ActiveOperations = 0;
      bool Stopping = false;
      bool Stopped = false;
  };

  class ModuleLoader::OperationGuard
  {
    public:
      explicit OperationGuard(ModuleLoader &Loader) noexcept
          : Loader(Loader)
      {
      }

      ~OperationGuard()
      {
        Loader.endOperation();
      }

      OperationGuard(const OperationGuard &) = delete;
      OperationGuard &operator=(const OperationGuard &) = delete;

    private:
      ModuleLoader &Loader;
  };

  ModuleLoadError ModuleLifecycle::prepare(ModuleInstance &) noexcept
  {
    return {};
  }

  ModuleLoadError ModuleLifecycle::initialize(ModuleLoader &, ModuleInstance &) noexcept
  {
    return {};
  }

  ModuleLoadError ModuleLifecycle::finalize(ModuleLoader &, ModuleInstance &) noexcept
  {
    return {};
  }

  ModuleLoadResult::ModuleLoadResult(std::shared_ptr<ModuleInstance> Instance, ModuleLoadError Error) noexcept
      : Instance(std::move(Instance)),
        Error(Error)
  {
  }

  bool ModuleLoadResult::succeeded() const noexcept
  {
    return Instance != nullptr && !Error.failed();
  }

  std::shared_ptr<ModuleInstance> ModuleLoadResult::instance() const noexcept
  {
    return Instance;
  }

  const ModuleLoadError &ModuleLoadResult::error() const noexcept
  {
    return Error;
  }

  ModuleLoader::ModuleLoader(ModuleProvider &Provider, ModuleLifecycle &Lifecycle, core::TargetContext Target)
      : Implementation(std::make_unique<Impl>(Provider, Lifecycle, Target))
  {
  }

  ModuleLoader::~ModuleLoader() noexcept
  {
    shutdownImpl();
  }

  ModuleLoadResult ModuleLoader::loadModule(const ir::Name &TargetName)
  {
    const std::shared_ptr<RegistryEntry> Entry = Implementation->getOrCreateEntry(TargetName);
    const ModuleId Target = Entry->Id;
    if (providerCallbackContains(*this))
    {
      return failedResult(ModuleLoadError::failure(ModuleLoadErrorKind::ProviderFailure, Target));
    }
    if (!beginOperation())
    {
      return failedResult(ModuleLoadError::failure(ModuleLoadErrorKind::LoaderStopped, Target));
    }
    const OperationFrameScope Operation(*this, Target);
    const OperationGuard ActiveOperation(*this);
    return loadModuleImpl(Entry->Name, Target, nullptr);
  }

  ModuleLoadResult ModuleLoader::importModule(ModuleInstance &Importer, const ir::Name &TargetName)
  {
    const std::shared_ptr<RegistryEntry> Entry = Implementation->getOrCreateEntry(TargetName);
    const ModuleId Target = Entry->Id;
    if (!Importer.belongsTo(*this))
    {
      return failedResult(ModuleLoadError::failure(ModuleLoadErrorKind::InvalidImporter, Target, Importer.id()));
    }
    if (!Importer.acceptsImport())
    {
      const ModuleLoadError Error = ModuleLoadError::failure(ModuleLoadErrorKind::InvalidState, Target, Importer.id());
      Importer.recordPendingFailure(Error);
      return failedResult(Error);
    }
    if (providerCallbackContains(*this))
    {
      const ModuleLoadError Error = ModuleLoadError::failure(ModuleLoadErrorKind::ProviderFailure, Target, Importer.id());
      Importer.recordPendingFailure(Error);
      return failedResult(Error);
    }
    if (!beginOperation())
    {
      const ModuleLoadError Error = ModuleLoadError::failure(ModuleLoadErrorKind::LoaderStopped, Target, Importer.id());
      Importer.recordPendingFailure(Error);
      return failedResult(Error);
    }

    const OperationFrameScope Operation(*this, Target);
    const OperationGuard ActiveOperation(*this);
    ModuleLoadResult Result = loadModuleImpl(Entry->Name, Target, &Importer);
    if (Result.succeeded())
    {
      if (!Importer.addActiveDependency(Target))
      {
        Result = failedResult(ModuleLoadError::failure(ModuleLoadErrorKind::InitializationFailed, Importer.id(), Target), Result.instance());
        Importer.recordPendingFailure(Result.error());
      }
    }
    else
    {
      Importer.recordPendingFailure(Result.error());
    }
    return Result;
  }

  std::shared_ptr<ModuleInstance> ModuleLoader::findModule(ModuleId Target) const noexcept
  {
    const std::shared_ptr<RegistryEntry> Entry = Implementation->findEntry(Target);
    if (Entry == nullptr)
    {
      return nullptr;
    }
    const std::lock_guard<std::mutex> Lock(Entry->Mutex);
    return Entry->Instance;
  }

  std::shared_ptr<ModuleInstance> ModuleLoader::findModule(const ir::Name &Target) const
  {
    const std::shared_ptr<RegistryEntry> Entry = Implementation->findEntry(Target);
    if (Entry == nullptr)
    {
      return nullptr;
    }
    const std::lock_guard<std::mutex> Lock(Entry->Mutex);
    return Entry->Instance;
  }

  ir::Name ModuleLoader::moduleName(ModuleId Target) const
  {
    const std::shared_ptr<RegistryEntry> Entry = Implementation->findEntry(Target);
    return Entry == nullptr ? ir::Name{} : Entry->Name;
  }

  std::vector<ModuleLoadError> ModuleLoader::shutdown()
  {
    const ModuleLoadError ImmediateError = shutdownImpl();
    return ImmediateError.failed() ? std::vector<ModuleLoadError>{ImmediateError} : Implementation->ShutdownErrors;
  }

  ModuleLoadError ModuleLoader::shutdownImpl() noexcept
  {
    if (operationChainContains(*this))
    {
      return ModuleLoadError::failure(ModuleLoadErrorKind::ShutdownDuringInitialization, currentOperationModule(*this));
    }
    if (currentFinalizingModule(*this).valid())
    {
      return ModuleLoadError::failure(ModuleLoadErrorKind::ShutdownDuringFinalization, currentFinalizingModule(*this));
    }

    std::vector<std::shared_ptr<ModuleInstance>> ReadyModules;
    {
      std::unique_lock<std::mutex> Lock(Implementation->Mutex);
      if (Implementation->Stopped)
      {
        return {};
      }
      if (Implementation->Stopping)
      {
        Implementation->ShutdownCompleted.wait(Lock, [this]()
                                               {
                                                 return Implementation->Stopped;
                                               });
        return {};
      }
      Implementation->Stopping = true;
      Implementation->OperationsChanged.wait(Lock, [this]()
                                             {
                                               return Implementation->ActiveOperations == 0;
                                             });
      ReadyModules = std::move(Implementation->ReadyOrder);
    }

    std::unordered_map<std::size_t, std::size_t> ImporterCounts;
    std::unordered_set<std::size_t> ReadyModuleIds;
    for (const std::shared_ptr<ModuleInstance> &Module : ReadyModules)
    {
      ReadyModuleIds.insert(Module->id().value());
      ImporterCounts.emplace(Module->id().value(), 0);
    }
    for (const std::shared_ptr<ModuleInstance> &Module : ReadyModules)
    {
      for (const ModuleId Dependency : Module->activeDependencies())
      {
        if (ReadyModuleIds.find(Dependency.value()) != ReadyModuleIds.end())
        {
          ++ImporterCounts[Dependency.value()];
        }
      }
    }

    std::vector<std::shared_ptr<ModuleInstance>> FinalizationOrder;
    FinalizationOrder.reserve(ReadyModules.size());
    std::unordered_set<std::size_t> ScheduledModules;
    while (FinalizationOrder.size() != ReadyModules.size())
    {
      std::shared_ptr<ModuleInstance> Next;
      for (auto Candidate = ReadyModules.rbegin(); Candidate != ReadyModules.rend(); ++Candidate)
      {
        if (ScheduledModules.find((*Candidate)->id().value()) == ScheduledModules.end() && ImporterCounts[(*Candidate)->id().value()] == 0)
        {
          Next = *Candidate;
          break;
        }
      }
      if (Next == nullptr)
      {
        for (auto Candidate = ReadyModules.rbegin(); Candidate != ReadyModules.rend(); ++Candidate)
        {
          if (ScheduledModules.find((*Candidate)->id().value()) == ScheduledModules.end())
          {
            Next = *Candidate;
            break;
          }
        }
      }
      ScheduledModules.insert(Next->id().value());
      FinalizationOrder.push_back(Next);
      for (const ModuleId Dependency : Next->activeDependencies())
      {
        const auto Count = ImporterCounts.find(Dependency.value());
        if (Count != ImporterCounts.end() && Count->second != 0)
        {
          --Count->second;
        }
      }
    }

    std::vector<ModuleLoadError> Errors;
    Errors.reserve(FinalizationOrder.size());
    for (const std::shared_ptr<ModuleInstance> &Module : FinalizationOrder)
    {
      if (!Module->beginFinalization())
      {
        continue;
      }
      {
        const FinalizationFrameScope Frame(*this, Module->id());
        const ModuleLoadError Error = normalizeLifecycleError(Implementation->Lifecycle.finalize(*this, *Module), ModuleLoadErrorKind::FinalizationFailed, Module->id());
        Module->completeFinalization(Error);
        if (Error.failed())
        {
          Errors.push_back(Error);
        }
      }
    }

    {
      const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
      Implementation->ShutdownErrors = std::move(Errors);
      Implementation->Stopped = true;
      Implementation->ShutdownCompleted.notify_all();
    }
    return {};
  }

  bool ModuleLoader::beginOperation() noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    if (Implementation->Stopped || (Implementation->Stopping && !operationChainContains(*this)))
    {
      return false;
    }
    ++Implementation->ActiveOperations;
    return true;
  }

  void ModuleLoader::endOperation() noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    if (Implementation->ActiveOperations != 0)
    {
      --Implementation->ActiveOperations;
    }
    if (Implementation->ActiveOperations == 0)
    {
      Implementation->OperationsChanged.notify_all();
    }
  }

  ModuleLoadResult ModuleLoader::loadModuleImpl(const ir::Name &TargetName, ModuleId Target, ModuleInstance *Importer)
  {
    if (!Target.valid())
    {
      return failedResult(ModuleLoadError::failure(ModuleLoadErrorKind::ModuleNotFound, Target, Importer == nullptr ? ModuleId{} : Importer->id()));
    }

    ModuleLoadResult Resolution = resolveModule(TargetName, Target);
    if (!Resolution.succeeded())
    {
      return Resolution;
    }
    const std::shared_ptr<ModuleInstance> Instance = Resolution.instance();

    while (true)
    {
      switch (Instance->state())
      {
      case ModuleState::Created:
      {
        if (loadChainDepth(*this) >= MaximumModuleImportDepth)
        {
          return failedResult(ModuleLoadError::failure(ModuleLoadErrorKind::ImportDepthLimitExceeded, Target, Importer == nullptr ? ModuleId{} : Importer->id()), Instance);
        }
        if (!Instance->tryBeginPreparation())
        {
          continue;
        }
        const LoadFrameScope Frame(*this, Target);
        ModuleLoadError Error = normalizeLifecycleError(Implementation->Lifecycle.prepare(*Instance), ModuleLoadErrorKind::PreparationFailed, Target);
        if (!Error.failed())
        {
          Error = Instance->prepareGlobalStorage();
        }
        if (Error.failed())
        {
          Error = Instance->completeLoading(Error);
          return failedResult(Error, Instance);
        }
        Error = Instance->beginInitialization();
        if (Error.failed())
        {
          return failedResult(Error, Instance);
        }
        Error = normalizeLifecycleError(Implementation->Lifecycle.initialize(*this, *Instance), ModuleLoadErrorKind::InitializationFailed, Target);
        Error = Instance->completeLoading(Error);
        if (Error.failed())
        {
          return failedResult(Error, Instance);
        }
        {
          const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
          Implementation->ReadyOrder.push_back(Instance);
        }
        Instance->publishReady();
        return successfulResult(Instance);
      }
      case ModuleState::Preparing:
      case ModuleState::Initializing:
      {
        if (loadChainContains(*this, Target))
        {
          return failedResult(ModuleLoadError::failure(ModuleLoadErrorKind::CircularImport, Target, Importer == nullptr ? currentLoadingModule(*this) : Importer->id()), Instance);
        }
        std::uint64_t WaitEdgeToken = 0;
        if (Importer != nullptr)
        {
          const auto WaitEdge = Implementation->addWaitEdge(Importer->id(), Target);
          if (WaitEdge.second)
          {
            return failedResult(ModuleLoadError::failure(ModuleLoadErrorKind::CircularImport, Target, Importer->id()), Instance);
          }
          WaitEdgeToken = WaitEdge.first;
        }
        Instance->waitUntilLoaded();
        if (WaitEdgeToken != 0)
        {
          Implementation->removeWaitEdge(WaitEdgeToken);
        }
        continue;
      }
      case ModuleState::Publishing:
        Instance->waitUntilLoaded();
        continue;
      case ModuleState::Ready:
        return successfulResult(Instance);
      case ModuleState::Failed:
        return failedResult(Instance->failure(), Instance);
      case ModuleState::Finalizing:
      case ModuleState::Stopped:
        return failedResult(ModuleLoadError::failure(ModuleLoadErrorKind::LoaderStopped, Target), Instance);
      }
    }
  }

  ModuleLoadResult ModuleLoader::resolveModule(const ir::Name &TargetName, ModuleId Target)
  {
    const std::shared_ptr<RegistryEntry> Entry = Implementation->getOrCreateEntry(TargetName);

    {
      std::unique_lock<std::mutex> Lock(Entry->Mutex);
      while (Entry->Resolving)
      {
        if (Entry->ResolverThread == std::this_thread::get_id())
        {
          return failedResult(ModuleLoadError::failure(ModuleLoadErrorKind::ProviderFailure, Target));
        }
        Entry->Resolved.wait(Lock);
      }
      if (Entry->Instance != nullptr)
      {
        return successfulResult(Entry->Instance);
      }
      if (Entry->Failure.failed())
      {
        return failedResult(Entry->Failure);
      }
      Entry->Resolving = true;
      Entry->ResolverThread = std::this_thread::get_id();
    }

    ModuleProvisionResult Provision;
    {
      const ProviderFrameScope ProviderFrame(*this);
      Provision = Implementation->Provider.provideModule(TargetName);
    }

    std::shared_ptr<const ir::Module> Definition = std::move(Provision.Module);
    ModuleLoadError Error;
    switch (Provision.Status)
    {
    case ModuleProvisionStatus::Found:
      if (Definition == nullptr)
      {
        Error = ModuleLoadError::failure(ModuleLoadErrorKind::ProviderFailure, Target);
      }
      break;
    case ModuleProvisionStatus::NotFound:
      Error = ModuleLoadError::failure(ModuleLoadErrorKind::ModuleNotFound, Target);
      break;
    case ModuleProvisionStatus::Failed:
      Error = ModuleLoadError::failure(ModuleLoadErrorKind::ProviderFailure, Target, {}, Provision.DiagnosticReported);
      break;
    }
    if (!Error.failed() && Definition->Name.has_value() && *Definition->Name != TargetName)
    {
      Error = ModuleLoadError::failure(ModuleLoadErrorKind::ModuleIdentityMismatch, Target, Implementation->getOrCreateEntry(*Definition->Name)->Id);
    }
    std::shared_ptr<ModuleInstance> Instance;
    if (!Error.failed())
    {
      Instance = std::shared_ptr<ModuleInstance>(new ModuleInstance(*this, Target, TargetName, std::move(Definition), Implementation->Target));
    }

    {
      const std::lock_guard<std::mutex> Lock(Entry->Mutex);
      Entry->Instance = Instance;
      Entry->Failure = Error;
      Entry->Resolving = false;
      Entry->ResolverThread = {};
      Entry->Resolved.notify_all();
    }
    return Error.failed() ? failedResult(Error) : successfulResult(std::move(Instance));
  }

  ModuleLoadResult ModuleLoader::failedResult(ModuleLoadError Error, std::shared_ptr<ModuleInstance> Instance) const noexcept
  {
    return ModuleLoadResult(std::move(Instance), Error);
  }

  ModuleLoadResult ModuleLoader::successfulResult(std::shared_ptr<ModuleInstance> Instance) const noexcept
  {
    return ModuleLoadResult(std::move(Instance), {});
  }

  ModuleLoadError ModuleLoader::normalizeLifecycleError(ModuleLoadError Error, ModuleLoadErrorKind, ModuleId Module) const noexcept
  {
    if (Error.failed() && !Error.Module.valid())
    {
      Error.Module = Module;
    }
    return Error;
  }
} // namespace ink::execution
