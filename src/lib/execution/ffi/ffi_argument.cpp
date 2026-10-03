#include "ink/execution/ffi/ffi_argument.h"

#include "ink/execution/ffi/ffi_type.h"
#include "ink/execution/bridge/semantic_value_bridge.h"
#include "ink/ir/context.h"

#include <bit>
#include <limits>

namespace ink::execution
{
  namespace
  {
    bool isStringPointer(const ir::Type &Type) noexcept
    {
      if (Type.typeKind() != ir::TypeKind::Pointer)
      {
        return false;
      }
      const ir::Type &Pointee = static_cast<const ir::PointerType &>(Type).pointeeType();
      if (Pointee.typeKind() == ir::TypeKind::Void)
      {
        return true;
      }
      if (Pointee.typeKind() != ir::TypeKind::Integer)
      {
        return false;
      }
      const auto &Integer = static_cast<const ir::IntegerType &>(Pointee);
      return Integer.bitWidth() == 8 && !Integer.isSigned();
    }
  } // namespace

  static_assert(sizeof(float) == sizeof(std::uint32_t) && std::numeric_limits<float>::is_iec559);
  static_assert(sizeof(double) == sizeof(std::uint64_t) && std::numeric_limits<double>::is_iec559);

  FfiArgument::~FfiArgument()
  {
    reset();
  }

  void FfiArgument::reset() noexcept
  {
    Address = nullptr;
    Scalar.Unsigned64 = 0;
    if (TemporaryBuffer)
    {
      Owner->release(Buffer);
    }
    Buffer = {};
    Owner = nullptr;
    TemporaryBuffer = false;
  }

  void FfiArgument::promoteBuffer() noexcept
  {
    TemporaryBuffer = false;
  }

  ExecutionStatus FfiArgument::prepare(ExecutionHeap &Heap, const ir::Type &Type, const ExecutionValueRef &Value)
  {
    reset();
    if (&Type.context() != &Heap.context())
    {
      return ExecutionStatus::ForeignContext;
    }
    if (!Value.type())
    {
      return ExecutionStatus::InvalidArguments;
    }
    if (&Value.type()->context() != &Type.context())
    {
      return ExecutionStatus::ForeignContext;
    }
    if (!ffiType(Type, FfiTypeUsage::Argument))
    {
      return ExecutionStatus::UnsupportedExternalSignature;
    }
    const bool IsStringPointer = Value.kind() == ExecutionValueKind::String && isStringPointer(Type);
    if (Value.type() != &Type && !IsStringPointer)
    {
      return ExecutionStatus::TypeMismatch;
    }
    if (Value.kind() == ExecutionValueKind::Pointer && Value.pointer().status() != ExecutionStatus::Success)
    {
      return Value.pointer().status();
    }
    if (!Value.valid())
    {
      return ExecutionStatus::InvalidArguments;
    }

    switch (Type.typeKind())
    {
    case ir::TypeKind::Bool:
      return Value.kind() == ExecutionValueKind::Boolean ? prepareBoolean(Value) : ExecutionStatus::TypeMismatch;
    case ir::TypeKind::Integer:
      return Value.kind() == ExecutionValueKind::Integer ? prepareInteger(static_cast<const ir::IntegerType &>(Type).bitWidth(), static_cast<const ir::IntegerType &>(Type).isSigned(), Value.integer().bits().words().front()) : ExecutionStatus::TypeMismatch;
    case ir::TypeKind::Float:
      return Value.kind() == ExecutionValueKind::Float ? prepareFloat(static_cast<const ir::FloatType &>(Type).bitWidth(), Value.floating().bits()) : ExecutionStatus::TypeMismatch;
    case ir::TypeKind::Pointer:
      return IsStringPointer ? prepareString(Heap, Value.string()) : preparePointer(Heap, Value);
    default:
      return ExecutionStatus::UnsupportedExternalSignature;
    }
  }

  ExecutionStatus FfiArgument::prepare(ExecutionHeap &Heap, const ir::Type &Type, const ir::Value &Value)
  {
    reset();
    if (&Value.context() != &Type.context() || &Type.context() != &Heap.context())
    {
      return ExecutionStatus::ForeignContext;
    }
    if (!ffiType(Type, FfiTypeUsage::Argument))
    {
      return ExecutionStatus::UnsupportedExternalSignature;
    }
    if (&Value.type() != &Type && !(ir::StringConstant::classof(&Value) && isStringPointer(Type)))
    {
      return ExecutionStatus::TypeMismatch;
    }
    if (!ir::Constant::classof(&Value))
    {
      return ExecutionStatus::RuntimeValue;
    }
    const auto &Constant = static_cast<const ir::Constant &>(Value);
    if (!Type.context().constantPool().owns(Constant))
    {
      return ExecutionStatus::ForeignContext;
    }
    const ExecutionValueRef Converted = Heap.fromConstant(Constant);
    if (!Converted.valid())
    {
      return Heap.lastStatus();
    }
    return prepare(Heap, Type, Converted);
  }

