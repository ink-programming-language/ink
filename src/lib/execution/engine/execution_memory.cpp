#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/context.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/pointer_type.h"

namespace ink::execution
{
  ExecutionStatus ExecutionEngine::validateValue(const ExecutionValueRef &Value) const noexcept
  {
    if (!Value.type())
    {
      return ExecutionStatus::TypeMismatch;
    }
    if (&Value.type()->context() != &Context)
    {
      return ExecutionStatus::ForeignContext;
    }
    if (Value.kind() == ExecutionValueKind::Class || Value.kind() == ExecutionValueKind::Array)
    {
      for (const ExecutionValueRef &Field : Value.kind() == ExecutionValueKind::Class ? Value.fields() : Value.array())
      {
        const ExecutionStatus Status = validateValue(Field);
        if (Status != ExecutionStatus::Success)
        {
          return Status;
        }
      }
    }
    return Value.valid() ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch;
  }

  ExecutionResult ExecutionEngine::freeze(const ExecutionValueResult &Result)
  {
    if (!Result)
    {
      return {LastStatus = Result.Status};
    }
    const ExecutionStatus Status = validateValue(Result.Value);
    if (Status != ExecutionStatus::Success)
    {
      return {LastStatus = Status};
    }
    if (Result.Value.kind() == ExecutionValueKind::Void)
    {
      LastStatus = ExecutionStatus::Success;
      return {};
    }
    const ir::Constant *Constant = Result.Value.toConstant(Context);
    if (!Constant)
    {
      return {LastStatus = ExecutionStatus::UnsupportedOperation};
    }
    LastStatus = ExecutionStatus::Success;
    return {ExecutionStatus::Success, Constant};
  }

  ExecutionValueResult ExecutionEngine::loadPointer(const ExecutionValueRef &Address)
  {
    const ExecutionStatus Status = validateValue(Address);
    if (Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    if (Address.kind() != ExecutionValueKind::Pointer)
    {
      return {ExecutionStatus::TypeMismatch};
    }
    const ir::Type &Pointee = static_cast<const ir::PointerType &>(*Address.type()).pointeeType();
    const RuntimeTypeId Type = Heap.bridge().lowerType(Pointee);
    const TypeDesc *Layout = Heap.bridge().types()->get(Type);
    if (!Layout)
    {
      return {ExecutionStatus::TypeMismatch};
    }
    if (consumeStep() != ExecutionStatus::Success)
    {
      return {LastStatus};
    }
    const RuntimeValueResult Loaded = Heap.memoryManager().loadPointer(Address.pointer(), *Layout);
    return Loaded ? Heap.bridge().raiseValue(Heap, Loaded.Value, Type) : ExecutionValueResult{Loaded.Status};
  }

  ExecutionStatus ExecutionEngine::storePointer(const ExecutionValueRef &Address, const ExecutionValueRef &Value)
  {
    const ExecutionStatus AddressStatus = validateValue(Address);
    if (AddressStatus != ExecutionStatus::Success)
    {
      return AddressStatus;
    }
    const ExecutionStatus ValueStatus = validateValue(Value);
    if (ValueStatus != ExecutionStatus::Success)
    {
      return ValueStatus;
    }
    if (Address.kind() != ExecutionValueKind::Pointer)
    {
      return ExecutionStatus::TypeMismatch;
    }
    const auto &Type = static_cast<const ir::PointerType &>(*Address.type());
    if (Type.access() != ir::AccessKind::ReadWrite)
    {
      return ExecutionStatus::ReadOnly;
    }
    if (Value.type() != &Type.pointeeType())
    {
      return ExecutionStatus::TypeMismatch;
    }
    const RuntimeTypeId Pointee = Heap.bridge().lowerType(Type.pointeeType());
    const TypeDesc *Layout = Heap.bridge().types()->get(Pointee);
    if (!Layout)
    {
      return ExecutionStatus::TypeMismatch;
    }
    if (consumeStep() != ExecutionStatus::Success)
    {
      return LastStatus;
    }
    const RuntimeValueResult Stored = Heap.bridge().lowerValue(Value);
    return Stored ? Heap.memoryManager().storePointer(Address.pointer(), *Layout, Stored.Value) : Stored.Status;
  }
} // namespace ink::execution
