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
    if (Value.kind() == ExecutionValueKind::Pointer)
    {
      const ExecutionPointer &Pointer = Value.pointer();
      if (Pointer.kind() == ExecutionPointer::Kind::Place)
      {
        const ExecutionStatus Status = validatePlace(Pointer.place());
        if (Status != ExecutionStatus::Success)
        {
          return Status;
        }
      }
      const ExecutionStatus Status = Pointer.status();
      if (Status != ExecutionStatus::Success)
      {
        return Status;
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
    const ExecutionPointer &Pointer = Address.pointer();
    const ir::Type &Pointee = static_cast<const ir::PointerType &>(*Address.type()).pointeeType();
    if (Pointer.kind() == ExecutionPointer::Kind::Place)
    {
      ExecutionValueResult Result = loadValue(Pointer.place());
      return Result && Result.Value.type() != &Pointee ? ExecutionValueResult{ExecutionStatus::TypeMismatch} : Result;
    }
    if (Pointer.kind() == ExecutionPointer::Kind::Buffer)
    {
      if (Pointer.offset() >= Pointer.buffer()->size())
      {
        return {ExecutionStatus::InvalidPlace};
      }
      if (!ir::IntegerType::classof(&Pointee) || static_cast<const ir::IntegerType &>(Pointee).bitWidth() != 8)
      {
        return {ExecutionStatus::UnsupportedOperation};
      }
      const auto Byte = static_cast<unsigned char>(Pointer.buffer()->data()[Pointer.offset()]);
      return {ExecutionStatus::Success, Heap.integer(Pointee, ExecutionInteger(8, Byte))};
    }
    // Opaque native addresses can cross FFI, but are not unchecked host loads.
    return {Pointer.kind() == ExecutionPointer::Kind::Null ? ExecutionStatus::InvalidPlace : ExecutionStatus::UnsupportedOperation};
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
    const ExecutionPointer &Pointer = Address.pointer();
    if (Pointer.kind() == ExecutionPointer::Kind::Place)
    {
      return storeValue(Pointer.place(), Value);
    }
    if (Pointer.kind() == ExecutionPointer::Kind::Buffer)
    {
      if (Pointer.offset() >= Pointer.buffer()->size())
      {
        return ExecutionStatus::InvalidPlace;
      }
      if (Value.kind() != ExecutionValueKind::Integer || Value.integer().bitWidth() != 8)
      {
        return ExecutionStatus::UnsupportedOperation;
      }
      Pointer.buffer()->data()[Pointer.offset()] = static_cast<char>(Value.integer().bits().words()[0]);
      return ExecutionStatus::Success;
    }
    return Pointer.kind() == ExecutionPointer::Kind::Null ? ExecutionStatus::InvalidPlace : ExecutionStatus::UnsupportedOperation;
  }
} // namespace ink::execution
