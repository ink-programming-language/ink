#include "runtime/runtime_memory.h"

#include "ink/ir/analysis/type_layout.h"
#include "ink/ir/model/struct_type.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>
#include <vector>

namespace ink::execution::detail
{
  namespace
  {
    std::uint64_t normalizeIntegerPayload(ir::TypeKind Kind, std::uint64_t Value) noexcept
    {
      if (Kind != ir::TypeKind::I32 || (Value & 0x80000000ULL) == 0)
      {
        return Value;
      }
      return Value | 0xFFFFFFFF00000000ULL;
    }

    std::optional<std::size_t> integerPayloadWidth(const ir::Type &ValueType, const core::TargetContext &Target)
    {
      const ir::TypeKind Kind = ValueType.kind();
      if (Kind != ir::TypeKind::Bool && Kind != ir::TypeKind::Byte && Kind != ir::TypeKind::I32 && Kind != ir::TypeKind::PointerSize)
      {
        return std::nullopt;
      }
      const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(ValueType, Target);
      return Layout.has_value() && Layout->Size != 0 && Layout->Size <= sizeof(std::uint64_t) ? std::optional<std::size_t>(Layout->Size) : std::nullopt;
    }

    std::uint64_t decodeTargetScalar(const std::uint8_t *Data, std::size_t Width, core::ByteOrder Order) noexcept
    {
      std::uint64_t Value = 0;
      if (Order == core::ByteOrder::LittleEndian)
      {
        for (std::size_t ByteIndex = 0; ByteIndex < Width; ++ByteIndex)
        {
          Value |= static_cast<std::uint64_t>(Data[ByteIndex]) << (ByteIndex * 8U);
        }
        return Value;
      }
      for (std::size_t ByteIndex = 0; ByteIndex < Width; ++ByteIndex)
      {
        Value = (Value << 8U) | Data[ByteIndex];
      }
      return Value;
    }

    void encodeTargetScalar(std::uint8_t *Data, std::size_t Width, core::ByteOrder Order, std::uint64_t Value) noexcept
    {
      for (std::size_t ByteIndex = 0; ByteIndex < Width; ++ByteIndex)
      {
        const std::size_t DestinationIndex = Order == core::ByteOrder::LittleEndian ? ByteIndex : Width - ByteIndex - 1;
        Data[DestinationIndex] = static_cast<std::uint8_t>(Value >> (ByteIndex * 8U));
      }
    }

    RuntimeMemoryStatus decodeValue(RuntimeValueArena &Values, const ir::Type &ValueType, const std::uint8_t *Data, const core::TargetContext &Target, RuntimeValueRef &Value)
    {
      if (ValueType.kind() == ir::TypeKind::Bool || ValueType.kind() == ir::TypeKind::Byte || ValueType.kind() == ir::TypeKind::I32 || ValueType.kind() == ir::TypeKind::PointerSize)
      {
        std::uint64_t Payload = 0;
        const RuntimeMemoryStatus Status = loadRuntimeIntegerPayload(ValueType, Data, Target, Payload);
        if (Status != RuntimeMemoryStatus::Ok)
        {
          return Status;
        }
        Value = Values.integerValue(ValueType, Payload);
        return Value == nullptr ? RuntimeMemoryStatus::InvalidRepresentation : RuntimeMemoryStatus::Ok;
      }
      if (ir::isFloatingPointType(ValueType.kind()))
      {
        const std::size_t Width = ir::floatingPointBitWidth(ValueType.kind()) / 8U;
        const std::uint64_t Bits = decodeTargetScalar(Data, Width, Target.byteOrder());
        Value = Values.floatingPointValue(ValueType, Bits);
        return Value == nullptr ? RuntimeMemoryStatus::InvalidRepresentation : RuntimeMemoryStatus::Ok;
      }
      if (ValueType.kind() != ir::TypeKind::Struct)
      {
        return RuntimeMemoryStatus::InvalidValue;
      }

      const ir::StructType &Struct = static_cast<const ir::StructType &>(ValueType);
      const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(ValueType, Target);
      if (!Layout.has_value() || Layout->FieldOffsets.size() != Struct.fieldCount())
      {
        return RuntimeMemoryStatus::InvalidValue;
      }
      std::vector<RuntimeValueRef> Fields;
      Fields.reserve(Struct.fieldCount());
      for (std::size_t FieldIndex = 0; FieldIndex < Struct.fieldCount(); ++FieldIndex)
      {
        RuntimeValueRef Field = nullptr;
        const RuntimeMemoryStatus Status = decodeValue(Values, *Struct.fieldType(FieldIndex), Data + Layout->FieldOffsets[FieldIndex], Target, Field);
        if (Status != RuntimeMemoryStatus::Ok)
        {
          return Status;
        }
        Fields.push_back(Field);
      }
      Value = Values.aggregateValue(Struct, std::move(Fields));
      return Value == nullptr ? RuntimeMemoryStatus::InvalidRepresentation : RuntimeMemoryStatus::Ok;
    }

