#include "ink/ir/type/structure_type/pointer_type.h"

#include "../../value_string_builder.h"

namespace ink::ir
{
  PointerType::PointerType(const MetaType &Meta, const target::TargetContext &Target, const Type *PointeeType) noexcept
      : Type(ValueKind::Pointer, Meta),
        PointeeType(PointeeType),
        PointerByteWidth(Target.pointerByteWidth())
  {
  }

  const Type *PointerType::pointeeType() const noexcept
  {
    return PointeeType;
  }

  bool PointerType::isOpaque() const noexcept
  {
    return PointeeType->kind() == ValueKind::Void;
  }

  std::uint64_t PointerType::size() const noexcept
  {
    return PointerByteWidth;
  }

  std::uint64_t PointerType::alignment() const noexcept
  {
    return PointerByteWidth;
  }

  std::string PointerType::toString() const
  {
    return ValueStringBuilder::format(*this);
  }

  void PointerType::appendToString(ValueStringBuilder &Builder) const
  {
    Builder.append("*");
    Builder.pushValue(*PointeeType);
  }

  bool PointerType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Pointer;
  }
} // namespace ink::ir
