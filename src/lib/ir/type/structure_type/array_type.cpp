#include "ink/ir/type/structure_type/array_type.h"

#include "../../value_string_builder.h"

namespace ink::ir
{
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

  std::string ArrayType::toString() const
  {
    return ValueStringBuilder::format(*this);
  }

  void ArrayType::appendToString(ValueStringBuilder &Builder) const
  {
    Builder.append("[");
    Builder.pushText("; " + std::to_string(ElementCount) + "]");
    Builder.pushValue(*ElementType);
  }

  bool ArrayType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Array;
  }
} // namespace ink::ir