    RuntimeMemoryStatus encodeValue(const RuntimeValue &Value, std::uint8_t *Data, const core::TargetContext &Target)
    {
      const ir::Type &ValueType = Value.type();
      if (ValueType.kind() == ir::TypeKind::Bool || ValueType.kind() == ir::TypeKind::Byte || ValueType.kind() == ir::TypeKind::I32 || ValueType.kind() == ir::TypeKind::PointerSize)
      {
        const std::optional<std::uint64_t> Payload = Value.integer();
        return Payload.has_value() ? storeRuntimeIntegerPayload(ValueType, Data, Target, *Payload) : RuntimeMemoryStatus::InvalidValue;
      }
      if (ir::isFloatingPointType(ValueType.kind()))
      {
        const std::optional<std::uint64_t> Bits = Value.floatingPointBits();
        if (!Bits.has_value() || !isValidRuntimeFloatingPointValue(ValueType, *Bits))
        {
          return RuntimeMemoryStatus::InvalidValue;
        }
        encodeTargetScalar(Data, ir::floatingPointBitWidth(ValueType.kind()) / 8U, Target.byteOrder(), *Bits);
        return RuntimeMemoryStatus::Ok;
      }
      if (ValueType.kind() != ir::TypeKind::Struct)
      {
        return RuntimeMemoryStatus::InvalidValue;
      }

      const ir::StructType &Struct = static_cast<const ir::StructType &>(ValueType);
      const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(ValueType, Target);
      if (!Layout.has_value() || Layout->FieldOffsets.size() != Struct.fieldCount() || Value.fieldCount() != Struct.fieldCount())
      {
        return RuntimeMemoryStatus::InvalidValue;
      }
      for (std::size_t FieldIndex = 0; FieldIndex < Struct.fieldCount(); ++FieldIndex)
      {
        const RuntimeValue *Field = Value.field(FieldIndex);
        if (Field == nullptr || &Field->type() != Struct.fieldType(FieldIndex))
        {
          return RuntimeMemoryStatus::InvalidValue;
        }
        const RuntimeMemoryStatus Status = encodeValue(*Field, Data + Layout->FieldOffsets[FieldIndex], Target);
        if (Status != RuntimeMemoryStatus::Ok)
        {
          return Status;
        }
      }
      return RuntimeMemoryStatus::Ok;
    }
  } // namespace

