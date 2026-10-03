#include "ink/execution/ffi/ffi_argument.h"

namespace ink::execution
{
  ExecutionStatus FfiArgument::prepare(ExecutionMemoryManager &Memory, const FfiValueLayout &Layout, const RuntimeValue &Value)
  {
    reset();
    if (!Value.Initialized)
    {
      return ExecutionStatus::InvalidArguments;
    }
    const bool StringPointer = Value.kind() == RuntimeKind::String && Layout.Kind == RuntimeKind::Pointer && (Layout.VoidPointer || Layout.BytePointer);
    if (Value.Type != Layout.Type && !StringPointer)
    {
      return ExecutionStatus::TypeMismatch;
    }
    switch (Layout.Kind)
    {
    case RuntimeKind::Boolean:
      if (Value.Object)
      {
        return ExecutionStatus::TypeMismatch;
      }
      Scalar.Unsigned8 = Value.Bits != 0;
      Address = &Scalar.Unsigned8;
      return ExecutionStatus::Success;
    case RuntimeKind::Integer:
      return Value.Object ? ExecutionStatus::TypeMismatch : prepareInteger(Layout.BitWidth, Layout.Signed, Value.Bits);
    case RuntimeKind::Float:
      return Value.Object ? ExecutionStatus::TypeMismatch : prepareFloat(Layout.BitWidth, Value.Bits);
    case RuntimeKind::Pointer:
      if (StringPointer)
      {
        return prepareString(Memory, Value.string());
      }
      return Value.kind() == RuntimeKind::Pointer ? preparePointer(Memory, Layout, Value.pointer()) : ExecutionStatus::TypeMismatch;
    default:
      return ExecutionStatus::UnsupportedExternalSignature;
    }
  }

  ExecutionStatus FfiArgument::preparePointer(ExecutionMemoryManager &Memory, const FfiValueLayout &Layout, const ExecutionPointer &Pointer)
  {
    const ExecutionStatus Status = Pointer.status();
    if (Status != ExecutionStatus::Success)
    {
      return Status;
    }
    if (Pointer.kind() == ExecutionPointer::Kind::Place)
    {
      const ExecutionStorageRef &Storage = Pointer.place().storage();
      if (!Memory.owns(Storage))
      {
        return ExecutionStatus::ForeignContext;
      }
      const ExecutionCell &Cell = *Storage.cell();
      if (Cell.layout().Domain != Layout.Domain)
      {
        return ExecutionStatus::ForeignContext;
      }
      if (Cell.runtime())
      {
        return ExecutionStatus::RuntimeValue;
      }
      if (!Cell.initialized())
      {
        return ExecutionStatus::Uninitialized;
      }
      if (Layout.Writable && !Cell.writable())
      {
        return ExecutionStatus::ReadOnly;
      }
      if (!Layout.BytePointer && !Layout.VoidPointer && (Layout.Pointee != Cell.type() || (Pointer.offset() != 0 && Pointer.offset() != Cell.size())))
      {
        return ExecutionStatus::TypeMismatch;
      }
      if (!Cell.data())
      {
        return ExecutionStatus::UnsupportedExternalSignature;
      }
    }
    if (Pointer.kind() == ExecutionPointer::Kind::Buffer)
    {
      if (!Memory.owns(Pointer.bufferRef()))
      {
        return ExecutionStatus::ForeignContext;
      }
      if (!Layout.VoidPointer && !Layout.ByteBufferPointer)
      {
        return ExecutionStatus::UnsupportedExternalSignature;
      }
      Buffer = Pointer.bufferRef();
    }
    Scalar.Pointer = Pointer.address();
    Address = &Scalar.Pointer;
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