  ExecutionStatus FfiArgument::prepareBoolean(const ExecutionValueRef &Value)
  {
    Scalar.Unsigned8 = Value.boolean() ? 1 : 0;
    Address = &Scalar.Unsigned8;
    return ExecutionStatus::Success;
  }

  ExecutionStatus FfiArgument::prepareInteger(std::uint32_t BitWidth, bool Signed, std::uint64_t Bits)
  {
    switch (BitWidth)
    {
    case 8:
      if (Signed)
      {
        Scalar.Signed8 = std::bit_cast<std::int8_t>(static_cast<std::uint8_t>(Bits));
        Address = &Scalar.Signed8;
      }
      else
      {
        Scalar.Unsigned8 = static_cast<std::uint8_t>(Bits);
        Address = &Scalar.Unsigned8;
      }
      return ExecutionStatus::Success;
    case 16:
      if (Signed)
      {
        Scalar.Signed16 = std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(Bits));
        Address = &Scalar.Signed16;
      }
      else
      {
        Scalar.Unsigned16 = static_cast<std::uint16_t>(Bits);
        Address = &Scalar.Unsigned16;
      }
      return ExecutionStatus::Success;
    case 32:
      if (Signed)
      {
        Scalar.Signed32 = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(Bits));
        Address = &Scalar.Signed32;
      }
      else
      {
        Scalar.Unsigned32 = static_cast<std::uint32_t>(Bits);
        Address = &Scalar.Unsigned32;
      }
      return ExecutionStatus::Success;
    case 64:
      if (Signed)
      {
        Scalar.Signed64 = std::bit_cast<std::int64_t>(Bits);
        Address = &Scalar.Signed64;
      }
      else
      {
        Scalar.Unsigned64 = Bits;
        Address = &Scalar.Unsigned64;
      }
      return ExecutionStatus::Success;
    default:
      return ExecutionStatus::UnsupportedExternalSignature;
    }
  }

  ExecutionStatus FfiArgument::prepareFloat(std::uint32_t BitWidth, std::uint64_t Bits)
  {
    switch (BitWidth)
    {
    case 32:
      Scalar.Float32 = std::bit_cast<float>(static_cast<std::uint32_t>(Bits));
      Address = &Scalar.Float32;
      return ExecutionStatus::Success;
    case 64:
      Scalar.Float64 = std::bit_cast<double>(Bits);
      Address = &Scalar.Float64;
      return ExecutionStatus::Success;
    default:
      return ExecutionStatus::UnsupportedExternalSignature;
    }
  }

  ExecutionStatus FfiArgument::prepareString(ExecutionHeap &Heap, std::string_view Value)
  {
    return prepareString(Heap.memoryManager(), Value);
  }

  ExecutionStatus FfiArgument::prepareString(ExecutionMemoryManager &Memory, std::string_view Value)
  {
    Buffer = Memory.allocateBuffer(Value);
    if (!Buffer.valid())
    {
      return Memory.lastStatus();
    }
    Owner = &Memory;
    TemporaryBuffer = true;
    Scalar.Pointer = Buffer.buffer()->data();
    Address = &Scalar.Pointer;
    return ExecutionStatus::Success;
  }

  ExecutionStatus FfiArgument::preparePointer(ExecutionHeap &Heap, const ExecutionValueRef &Value)
  {
    if (Value.kind() != ExecutionValueKind::Pointer)
    {
      return ExecutionStatus::TypeMismatch;
    }
    const ExecutionPointer &Pointer = Value.pointer();
    if (Pointer.kind() == ExecutionPointer::Kind::Place)
    {
      const ExecutionStorageRef &Storage = Pointer.place().storage();
      if (!Heap.owns(Storage))
      {
        return ExecutionStatus::ForeignContext;
      }
      const ExecutionCell &Cell = *Storage.cell();
      if (Cell.layout().Domain != Heap.bridge().types()->domain())
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
      const auto &Type = static_cast<const ir::PointerType &>(*Value.type());
      if (Type.access() == ir::AccessKind::ReadWrite && !Cell.writable())
      {
        return ExecutionStatus::ReadOnly;
      }
      const ir::Type &Pointee = Type.pointeeType();
      const bool ByteAccess = ir::IntegerType::classof(&Pointee) && static_cast<const ir::IntegerType &>(Pointee).bitWidth() == 8 && !static_cast<const ir::IntegerType &>(Pointee).isSigned();
      if (!ByteAccess && Pointee.typeKind() != ir::TypeKind::Void && (&Pointee != Heap.bridge().sourceType(Cell.type()) || (Pointer.offset() != 0 && Pointer.offset() != Cell.size())))
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
      if (!Heap.owns(Pointer.bufferRef()))
      {
        return ExecutionStatus::ForeignContext;
      }
      const auto &Pointee = static_cast<const ir::PointerType &>(*Value.type()).pointeeType();
      if (Pointee.typeKind() != ir::TypeKind::Void && (!ir::IntegerType::classof(&Pointee) || static_cast<const ir::IntegerType &>(Pointee).bitWidth() != 8))
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
