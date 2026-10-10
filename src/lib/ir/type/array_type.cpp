#include "ink/ir/type/array_type.h"
#include "type_layout.h"

#include <limits>

namespace ink::ir
{
  std::unique_ptr<ArrayType> ArrayType::create(const Type *ElementType, std::uint64_t ElementCount)
  {
    if (!ElementType)
    {
      return nullptr;
    }
    const std::uint64_t Alignment = ElementType->alignment();
    if (!detail::validAlignment(Alignment))
    {
      return nullptr;
    }
    std::uint64_t Size = 0;
    if (ElementCount != 0)
    {
      const std::uint64_t ElementSize = ElementType->size();
      const std::uint64_t MaximumSize = std::numeric_limits<std::uint64_t>::max();
      const std::uint64_t Padding = (Alignment - ElementSize % Alignment) % Alignment;
      if (ElementSize > MaximumSize - Padding)
      {
        return nullptr;
      }
      const std::uint64_t Stride = ElementSize + Padding;
      if (Stride != 0 && ElementCount > MaximumSize / Stride)
      {
        return nullptr;
      }
      Size = Stride * ElementCount;
    }
    return std::unique_ptr<ArrayType>(new ArrayType(ElementType, ElementCount, Size, Alignment));
  }

  ArrayType::ArrayType(const Type *ElementType, std::uint64_t ElementCount, std::uint64_t Size, std::uint64_t Alignment) noexcept
      : Type(ValueKind::Array),
        ElementType(ElementType),
        ElementCount(ElementCount),
        Size(Size),
        Alignment(Alignment)
  {
  }

  const Type *ArrayType::elementType() const noexcept
  {
    return ElementType;
  }

  std::uint64_t ArrayType::elementCount() const noexcept
  {
    return ElementCount;
  }

  std::uint64_t ArrayType::size() const noexcept
  {
    return Size;
  }

  std::uint64_t ArrayType::alignment() const noexcept
  {
    return Alignment;
  }

  bool ArrayType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Array;
  }
} // namespace ink::ir
