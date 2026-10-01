#include "ink/execution/memory/execution_heap.h"

#include "ink/execution/value/execution_bool_value.h"
#include "ink/execution/value/execution_float_value.h"
#include "ink/execution/value/execution_function_value.h"
#include "ink/execution/value/execution_integer_value.h"
#include "ink/execution/value/execution_pointer_value.h"
#include "ink/execution/value/execution_string_value.h"
#include "ink/execution/value/execution_void_value.h"

#include "ink/ir/context.h"
#include "ink/ir/function/function.h"

#include <limits>
#include <utility>
#include <vector>

namespace ink::execution
{
  struct ExecutionHeapState
  {
      struct Slot
      {
          std::unique_ptr<ExecutionStorage> Storage;
          std::uint64_t Generation = 1;
      };

      std::vector<Slot> Slots;
      std::vector<std::size_t> FreeSlots;
      std::size_t LiveValues = 0;
      std::size_t LiveStorage = 0;
      std::size_t AllocatedStorage = 0;
      std::size_t MaxStorage;

      explicit ExecutionHeapState(std::size_t MaxStorage)
          : MaxStorage(MaxStorage)
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

  ExecutionHeap::ExecutionHeap(ir::IRContext &Context, std::size_t MaxStorage)
      : Context(Context),
        State(std::make_shared<ExecutionHeapState>(MaxStorage))
  {
  }

  ExecutionHeap::~ExecutionHeap() = default;

  ir::IRContext &ExecutionHeap::context() const noexcept
  {
    return Context;
  }

  ExecutionStatus ExecutionHeap::lastStatus() const noexcept
  {
    return LastStatus;
  }

  std::size_t ExecutionHeap::liveValueCount() const noexcept
  {
    return State->LiveValues;
  }

  std::size_t ExecutionHeap::liveStorageCount() const noexcept
  {
    return State->LiveStorage;
  }

  std::size_t ExecutionHeap::allocatedStorageCount() const noexcept
  {
    return State->AllocatedStorage;
  }

  ExecutionValueRef ExecutionHeap::ownValue(std::unique_ptr<ExecutionValue> Value)
  {
    if (!Value || !Value->type() || &Value->type()->context() != &Context)
    {
      LastStatus = ExecutionStatus::ForeignContext;
      return {};
    }
    if (!Value->valid())
    {
      LastStatus = Value->kind() == ExecutionValueKind::Pointer ? static_cast<const ExecutionPointerValue &>(*Value).value().status() : ExecutionStatus::TypeMismatch;
      if (LastStatus == ExecutionStatus::Success)
      {
        LastStatus = ExecutionStatus::TypeMismatch;
      }
      return {};
    }
    const std::weak_ptr<ExecutionHeapState> Owner = State;
    std::shared_ptr<const ExecutionValue> Shared(Value.release(), [Owner](const ExecutionValue *Object)
    {
      delete Object;
      if (const auto State = Owner.lock())
      {
        --State->LiveValues;
      }
    });
    ++State->LiveValues;
    LastStatus = ExecutionStatus::Success;
    return ExecutionValueRef(std::move(Shared));
  }

  ExecutionValueRef ExecutionHeap::fromConstant(const ir::Constant &Value)
  {
    if (&Value.context() != &Context || !Context.constantPool().owns(Value))
    {
      LastStatus = ExecutionStatus::ForeignContext;
      return {};
    }
    switch (Value.kind())
    {
    case ir::ValueKind::BoolConstant:
      return boolean(Value.type(), static_cast<const ir::BoolConstant &>(Value).value());
    case ir::ValueKind::IntegerConstant:
      return integer(Value.type(), ExecutionInteger(static_cast<const ir::IntegerConstant &>(Value).value()));
    case ir::ValueKind::FloatConstant:
      return floating(Value.type(), static_cast<const ir::FloatConstant &>(Value).value());
    case ir::ValueKind::StringConstant:
      return string(Value.type(), static_cast<const ir::StringConstant &>(Value).value());
    default:
      LastStatus = ExecutionStatus::UnsupportedOperation;
      return {};
    }
  }

  ExecutionValueRef ExecutionHeap::boolean(const ir::Type &Type, bool Value)
  {
    return ownValue(std::unique_ptr<ExecutionValue>(new ExecutionBoolValue(Type, Value)));
  }

  ExecutionValueRef ExecutionHeap::integer(const ir::Type &Type, ExecutionInteger Value)
  {
    return ownValue(std::unique_ptr<ExecutionValue>(new ExecutionIntegerValue(Type, std::move(Value))));
  }

  ExecutionValueRef ExecutionHeap::floating(const ir::Type &Type, ir::FloatBits Value)
  {
    return ownValue(std::unique_ptr<ExecutionValue>(new ExecutionFloatValue(Type, Value)));
  }

  ExecutionValueRef ExecutionHeap::string(const ir::Type &Type, std::string_view Value)
  {
    return ownValue(std::unique_ptr<ExecutionValue>(new ExecutionStringValue(Type, Value)));
  }

