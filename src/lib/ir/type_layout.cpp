#include "ink/ir/type_layout.h"

#include <algorithm>
#include <limits>

namespace ink::ir
{
  namespace
  {
    std::optional<std::size_t> checkedAdd(std::size_t Left, std::size_t Right) noexcept
    {
      if (Right > std::numeric_limits<std::size_t>::max() - Left)
      {
        return std::nullopt;
      }
      return Left + Right;
    }

    std::optional<std::size_t> checkedAlign(std::size_t Value, std::size_t Alignment) noexcept
    {
      if (Alignment == 0)
      {
        return std::nullopt;
      }
      const std::size_t Remainder = Value % Alignment;
      return Remainder == 0 ? std::optional<std::size_t>(Value) : checkedAdd(Value, Alignment - Remainder);
    }

    TypeLayout scalarLayout(std::size_t Size)
    {
      return TypeLayout{Size, Size, Size, {}};
    }
  } // namespace

  std::optional<TypeLayout> computeTypeLayout(const Type &TypeValue, const core::TargetContext &Target)
  {
    switch (TypeValue.kind())
    {
    case TypeKind::Bool:
    case TypeKind::Byte:
      return scalarLayout(1);
    case TypeKind::F16:
      return scalarLayout(2);
    case TypeKind::I32:
    case TypeKind::F32:
      return scalarLayout(4);
    case TypeKind::F64:
      return scalarLayout(8);
    case TypeKind::PointerSize:
    case TypeKind::BytePointer:
    case TypeKind::ConstBytePointer:
      return scalarLayout(Target.pointerByteWidth());
    case TypeKind::Struct:
    {
      const StructType &Struct = static_cast<const StructType &>(TypeValue);
      TypeLayout Result;
      std::size_t NextFieldOffset = 0;
      Result.FieldOffsets.reserve(Struct.fieldTypes().size());
      for (const Type *FieldType : Struct.fieldTypes())
      {
        if (FieldType == nullptr)
        {
          return std::nullopt;
        }
        const std::optional<TypeLayout> FieldLayout = computeTypeLayout(*FieldType, Target);
        if (!FieldLayout.has_value())
        {
          return std::nullopt;
        }
        const std::optional<std::size_t> FieldOffset = checkedAlign(NextFieldOffset, FieldLayout->Alignment);
        if (!FieldOffset.has_value())
        {
          return std::nullopt;
        }
        const std::optional<std::size_t> OccupiedEnd = checkedAdd(*FieldOffset, FieldLayout->Size);
        const std::optional<std::size_t> AllocationEnd = checkedAdd(*FieldOffset, FieldLayout->StrideSize);
        if (!OccupiedEnd.has_value() || !AllocationEnd.has_value())
        {
          return std::nullopt;
        }
        Result.FieldOffsets.push_back(*FieldOffset);
        Result.Size = *OccupiedEnd;
        NextFieldOffset = *AllocationEnd;
        Result.Alignment = std::max(Result.Alignment, FieldLayout->Alignment);
      }
      const std::optional<std::size_t> StrideSize = checkedAlign(Result.Size, Result.Alignment);
      if (!StrideSize.has_value())
      {
        return std::nullopt;
      }
      Result.StrideSize = *StrideSize;
      return Result;
    }
    case TypeKind::Void:
    case TypeKind::ByteSlice:
    case TypeKind::ConstByteSlice:
    case TypeKind::Count:
      return std::nullopt;
    }
    return std::nullopt;
  }

  bool isMemoryValueType(const Type &TypeValue) noexcept
  {
    switch (TypeValue.kind())
    {
    case TypeKind::Bool:
    case TypeKind::Byte:
    case TypeKind::I32:
    case TypeKind::PointerSize:
    case TypeKind::F16:
    case TypeKind::F32:
    case TypeKind::F64:
      return true;
    case TypeKind::Struct:
    {
      const StructType &Struct = static_cast<const StructType &>(TypeValue);
      for (const Type *FieldType : Struct.fieldTypes())
      {
        if (FieldType == nullptr || !isMemoryValueType(*FieldType))
        {
          return false;
        }
      }
      return true;
    }
    case TypeKind::Void:
    case TypeKind::BytePointer:
    case TypeKind::ConstBytePointer:
    case TypeKind::ByteSlice:
    case TypeKind::ConstByteSlice:
    case TypeKind::Count:
      return false;
    }
    return false;
  }
} // namespace ink::ir
