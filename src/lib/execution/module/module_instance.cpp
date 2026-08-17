#include "ink/execution/module/module_instance.h"

#include "ink/execution/module/module_loader.h"
#include "ink/ir/analysis/type_layout.h"
#include "ink/ir/model/constant.h"
#include "ink/ir/model/context.h"
#include "ink/ir/model/module.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    struct GlobalStorageSlot
    {
        RuntimeValueRef Storage = nullptr;
        RuntimeValueRef Address = nullptr;
        RuntimeValueRef MutableAddress = nullptr;
    };

    struct ByteConstantStorageSlot
    {
        RuntimeValueRef Storage = nullptr;
        RuntimeValueRef Address = nullptr;
    };
  } // namespace

  class ModuleInstance::Impl
  {
    public:
      Impl(const ModuleLoader &Owner, ModuleId Id, ir::Name Name, std::shared_ptr<const ir::Module> Definition, core::TargetContext Target)
          : Owner(&Owner),
            Id(Id),
            Name(std::move(Name)),
            Definition(std::move(Definition)),
            Target(Target),
            GlobalValues(Target)
      {
      }

      const ModuleLoader *Owner;
      ModuleId Id;
      ir::Name Name;
      std::shared_ptr<const ir::Module> Definition;
      core::TargetContext Target;
      mutable std::mutex Mutex;
      mutable std::condition_variable StateChanged;
      ModuleState State = ModuleState::Created;
      ModuleLoadError Failure;
      ModuleLoadError PendingFailure;
      std::vector<ModuleId> ActiveDependencies;
      RuntimeValueArena GlobalValues;
      std::vector<ByteConstantStorageSlot> ByteConstantStorage;
      std::unordered_map<const ir::StringConstant *, RuntimeValueRef> StringConstantStorage;
      std::vector<GlobalStorageSlot> GlobalStorage;
  };

  ModuleInstance::ModuleInstance(const ModuleLoader &Owner, ModuleId Id, ir::Name Name, std::shared_ptr<const ir::Module> Definition, core::TargetContext Target)
      : Implementation(std::make_unique<Impl>(Owner, Id, std::move(Name), std::move(Definition), Target))
  {
  }

  ModuleInstance::~ModuleInstance() = default;

  ModuleId ModuleInstance::id() const noexcept
  {
    return Implementation->Id;
  }

  const ir::Name &ModuleInstance::name() const noexcept
  {
    return Implementation->Name;
  }

  const ir::Module &ModuleInstance::definition() const noexcept
  {
    return *Implementation->Definition;
  }

  const core::TargetContext &ModuleInstance::targetContext() const noexcept
  {
    return Implementation->Target;
  }

  ModuleState ModuleInstance::state() const noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    return Implementation->State;
  }

  ModuleLoadError ModuleInstance::failure() const noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    return Implementation->Failure;
  }

  std::vector<ModuleId> ModuleInstance::activeDependencies() const
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    return Implementation->ActiveDependencies;
  }

  RuntimeValueRef ModuleInstance::byteConstantAddress(ir::ByteConstantId Constant) const noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    if (!Constant.valid() || Constant.value() >= Implementation->ByteConstantStorage.size())
    {
      return nullptr;
    }
    return Implementation->ByteConstantStorage[Constant.value()].Address;
  }

  RuntimeValueRef ModuleInstance::stringConstantValue(const ir::StringConstant &Constant) noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    const auto Existing = Implementation->StringConstantStorage.find(&Constant);
    if (Existing != Implementation->StringConstantStorage.end())
    {
      return Existing->second;
    }
    RuntimeMemoryStatus Status = RuntimeMemoryStatus::Ok;
    RuntimeValueRef Result = Implementation->GlobalValues.copyPersistentByteSlice(Constant.type(), Constant.data().data(), Constant.data().size(), Status);
    if (Result == nullptr || Status != RuntimeMemoryStatus::Ok)
    {
      return nullptr;
    }
    Implementation->StringConstantStorage.emplace(&Constant, Result);
    return Result;
  }

  RuntimeValueRef ModuleInstance::globalAddress(ir::GlobalId Global) const noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    if (!Global.valid() || Global.value() >= Implementation->GlobalStorage.size())
    {
      return nullptr;
    }
    return Implementation->GlobalStorage[Global.value()].Address;
  }

  RuntimeValueRef ModuleInstance::mutableGlobalAddress(ir::GlobalId Global) const noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    if (!Global.valid() || Global.value() >= Implementation->GlobalStorage.size())
    {
      return nullptr;
    }
    return Implementation->GlobalStorage[Global.value()].MutableAddress;
  }

  bool ModuleInstance::belongsTo(const ModuleLoader &Loader) const noexcept
  {
    return Implementation->Owner == &Loader;
  }

  bool ModuleInstance::acceptsImport() const noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    return Implementation->State == ModuleState::Initializing || Implementation->State == ModuleState::Ready;
  }

  bool ModuleInstance::tryBeginPreparation() noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    if (Implementation->State != ModuleState::Created)
    {
      return false;
    }
    Implementation->State = ModuleState::Preparing;
    return true;
  }

  ModuleLoadError ModuleInstance::prepareGlobalStorage() noexcept
  {
    std::vector<ByteConstantStorageSlot> ConstantStorage;
    std::vector<GlobalStorageSlot> Storage;
    ConstantStorage.reserve(Implementation->Definition->ByteConstants.size());
    Storage.reserve(Implementation->Definition->Globals.size());
    const ir::Type &ByteSliceType = Implementation->Definition->context().getType(ir::TypeKind::ByteSlice);
    const ir::Type &BytePointerType = Implementation->Definition->context().getType(ir::TypeKind::BytePointer);
    const ir::Type &ConstBytePointerType = Implementation->Definition->context().getType(ir::TypeKind::ConstBytePointer);
    const ir::Type &ConstByteSliceType = Implementation->Definition->context().getType(ir::TypeKind::ConstByteSlice);
    for (const ir::ByteConstant &Constant : Implementation->Definition->ByteConstants)
    {
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::Ok;
      RuntimeValueRef ConstantStorageValue = Implementation->GlobalValues.copyPersistentByteSlice(ConstByteSliceType, Constant.Data.data(), Constant.Data.size(), Status);
      if (Status != RuntimeMemoryStatus::Ok || ConstantStorageValue == nullptr)
      {
        return ModuleLoadError::failure(ModuleLoadErrorKind::InvalidGlobalStorage, Implementation->Id);
      }
      RuntimeValueRef Address = Implementation->GlobalValues.pointerFromByteSlice(ConstBytePointerType, *ConstantStorageValue);
      if (Address == nullptr)
      {
        return ModuleLoadError::failure(ModuleLoadErrorKind::InvalidGlobalStorage, Implementation->Id);
      }
      ConstantStorage.push_back({ConstantStorageValue, Address});
    }
    for (const ir::GlobalVariable &Global : Implementation->Definition->Globals)
    {
      if (Global.Kind == ir::GlobalVariableKind::Imported)
      {
        Storage.push_back({});
        continue;
      }
      if (Global.ValueType == nullptr)
      {
        return ModuleLoadError::failure(ModuleLoadErrorKind::InvalidGlobalStorage, Implementation->Id);
      }
      const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(*Global.ValueType, Implementation->Target);
      if (!Layout.has_value())
      {
        return ModuleLoadError::failure(ModuleLoadErrorKind::InvalidGlobalStorage, Implementation->Id);
      }
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::Ok;
      RuntimeValueRef GlobalStorage = Implementation->GlobalValues.allocatePersistentByteSlice(ByteSliceType, Layout->StrideSize, Status);
      if (Status != RuntimeMemoryStatus::Ok || GlobalStorage == nullptr)
      {
        return ModuleLoadError::failure(ModuleLoadErrorKind::InvalidGlobalStorage, Implementation->Id);
      }
      RuntimeValueRef MutableAddress = Implementation->GlobalValues.pointerFromByteSlice(BytePointerType, *GlobalStorage);
      RuntimeValueRef Address = Implementation->GlobalValues.pointerFromByteSlice(ConstBytePointerType, *GlobalStorage);
      if (MutableAddress == nullptr || Address == nullptr)
      {
        return ModuleLoadError::failure(ModuleLoadErrorKind::InvalidGlobalStorage, Implementation->Id);
      }
      Storage.push_back({GlobalStorage, Address, MutableAddress});
    }
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    Implementation->ByteConstantStorage = std::move(ConstantStorage);
    Implementation->GlobalStorage = std::move(Storage);
    return {};
  }

  ModuleLoadError ModuleInstance::beginInitialization() noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    if (Implementation->State != ModuleState::Preparing)
    {
      return ModuleLoadError::failure(ModuleLoadErrorKind::InvalidState, Implementation->Id);
    }
    if (Implementation->PendingFailure.failed())
    {
      Implementation->GlobalValues.endPersistentLifetimes();
      Implementation->Failure = Implementation->PendingFailure;
      Implementation->State = ModuleState::Failed;
      Implementation->StateChanged.notify_all();
      return Implementation->Failure;
    }
    Implementation->State = ModuleState::Initializing;
    return {};
  }

  ModuleLoadError ModuleInstance::completeLoading(ModuleLoadError Error) noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    if (Implementation->PendingFailure.failed())
    {
      Error = Implementation->PendingFailure;
    }
    if (Error.failed())
    {
      Implementation->GlobalValues.endPersistentLifetimes();
      Implementation->Failure = Error;
      Implementation->State = ModuleState::Failed;
      Implementation->StateChanged.notify_all();
      return Error;
    }
    for (std::size_t GlobalIndex = 0; GlobalIndex < Implementation->Definition->Globals.size(); ++GlobalIndex)
    {
      if (Implementation->Definition->Globals[GlobalIndex].Kind == ir::GlobalVariableKind::Definition && !Implementation->Definition->Globals[GlobalIndex].Mutable)
      {
        const RuntimeValueRef Storage = Implementation->GlobalStorage[GlobalIndex].Storage;
        if (Storage == nullptr || Implementation->GlobalValues.makeByteSliceReadOnly(*Storage) != RuntimeMemoryStatus::Ok)
        {
          Error = ModuleLoadError::failure(ModuleLoadErrorKind::InvalidGlobalStorage, Implementation->Id);
          Implementation->GlobalValues.endPersistentLifetimes();
          Implementation->Failure = Error;
          Implementation->State = ModuleState::Failed;
          Implementation->StateChanged.notify_all();
          return Error;
        }
      }
    }
    Implementation->State = ModuleState::Publishing;
    return {};
  }

  void ModuleInstance::publishReady() noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    if (Implementation->State == ModuleState::Publishing)
    {
      Implementation->State = ModuleState::Ready;
      Implementation->StateChanged.notify_all();
    }
  }

  void ModuleInstance::waitUntilLoaded() const noexcept
  {
    std::unique_lock<std::mutex> Lock(Implementation->Mutex);
    Implementation->StateChanged.wait(Lock, [this]()
                                      {
                                        return Implementation->State != ModuleState::Preparing && Implementation->State != ModuleState::Initializing && Implementation->State != ModuleState::Publishing;
                                      });
  }

  void ModuleInstance::recordPendingFailure(ModuleLoadError Error) noexcept
  {
    if (!Error.failed())
    {
      return;
    }
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    if ((Implementation->State == ModuleState::Preparing || Implementation->State == ModuleState::Initializing) && !Implementation->PendingFailure.failed())
    {
      Implementation->PendingFailure = Error;
    }
  }

  bool ModuleInstance::addActiveDependency(ModuleId Dependency) noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    if (Implementation->State != ModuleState::Initializing && Implementation->State != ModuleState::Ready)
    {
      return false;
    }
    const auto Existing = std::find(Implementation->ActiveDependencies.begin(), Implementation->ActiveDependencies.end(), Dependency);
    if (Existing == Implementation->ActiveDependencies.end())
    {
      Implementation->ActiveDependencies.push_back(Dependency);
    }
    return true;
  }

  bool ModuleInstance::beginFinalization() noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    if (Implementation->State != ModuleState::Ready)
    {
      return false;
    }
    Implementation->State = ModuleState::Finalizing;
    return true;
  }

  void ModuleInstance::completeFinalization(ModuleLoadError Error) noexcept
  {
    const std::lock_guard<std::mutex> Lock(Implementation->Mutex);
    Implementation->GlobalValues.endPersistentLifetimes();
    if (Error.failed())
    {
      Implementation->Failure = Error;
      Implementation->State = ModuleState::Failed;
    }
    else
    {
      Implementation->State = ModuleState::Stopped;
    }
    Implementation->StateChanged.notify_all();
  }
} // namespace ink::execution