  ExecutionValueRef ExecutionHeap::pointer(const ir::Type &Type, ExecutionPointer Value)
  {
    return ownValue(std::unique_ptr<ExecutionValue>(new ExecutionPointerValue(Type, std::move(Value))));
  }

  ExecutionValueRef ExecutionHeap::function(const ir::Function &Value)
  {
    return ownValue(std::unique_ptr<ExecutionValue>(new ExecutionFunctionValue(Value)));
  }

  ExecutionValueRef ExecutionHeap::voidValue(const ir::Type &Type)
  {
    return ownValue(std::unique_ptr<ExecutionValue>(new ExecutionVoidValue(Type)));
  }

  ExecutionStorageRef ExecutionHeap::ownStorage(std::unique_ptr<ExecutionStorage> Storage)
  {
    if (State->AllocatedStorage >= State->MaxStorage)
    {
      LastStatus = ExecutionStatus::BudgetExceeded;
      return {};
    }
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
    ++State->AllocatedStorage;
    ++State->LiveStorage;
    LastStatus = ExecutionStatus::Success;
    return ExecutionStorageRef(State, Index, Slot.Generation);
  }

  ExecutionPlaceResult ExecutionHeap::allocateCell(const ir::Type &Type, bool Writable, const ExecutionValueRef &Initial, bool Runtime)
  {
    if (&Type.context() != &Context || (Initial.type() && &Initial.type()->context() != &Context))
    {
      return {LastStatus = ExecutionStatus::ForeignContext};
    }
    if (Initial.get() && (!Initial.valid() || Initial.type() != &Type))
    {
      return {LastStatus = ExecutionStatus::TypeMismatch};
    }
    if (State->AllocatedStorage >= State->MaxStorage)
    {
      return {LastStatus = ExecutionStatus::BudgetExceeded};
    }
    const ExecutionStorageRef Storage = ownStorage(std::unique_ptr<ExecutionStorage>(new ExecutionCell(Type, Writable, Initial, Runtime)));
    return Storage.valid() ? ExecutionPlaceResult{ExecutionStatus::Success, ExecutionPlace(Storage)} : ExecutionPlaceResult{LastStatus};
  }

  ExecutionStorageRef ExecutionHeap::allocateBuffer(std::string_view Bytes, bool Terminate)
  {
    if (State->AllocatedStorage >= State->MaxStorage)
    {
      LastStatus = ExecutionStatus::BudgetExceeded;
      return {};
    }
    return ownStorage(std::unique_ptr<ExecutionStorage>(new ExecutionBuffer(Bytes, Terminate)));
  }

  bool ExecutionHeap::owns(const ExecutionStorageRef &Storage) const noexcept
  {
    return Storage.Generation != 0 && !Storage.State.owner_before(State) && !State.owner_before(Storage.State);
  }

  ExecutionStatus ExecutionHeap::release(const ExecutionStorageRef &Storage) noexcept
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
    // A generation never wraps; a saturated slot is permanently retired.
    if (Slot.Generation != std::numeric_limits<std::uint64_t>::max())
    {
      ++Slot.Generation;
      State->FreeSlots.push_back(Storage.Slot);
    }
    return LastStatus = ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionHeap::release(ExecutionPlace Place) noexcept
  {
    const ExecutionStatus Status = validatePlace(Place);
    return Status == ExecutionStatus::Success ? release(Place.storage()) : (LastStatus = Status);
  }

  ExecutionStatus ExecutionHeap::validatePlace(ExecutionPlace Place) const noexcept
  {
    if (!owns(Place.storage()))
    {
      return ExecutionStatus::InvalidPlace;
    }
    return Place.status();
  }

  ExecutionValueResult ExecutionHeap::load(ExecutionPlace Place)
  {
    const ExecutionStatus Status = validatePlace(Place);
    if (Status != ExecutionStatus::Success)
    {
      return {LastStatus = Status};
    }
    const ExecutionCell &Cell = *Place.storage().cell();
    if (Cell.Runtime)
    {
      return {LastStatus = ExecutionStatus::RuntimeValue};
    }
    if (!Cell.Value.get())
    {
      return {LastStatus = ExecutionStatus::Uninitialized};
    }
    return {LastStatus = ExecutionStatus::Success, Cell.Value};
  }

  ExecutionStatus ExecutionHeap::store(ExecutionPlace Place, const ExecutionValueRef &Value)
  {
    const ExecutionStatus Status = validatePlace(Place);
    if (Status != ExecutionStatus::Success)
    {
      return LastStatus = Status;
    }
    ExecutionCell &Cell = *Place.storage().cell();
    if (Cell.Runtime)
    {
      return LastStatus = ExecutionStatus::RuntimeValue;
    }
    if (!Cell.Writable && Cell.Value.get())
    {
      return LastStatus = ExecutionStatus::ReadOnly;
    }
    if (Value.type() && &Value.type()->context() != &Context)
    {
      return LastStatus = ExecutionStatus::ForeignContext;
    }
    if (!Value.valid() || Value.type() != Cell.ValueType)
    {
      return LastStatus = ExecutionStatus::TypeMismatch;
    }
    Cell.Value = Value;
    return LastStatus = ExecutionStatus::Success;
  }
} // namespace ink::execution
