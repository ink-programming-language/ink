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
    const ExecutionStorageRef Storage = Memory.storageFromAddress(Pointer.address());
    if (const ExecutionCell *Cell = Storage.cell())
    {
      if (Cell->runtime())
      {
        return ExecutionStatus::RuntimeValue;
      }
      if (Cell->layout().Domain != Layout.Domain)
      {
        return ExecutionStatus::ForeignContext;
      }
      if (!Cell->initialized())
      {
        return ExecutionStatus::Uninitialized;
      }
      if (Layout.Writable && !Cell->writable())
      {
        return ExecutionStatus::ReadOnly;
      }
      const std::size_t Offset = reinterpret_cast<std::uintptr_t>(Pointer.address()) - reinterpret_cast<std::uintptr_t>(Cell->data());
      if (!Layout.BytePointer && !Layout.VoidPointer && !Cell->elementLayout(Offset, Layout.Pointee, true))
      {
        return ExecutionStatus::TypeMismatch;
      }
    }
    if (Storage.buffer())
    {
      Buffer = Storage;
    }
    Scalar.Pointer = Pointer.address();
    Address = &Scalar.Pointer;
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