  RuntimeMemoryStatus computeElementByteOffset(const ir::Type &ElementType, std::uint64_t Index, const std::vector<std::uint32_t> &FieldIndices, const core::TargetContext &Target, std::uint64_t &ByteOffset)
  {
    const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(ElementType, Target);
    if (!Layout.has_value() || Layout->StrideSize == 0)
    {
      return RuntimeMemoryStatus::InvalidValue;
    }
    const std::uint64_t Maximum = Target.maximumPointerSizeValue();
    const std::uint64_t Stride = Layout->StrideSize;
    if (Stride > Maximum || (Index != 0 && Stride > Maximum / Index))
    {
      return RuntimeMemoryStatus::AddressOverflow;
    }
    ByteOffset = Index * Stride;
    const ir::Type *IndexedType = &ElementType;
    for (const std::uint32_t FieldIndex : FieldIndices)
    {
      if (IndexedType->kind() != ir::TypeKind::Struct)
      {
        return RuntimeMemoryStatus::InvalidValue;
      }
      const ir::StructType &Struct = static_cast<const ir::StructType &>(*IndexedType);
      const std::optional<ir::TypeLayout> StructLayout = ir::computeTypeLayout(Struct, Target);
      if (!StructLayout.has_value() || StructLayout->FieldOffsets.size() != Struct.fieldCount() || FieldIndex >= Struct.fieldCount() || Struct.fieldType(FieldIndex) == nullptr)
      {
        return RuntimeMemoryStatus::InvalidValue;
      }
      const std::size_t FieldOffset = StructLayout->FieldOffsets[FieldIndex];
      if (FieldOffset > Maximum || static_cast<std::uint64_t>(FieldOffset) > Maximum - ByteOffset)
      {
        return RuntimeMemoryStatus::AddressOverflow;
      }
      ByteOffset += static_cast<std::uint64_t>(FieldOffset);
      IndexedType = Struct.fieldType(FieldIndex);
    }
    return RuntimeMemoryStatus::Ok;
  }

  RuntimeMemoryStatus loadRuntimeMemoryValue(RuntimeValueArena &Values, const ir::Type &ValueType, const std::uint8_t *Data, const core::TargetContext &Target, RuntimeValueRef &Value)
  {
    if (Data == nullptr || !ir::isMemoryValueType(ValueType) || !ir::computeTypeLayout(ValueType, Target).has_value())
    {
      return RuntimeMemoryStatus::InvalidValue;
    }
    RuntimeValueRef LoadedValue = nullptr;
    const RuntimeMemoryStatus Status = decodeValue(Values, ValueType, Data, Target, LoadedValue);
    if (Status == RuntimeMemoryStatus::Ok)
    {
      Value = LoadedValue;
    }
    return Status;
  }

  RuntimeMemoryStatus storeRuntimeMemoryValue(const RuntimeValue &Value, std::uint8_t *Data, const core::TargetContext &Target)
  {
    const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(Value.type(), Target);
    if (Data == nullptr || !ir::isMemoryValueType(Value.type()) || !Layout.has_value() || Layout->StrideSize == 0)
    {
      return RuntimeMemoryStatus::InvalidValue;
    }
    std::vector<std::uint8_t> Staged(Data, Data + Layout->StrideSize);
    const RuntimeMemoryStatus Status = encodeValue(Value, Staged.data(), Target);
    if (Status != RuntimeMemoryStatus::Ok)
    {
      return Status;
    }
    std::memcpy(Data, Staged.data(), Staged.size());
    return RuntimeMemoryStatus::Ok;
  }

  RuntimeMemoryStatus loadRuntimeIntegerPayload(const ir::Type &ValueType, const std::uint8_t *Data, const core::TargetContext &Target, std::uint64_t &Value)
  {
    const std::optional<std::size_t> Width = integerPayloadWidth(ValueType, Target);
    if (!Width.has_value() || Data == nullptr)
    {
      return RuntimeMemoryStatus::InvalidValue;
    }
    const std::uint64_t Payload = normalizeIntegerPayload(ValueType.kind(), decodeTargetScalar(Data, *Width, Target.byteOrder()));
    if (!isValidRuntimeIntegerValue(ValueType, Payload, Target))
    {
      return RuntimeMemoryStatus::InvalidRepresentation;
    }
    Value = Payload;
    return RuntimeMemoryStatus::Ok;
  }

  RuntimeMemoryStatus storeRuntimeIntegerPayload(const ir::Type &ValueType, std::uint8_t *Data, const core::TargetContext &Target, std::uint64_t Value)
  {
    const std::optional<std::size_t> Width = integerPayloadWidth(ValueType, Target);
    if (!Width.has_value() || Data == nullptr || !isValidRuntimeIntegerValue(ValueType, Value, Target))
    {
      return RuntimeMemoryStatus::InvalidValue;
    }
    encodeTargetScalar(Data, *Width, Target.byteOrder(), Value);
    return RuntimeMemoryStatus::Ok;
  }
} // namespace ink::execution::detail
