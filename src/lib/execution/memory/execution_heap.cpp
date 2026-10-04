#include "ink/execution/memory/execution_heap.h"
#include "ink/execution/value/execution_array_value.h"
#include "ink/execution/value/execution_class_value.h"
#include "ink/ir/constant/class_constant.h"
#include "ink/ir/constant/array_constant.h"

#include "ink/execution/value/execution_bool_value.h"
#include "ink/execution/value/execution_float_value.h"
#include "ink/execution/value/execution_function_value.h"
#include "ink/execution/value/execution_integer_value.h"
#include "ink/execution/value/execution_pointer_value.h"
#include "ink/execution/value/execution_string_value.h"
#include "ink/execution/value/execution_void_value.h"

#include "ink/ir/context.h"
#include "ink/ir/function/function.h"

#include <utility>

namespace ink::execution
{
  struct ExecutionHeapState
  {
      std::size_t LiveValues = 0;
  };

  ExecutionHeap::ExecutionHeap(ir::IRContext &Context, std::size_t MaxStorage, std::size_t MaxStorageBytes)
      : Context(Context),
        State(std::make_shared<ExecutionHeapState>()),
        Bridge(Context, true),
        Memory(MaxStorage, MaxStorageBytes)
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
    return Memory.liveStorageCount();
  }

  std::size_t ExecutionHeap::allocatedStorageCount() const noexcept
  {
    return Memory.allocatedStorageCount();
  }

  std::size_t ExecutionHeap::liveStorageBytes() const noexcept
  {
    return Memory.liveStorageBytes();
  }

  std::size_t ExecutionHeap::allocatedStorageBytes() const noexcept
  {
    return Memory.allocatedStorageBytes();
  }

  ExecutionMemoryManager &ExecutionHeap::memoryManager() noexcept
  {
    return Memory;
  }

  const ExecutionMemoryManager &ExecutionHeap::memoryManager() const noexcept
  {
    return Memory;
  }

  SemanticValueBridge &ExecutionHeap::bridge() noexcept
  {
    return Bridge;
  }

  const SemanticValueBridge &ExecutionHeap::bridge() const noexcept
  {
    return Bridge;
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
      LastStatus = ExecutionStatus::TypeMismatch;
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
    case ir::ValueKind::ArrayConstant:
    {
      std::vector<ExecutionValueRef> Elements;
      for (const ir::Constant *Element : static_cast<const ir::ArrayConstant &>(Value).elements())
      {
        ExecutionValueRef Converted = fromConstant(*Element);
        if (!Converted)
        {
          return {};
        }
        Elements.push_back(std::move(Converted));
      }
      return array(Value.type(), std::move(Elements));
    }
    case ir::ValueKind::ClassConstant:
    {
      std::vector<ExecutionValueRef> Fields;
      for (const ir::Constant *Field : static_cast<const ir::ClassConstant &>(Value).fields())
      {
        ExecutionValueRef Converted = fromConstant(*Field);
        if (!Converted)
        {
          return {};
        }
        Fields.push_back(std::move(Converted));
      }
      return classValue(Value.type(), std::move(Fields));
    }
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

  ExecutionValueRef ExecutionHeap::array(const ir::Type &Type, std::vector<ExecutionValueRef> Elements)
  {
    return ownValue(std::unique_ptr<ExecutionValue>(new ExecutionArrayValue(Type, std::move(Elements))));
  }

  ExecutionValueRef ExecutionHeap::classValue(const ir::Type &Type, std::vector<ExecutionValueRef> Fields)
  {
    return ownValue(std::unique_ptr<ExecutionValue>(new ExecutionClassValue(Type, std::move(Fields))));
  }

  ExecutionValueRef ExecutionHeap::pointerSnapshot(const ir::Type &Type, ExecutionPointer Value)
  {
    return pointer(Type, Value);
  }

  ExecutionValueRef ExecutionHeap::function(const ir::Function &Value)
  {
    return ownValue(std::unique_ptr<ExecutionValue>(new ExecutionFunctionValue(Value)));
  }

  ExecutionValueRef ExecutionHeap::voidValue(const ir::Type &Type)
  {
    return ownValue(std::unique_ptr<ExecutionValue>(new ExecutionVoidValue(Type)));
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
    const RuntimeTypeId TypeId = Bridge.lowerType(Type);
    const TypeDesc *Layout = Bridge.types()->get(TypeId);
    if (!Layout)
    {
      return {LastStatus = ExecutionStatus::Overflow};
    }
    RuntimeValue InitialValue;
    if (Initial.get())
    {
      const RuntimeValueResult Converted = Bridge.lowerValue(Initial);
      if (!Converted)
      {
        return {LastStatus = Converted.Status};
      }
      InitialValue = Converted.Value;
    }
    const ExecutionPlaceResult Result = Memory.allocateCell(*Layout, Writable, InitialValue, Runtime);
    LastStatus = Result.Status;
    return Result;
  }

  ExecutionStorageRef ExecutionHeap::allocateBuffer(std::string_view Bytes, bool Terminate)
  {
    const ExecutionStorageRef Result = Memory.allocateBuffer(Bytes, Terminate);
    LastStatus = Memory.lastStatus();
    return Result;
  }

  ExecutionStorageRef ExecutionHeap::allocateBuffer(std::size_t Size)
  {
    const ExecutionStorageRef Result = Memory.allocateBuffer(Size);
    LastStatus = Memory.lastStatus();
    return Result;
  }

  bool ExecutionHeap::owns(const ExecutionStorageRef &Storage) const noexcept
  {
    return Memory.owns(Storage);
  }

  ExecutionStatus ExecutionHeap::release(const ExecutionStorageRef &Storage) noexcept
  {
    return LastStatus = Memory.release(Storage);
  }

  ExecutionStatus ExecutionHeap::release(ExecutionPlace Place) noexcept
  {
    return LastStatus = Memory.release(Place);
  }

  ExecutionStatus ExecutionHeap::validatePlace(ExecutionPlace Place) const noexcept
  {
    return Memory.owns(Place.storage()) ? Place.status() : ExecutionStatus::InvalidPlace;
  }

  ExecutionValueResult ExecutionHeap::load(ExecutionPlace Place)
  {
    const ExecutionStatus Status = validatePlace(Place);
    if (Status != ExecutionStatus::Success)
    {
      return {LastStatus = Status};
    }
    const ExecutionCell &Cell = *Place.storage().cell();
    if (Cell.layout().Domain != Bridge.types()->domain())
    {
      return {LastStatus = ExecutionStatus::TypeMismatch};
    }
    const RuntimeValueResult Loaded = Cell.loadRuntime();
    if (!Loaded)
    {
      return {LastStatus = Loaded.Status};
    }
    ExecutionValueResult Result = Bridge.raiseValue(*this, Loaded.Value, Cell.type());
    LastStatus = Result.Status;
    return Result;
  }

  ExecutionStatus ExecutionHeap::store(ExecutionPlace Place, const ExecutionValueRef &Value)
  {
    const ExecutionStatus Status = validatePlace(Place);
    if (Status != ExecutionStatus::Success)
    {
      return LastStatus = Status;
    }
    if (Value.type() && &Value.type()->context() != &Context)
    {
      return LastStatus = ExecutionStatus::ForeignContext;
    }
    ExecutionCell &Cell = *Place.storage().cell();
    if (Cell.layout().Domain != Bridge.types()->domain())
    {
      return LastStatus = ExecutionStatus::TypeMismatch;
    }
    if (Cell.runtime())
    {
      return LastStatus = ExecutionStatus::RuntimeValue;
    }
    if (!Cell.writable() && Cell.initialized())
    {
      return LastStatus = ExecutionStatus::ReadOnly;
    }
    const RuntimeValueResult Converted = Bridge.lowerValue(Value);
    if (!Converted)
    {
      return LastStatus = Converted.Status;
    }
    return LastStatus = Cell.storeRuntime(Converted.Value);
  }

  ExecutionPointer ExecutionHeap::pointerFromAddress(void *Address) const noexcept
  {
    return Memory.pointerFromAddress(Address);
  }
} // namespace ink::execution
