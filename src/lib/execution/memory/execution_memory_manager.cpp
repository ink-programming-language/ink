#include "ink/execution/memory/execution_memory_manager.h"

#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    bool arrayMetadataBytes(const StorageLayout &Layout, std::size_t &Bytes)
    {
      Bytes = 0;
      if (Layout.Kind != RuntimeKind::Array)
      {
        return true;
      }
      std::size_t ChildBytes = 0;
      if (!Layout.ElementLayout || !arrayMetadataBytes(*Layout.ElementLayout, ChildBytes) || ChildBytes > std::numeric_limits<std::size_t>::max() - sizeof(RuntimeValue))
      {
        return false;
      }
      ChildBytes += sizeof(RuntimeValue);
      if (Layout.ElementCount > std::numeric_limits<std::size_t>::max() / ChildBytes)
      {
        return false;
      }
      Bytes = static_cast<std::size_t>(Layout.ElementCount) * ChildBytes;
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
      std::size_t LiveStorage = 0;
      std::size_t AllocatedStorage = 0;
      std::size_t LiveBytes = 0;
      std::size_t AllocatedBytes = 0;
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
    ++State->AllocatedStorage;
    ++State->LiveStorage;
    State->AllocatedBytes += Bytes;
    State->LiveBytes += Bytes;
    LastStatus = ExecutionStatus::Success;
    return ExecutionStorageRef(State, Index, Slot.Generation);
  }

  ExecutionPlaceResult ExecutionMemoryManager::allocateCell(const StorageLayout &Layout, bool Writable, const RuntimeValue &Initial, bool Runtime)
  {
    if (Layout.Type == InvalidRuntimeType || (Initial.Initialized && Initial.Type != Layout.Type))
    {
      return {LastStatus = ExecutionStatus::TypeMismatch};
    }
    std::size_t Bytes = sizeof(ExecutionCell);
    if (Layout.Kind == RuntimeKind::Array)
    {
      if (!Layout.ElementLayout || Layout.ElementLayout->Type != Layout.ElementType)
      {
        return {LastStatus = ExecutionStatus::TypeMismatch};
      }
      std::size_t MetadataBytes = 0;
      if (!arrayMetadataBytes(Layout, MetadataBytes) || MetadataBytes > std::numeric_limits<std::size_t>::max() - Bytes)
      {
        return {LastStatus = ExecutionStatus::BudgetExceeded};
      }
      Bytes += MetadataBytes;
      if (Layout.Size > std::numeric_limits<std::size_t>::max() - Bytes)
      {
        return {LastStatus = ExecutionStatus::BudgetExceeded};
      }
      Bytes += Layout.Size;
    }
    if (!checkBudget(Bytes))
    {
      return {LastStatus};
    }
    std::unique_ptr<ExecutionCell> Cell(new ExecutionCell(Layout, Writable, Runtime));
    if (Initial.Initialized)
    {
      if (Runtime)
      {
        Cell->Value = Initial;
        Cell->Initialized = true;
      }
      else if (const ExecutionStatus Status = Cell->storeRuntime(Initial); Status != ExecutionStatus::Success)
      {
        return {LastStatus = Status};
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
    if (Pointer.kind() == ExecutionPointer::Kind::Place && !owns(Pointer.place().storage()))
    {
      return ExecutionStatus::InvalidPlace;
    }
    if (Pointer.kind() == ExecutionPointer::Kind::Buffer && !owns(Pointer.bufferRef()))
    {
      return ExecutionStatus::InvalidPlace;
    }
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
    if (!Address)
    {
      return ExecutionPointer::fromNative(nullptr);
    }
    const std::uintptr_t Requested = reinterpret_cast<std::uintptr_t>(Address);
    ExecutionPointer OnePast = ExecutionPointer::fromNative(Address);
    for (std::size_t Index = 0; Index < State->Slots.size(); ++Index)
    {
      const auto &Slot = State->Slots[Index];
      if (!Slot.Storage)
      {
        continue;
      }
      const ExecutionStorageRef Reference(State, Index, Slot.Generation);
      const ExecutionCell *Cell = Reference.cell();
      const ExecutionBuffer *Buffer = Reference.buffer();
      const void *Data = Cell ? Cell->data() : Buffer->data();
      const std::size_t Size = Cell ? Cell->size() : Buffer->size();
      if (!Data)
      {
        continue;
      }
      const std::uintptr_t Begin = reinterpret_cast<std::uintptr_t>(Data);
      if (Requested < Begin || Requested - Begin > Size)
      {
        continue;
      }
      const std::size_t Offset = static_cast<std::size_t>(Requested - Begin);
      const ExecutionPointer Pointer = Cell ? ExecutionPointer::fromPlace(ExecutionPlace(Reference), Offset) : ExecutionPointer::fromBuffer(Reference, Offset);
      if (Offset < Size)
      {
        return Pointer;
      }
      OnePast = Pointer;
    }
    // An interior address takes precedence over an adjacent one-past address.
    return OnePast;
  }
} // namespace ink::execution
