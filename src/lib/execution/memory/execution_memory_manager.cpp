#include "ink/execution/memory/execution_memory_manager.h"

#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    bool arrayMetadataBytes(const TypeDesc &Layout, std::size_t &Bytes)
    {
      Bytes = 0;
      if (Layout.Kind == RuntimeKind::Class)
      {
        for (const auto &Field : Layout.classDesc().Fields)
        {
          std::size_t Child = 0;
          if (!Field.Layout || !arrayMetadataBytes(*Field.Layout, Child) || Child > std::numeric_limits<std::size_t>::max() - sizeof(RuntimeValue) || Child + sizeof(RuntimeValue) > std::numeric_limits<std::size_t>::max() - Bytes)
          {
            return false;
          }
          Bytes += Child + sizeof(RuntimeValue);
        }
        return true;
      }
      if (Layout.Kind != RuntimeKind::Array)
      {
        return true;
      }
      std::size_t ChildBytes = 0;
      if (!Layout.arrayDesc().ElementLayout || !arrayMetadataBytes(*Layout.arrayDesc().ElementLayout, ChildBytes) || ChildBytes > std::numeric_limits<std::size_t>::max() - sizeof(RuntimeValue))
      {
        return false;
      }
      ChildBytes += sizeof(RuntimeValue);
      if (Layout.arrayDesc().ElementCount > std::numeric_limits<std::size_t>::max() / ChildBytes)
      {
        return false;
      }
      Bytes = static_cast<std::size_t>(Layout.arrayDesc().ElementCount) * ChildBytes;
      return true;
    }
  } // namespace

  struct ExecutionMemoryState
  {
      struct Slot
      {
          std::unique_ptr<ExecutionStorage> Storage;
          std::uint64_t Generation = 1;
          std::size_t Bytes = 0;
      };

      std::vector<Slot> Slots;
      std::vector<std::size_t> FreeSlots;
      std::map<std::uintptr_t, std::size_t> Addresses;
      std::set<std::string, std::less<>> Strings;
      std::size_t LiveStorage = 0;
      std::size_t AllocatedStorage = 0;
      std::size_t LiveBytes = 0;
      std::size_t AllocatedBytes = 0;
      std::size_t ReservedBytes = 0;
      std::size_t MaxStorage;
      std::size_t MaxBytes;

      ExecutionMemoryState(std::size_t MaxStorage, std::size_t MaxBytes)
          : MaxStorage(MaxStorage),
            MaxBytes(MaxBytes)
      {
      }
  };

  bool ExecutionStorageRef::valid() const noexcept
  {
    return status() == ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionStorageRef::status() const noexcept
  {
    if (Generation == 0)
    {
      return ExecutionStatus::InvalidPlace;
    }
    const auto Owner = State.lock();
    if (!Owner || Slot >= Owner->Slots.size())
    {
      return ExecutionStatus::ExpiredPlace;
    }
    const auto &Entry = Owner->Slots[Slot];
    return Entry.Generation == Generation && Entry.Storage ? ExecutionStatus::Success : ExecutionStatus::ExpiredPlace;
  }

  ExecutionStorage *ExecutionStorageRef::get() const noexcept
  {
    const auto Owner = State.lock();
    if (!Owner || Generation == 0 || Slot >= Owner->Slots.size())
    {
      return nullptr;
    }
    const auto &Entry = Owner->Slots[Slot];
    return Entry.Generation == Generation ? Entry.Storage.get() : nullptr;
  }

  ExecutionCell *ExecutionStorageRef::cell() const noexcept
  {
    ExecutionStorage *Storage = get();
    return Storage && Storage->objectKind() == ExecutionObjectKind::Cell ? static_cast<ExecutionCell *>(Storage) : nullptr;
  }

  ExecutionBuffer *ExecutionStorageRef::buffer() const noexcept
  {
    ExecutionStorage *Storage = get();
    return Storage && Storage->objectKind() == ExecutionObjectKind::Buffer ? static_cast<ExecutionBuffer *>(Storage) : nullptr;
  }

  bool ExecutionStorageRef::operator==(const ExecutionStorageRef &Other) const noexcept
  {
    return Slot == Other.Slot && Generation == Other.Generation && !State.owner_before(Other.State) && !Other.State.owner_before(State);
  }

  ExecutionMemoryManager::ExecutionMemoryManager(std::size_t MaxStorage, std::size_t MaxBytes)
      : State(std::make_shared<ExecutionMemoryState>(MaxStorage, MaxBytes))
  {
  }

  ExecutionMemoryManager::~ExecutionMemoryManager() = default;

  ExecutionStatus ExecutionMemoryManager::lastStatus() const noexcept
  {
    return LastStatus;
  }

  std::size_t ExecutionMemoryManager::liveStorageCount() const noexcept
  {
    return State->LiveStorage;
  }

  std::size_t ExecutionMemoryManager::allocatedStorageCount() const noexcept
  {
    return State->AllocatedStorage;
  }

  std::size_t ExecutionMemoryManager::liveStorageBytes() const noexcept
  {
    return State->LiveBytes;
  }

  std::size_t ExecutionMemoryManager::allocatedStorageBytes() const noexcept
  {
    return State->AllocatedBytes;
  }

  bool ExecutionMemoryManager::checkBudget(std::size_t Bytes) noexcept
  {
    if (State->AllocatedStorage >= State->MaxStorage || Bytes > State->MaxBytes - State->AllocatedBytes)
    {
      LastStatus = ExecutionStatus::BudgetExceeded;
      return false;
    }
    return true;
  }

  ExecutionStorageRef ExecutionMemoryManager::ownStorage(std::unique_ptr<ExecutionStorage> Storage, std::size_t Bytes)
  {
    std::size_t Index;
    if (State->FreeSlots.empty())
    {
      Index = State->Slots.size();
      State->Slots.push_back({});
    }
    else
    {
      Index = State->FreeSlots.back();
      State->FreeSlots.pop_back();
    }
    auto &Slot = State->Slots[Index];
    Slot.Storage = std::move(Storage);
    Slot.Bytes = Bytes;
    const void *Address = Slot.Storage->objectKind() == ExecutionObjectKind::Cell ? static_cast<ExecutionCell *>(Slot.Storage.get())->data() : static_cast<ExecutionBuffer *>(Slot.Storage.get())->data();
    if (Address)
    {
      State->Addresses.emplace(reinterpret_cast<std::uintptr_t>(Address), Index);
    }
    ++State->AllocatedStorage;
    ++State->LiveStorage;
    State->AllocatedBytes += Bytes;
    State->LiveBytes += Bytes;
    LastStatus = ExecutionStatus::Success;
    return ExecutionStorageRef(State, Index, Slot.Generation);
  }

  ExecutionPlaceResult ExecutionMemoryManager::allocateCell(const TypeDesc &Layout, bool Writable, const RuntimeValue &Initial, bool Runtime)
  {
    if (Layout.Type == InvalidRuntimeType || (Initial.Initialized && Initial.Type != Layout.Type))
    {
      return {LastStatus = ExecutionStatus::TypeMismatch};
    }
    std::size_t Bytes = sizeof(ExecutionCell);
    if (Layout.Kind == RuntimeKind::Array || Layout.Kind == RuntimeKind::Class)
    {
      if (Layout.Kind == RuntimeKind::Array && (!Layout.arrayDesc().ElementLayout || Layout.arrayDesc().ElementLayout->Type != Layout.arrayDesc().ElementType))
      {
        return {LastStatus = ExecutionStatus::TypeMismatch};
      }
      std::size_t MetadataBytes = 0;
      if (!arrayMetadataBytes(Layout, MetadataBytes) || MetadataBytes > std::numeric_limits<std::size_t>::max() - Bytes)
      {
        return {LastStatus = ExecutionStatus::BudgetExceeded};
      }
      Bytes += MetadataBytes;
    }
    if (Layout.Size != 0)
    {
      const std::size_t BackingBytes = std::max<std::size_t>(Layout.Size, 1);
      if (BackingBytes > std::numeric_limits<std::size_t>::max() - Bytes)
      {
        return {LastStatus = ExecutionStatus::BudgetExceeded};
      }
      Bytes += BackingBytes;
    }
    if (!checkBudget(Bytes))
    {
      return {LastStatus};
    }
    std::unique_ptr<ExecutionCell> Cell(new ExecutionCell(*this, Layout, Writable, Runtime));
    if (Initial.Initialized)
    {
      if (Runtime)
      {
        Cell->Initialized = true;
      }
      else
      {
        State->ReservedBytes += Bytes;
        const ExecutionStatus Status = Cell->storeRuntime(Initial);
        State->ReservedBytes -= Bytes;
        if (Status != ExecutionStatus::Success)
        {
          return {LastStatus = Status};
        }
      }
    }
    const ExecutionStorageRef Storage = ownStorage(std::move(Cell), Bytes);
    return {ExecutionStatus::Success, ExecutionPlace(Storage)};
  }

  ExecutionStatus ExecutionMemoryManager::chargeLocalAllocation() noexcept
  {
    if (!checkBudget(sizeof(ExecutionCell)))
    {
      return LastStatus;
    }
    ++State->AllocatedStorage;
    State->AllocatedBytes += sizeof(ExecutionCell);
    return LastStatus = ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionMemoryManager::validatePointer(const ExecutionPointer &Pointer) const noexcept
  {
    return Pointer.status();
  }

  ExecutionStorageRef ExecutionMemoryManager::allocateBuffer(std::string_view Bytes, bool Terminate)
  {
    const std::size_t Overhead = sizeof(ExecutionBuffer) + static_cast<std::size_t>(Terminate);
    if (Bytes.size() > std::numeric_limits<std::size_t>::max() - Overhead)
    {
      LastStatus = ExecutionStatus::BudgetExceeded;
      return {};
    }
    const std::size_t Size = Bytes.size() + Overhead;
    if (!checkBudget(Size))
    {
      return {};
    }
    return ownStorage(std::unique_ptr<ExecutionStorage>(new ExecutionBuffer(Bytes, Terminate)), Size);
  }

  ExecutionStorageRef ExecutionMemoryManager::allocateBuffer(std::size_t Size)
  {
    if (Size > std::numeric_limits<std::size_t>::max() - sizeof(ExecutionBuffer))
    {
      LastStatus = ExecutionStatus::BudgetExceeded;
      return {};
    }
    const std::size_t Bytes = sizeof(ExecutionBuffer) + Size;
    if (!checkBudget(Bytes))
    {
      return {};
    }
    return ownStorage(std::unique_ptr<ExecutionStorage>(new ExecutionBuffer(Size)), Bytes);
  }

  bool ExecutionMemoryManager::owns(const ExecutionStorageRef &Storage) const noexcept
  {
    return Storage.Generation != 0 && !Storage.State.owner_before(State) && !State.owner_before(Storage.State);
  }

  ExecutionStatus ExecutionMemoryManager::release(const ExecutionStorageRef &Storage) noexcept
  {
    if (!owns(Storage))
    {
      return LastStatus = ExecutionStatus::InvalidPlace;
    }
    const ExecutionStatus Status = Storage.status();
    if (Status != ExecutionStatus::Success)
    {
      return LastStatus = Status;
    }
    auto &Slot = State->Slots[Storage.Slot];
    const void *Address = Slot.Storage->objectKind() == ExecutionObjectKind::Cell ? static_cast<ExecutionCell *>(Slot.Storage.get())->data() : static_cast<ExecutionBuffer *>(Slot.Storage.get())->data();
    State->Addresses.erase(reinterpret_cast<std::uintptr_t>(Address));
    Slot.Storage.reset();
    --State->LiveStorage;
    State->LiveBytes -= Slot.Bytes;
    Slot.Bytes = 0;
    // A generation never wraps; a saturated slot is permanently retired.
    if (Slot.Generation != std::numeric_limits<std::uint64_t>::max())
    {
      ++Slot.Generation;
      State->FreeSlots.push_back(Storage.Slot);
    }
    return LastStatus = ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionMemoryManager::release(ExecutionPlace Place) noexcept
  {
    if (!owns(Place.storage()))
    {
      return LastStatus = ExecutionStatus::InvalidPlace;
    }
    const ExecutionStatus Status = Place.status();
    return Status == ExecutionStatus::Success ? release(Place.storage()) : (LastStatus = Status);
  }

  ExecutionPointer ExecutionMemoryManager::pointerFromAddress(void *Address) const noexcept
  {
    return ExecutionPointer::fromNative(Address);
  }

  ExecutionStorageRef ExecutionMemoryManager::storageFromAddress(const void *Address) const noexcept
  {
    if (!Address)
    {
      return {};
    }
    const std::uintptr_t Requested = reinterpret_cast<std::uintptr_t>(Address);
    auto Found = State->Addresses.upper_bound(Requested);
    if (Found == State->Addresses.begin())
    {
      return {};
    }
    --Found;
    const auto &Slot = State->Slots[Found->second];
    const std::size_t Size = Slot.Storage->objectKind() == ExecutionObjectKind::Cell ? static_cast<const ExecutionCell *>(Slot.Storage.get())->size() : static_cast<const ExecutionBuffer *>(Slot.Storage.get())->size();
    return Requested - Found->first <= Size ? ExecutionStorageRef(State, Found->second, Slot.Generation) : ExecutionStorageRef{};
  }

  ExecutionStatus ExecutionMemoryManager::writeValueBytes(const TypeDesc &Layout, void *Destination, const RuntimeValue &Value)
  {
    if (Layout.Kind == RuntimeKind::Void)
    {
      return ExecutionStatus::Success;
    }
    if (Layout.Kind == RuntimeKind::String)
    {
      const std::string_view String = Value.string();
      const char *Data = retainString(String);
      if (!Data)
      {
        return lastStatus();
      }
      const std::size_t Length = String.size();
      std::memcpy(Destination, &Data, sizeof(Data));
      std::memcpy(static_cast<std::byte *>(Destination) + sizeof(Data), &Length, sizeof(Length));
      return ExecutionStatus::Success;
    }
    if (Layout.Kind == RuntimeKind::Array || Layout.Kind == RuntimeKind::Class)
    {
      const auto Elements = Value.aggregate();
      for (std::size_t Index = 0; Index < Elements.size(); ++Index)
      {
        const TypeDesc &Element = Layout.Kind == RuntimeKind::Array ? *Layout.arrayDesc().ElementLayout : *Layout.classDesc().Fields[Index].Layout;
        const std::size_t Offset = Layout.Kind == RuntimeKind::Array ? Index * Element.Size : Layout.classDesc().Fields[Index].Offset;
        const ExecutionStatus Status = writeValueBytes(Element, static_cast<std::byte *>(Destination) + Offset, Elements[Index]);
        if (Status != ExecutionStatus::Success)
        {
          return Status;
        }
      }
      return ExecutionStatus::Success;
    }
    return writeStorage(Layout, Destination, Value);
  }

  const char *ExecutionMemoryManager::retainString(std::string_view String)
  {
    if (String.empty())
    {
      LastStatus = ExecutionStatus::Success;
      return "";
    }
    if (const auto Found = State->Strings.find(String); Found != State->Strings.end())
    {
      LastStatus = ExecutionStatus::Success;
      return Found->data();
    }
    if (String.size() == std::numeric_limits<std::size_t>::max() || String.size() + 1 > State->MaxBytes - State->AllocatedBytes - State->ReservedBytes)
    {
      LastStatus = ExecutionStatus::BudgetExceeded;
      return nullptr;
    }
    const auto Stored = State->Strings.emplace(String).first;
    State->AllocatedBytes += String.size() + 1;
    State->LiveBytes += String.size() + 1;
    LastStatus = ExecutionStatus::Success;
    return Stored->data();
  }

  ExecutionStatus ExecutionMemoryManager::loadBytes(const void *Address, const TypeDesc &Layout, void *Destination) const
  {
    if (!Address)
    {
      return ExecutionStatus::InvalidPlace;
    }
    if (const ExecutionCell *Cell = storageFromAddress(Address).cell())
    {
      const std::size_t Offset = reinterpret_cast<std::uintptr_t>(Address) - reinterpret_cast<std::uintptr_t>(Cell->data());
      return Cell->loadBytes(Offset, Layout, Destination);
    }
    std::memmove(Destination, Address, Layout.Size);
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionMemoryManager::storeBytes(void *Address, const TypeDesc &Layout, const void *Source)
  {
    if (!Address)
    {
      return ExecutionStatus::InvalidPlace;
    }
    if (ExecutionCell *Cell = storageFromAddress(Address).cell())
    {
      const std::size_t Offset = reinterpret_cast<std::uintptr_t>(Address) - reinterpret_cast<std::uintptr_t>(Cell->data());
      return Cell->storeBytes(Offset, Layout, Source);
    }
    std::memmove(Address, Source, Layout.Size);
    return ExecutionStatus::Success;
  }

  RuntimeValueResult ExecutionMemoryManager::loadPointer(const ExecutionPointer &Pointer, const TypeDesc &Layout) const
  {
    if (!Pointer.address())
    {
      return {ExecutionStatus::InvalidPlace};
    }
    const ExecutionStorageRef Storage = storageFromAddress(Pointer.address());
    if (const ExecutionCell *Cell = Storage.cell())
    {
      if (Cell->runtime())
      {
        return {ExecutionStatus::RuntimeValue};
      }
      const std::size_t Offset = reinterpret_cast<std::uintptr_t>(Pointer.address()) - reinterpret_cast<std::uintptr_t>(Cell->data());
      if (Cell->layout().Domain == Layout.Domain && Cell->elementLayout(Offset, Layout.Type))
      {
        return Cell->loadElement(Offset, Layout.Type);
      }
      if (Layout.Kind != RuntimeKind::Integer || Layout.bitWidth() != 8)
      {
        return {ExecutionStatus::TypeMismatch};
      }
      if (Offset >= Cell->size())
      {
        return {ExecutionStatus::InvalidPlace};
      }
      if (!Cell->initialized())
      {
        return {ExecutionStatus::Uninitialized};
      }
    }
    return readStorage(Layout, Pointer.address());
  }

  ExecutionStatus ExecutionMemoryManager::storePointer(const ExecutionPointer &Pointer, const TypeDesc &Layout, const RuntimeValue &Value)
  {
    if (!Pointer.address())
    {
      return ExecutionStatus::InvalidPlace;
    }
    const ExecutionStorageRef Storage = storageFromAddress(Pointer.address());
    if (ExecutionCell *Cell = Storage.cell())
    {
      if (Cell->runtime())
      {
        return ExecutionStatus::RuntimeValue;
      }
      if (!Cell->writable() && Cell->initialized())
      {
        return ExecutionStatus::ReadOnly;
      }
      const std::size_t Offset = reinterpret_cast<std::uintptr_t>(Pointer.address()) - reinterpret_cast<std::uintptr_t>(Cell->data());
      if (Cell->layout().Domain == Layout.Domain && Cell->elementLayout(Offset, Layout.Type))
      {
        return Cell->storeElement(Offset, Value);
      }
      if (Layout.Kind != RuntimeKind::Integer || Layout.bitWidth() != 8)
      {
        return ExecutionStatus::TypeMismatch;
      }
      if (Offset >= Cell->size())
      {
        return ExecutionStatus::InvalidPlace;
      }
    }
    return writeStorage(Layout, Pointer.address(), Value);
  }
} // namespace ink::execution
