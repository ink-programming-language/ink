#include "ink/ir/type/structure_type/reference_type.h"

#include "../../value_string_builder.h"

namespace ink::ir
{
  ReferenceType::ReferenceType(const MetaType &Meta, const target::TargetContext &Target, const Type *ReferencedType, bool Mutable) noexcept
      : Type(ValueKind::Reference, Meta, Mutable),
        ReferencedType(ReferencedType),
        PointerByteWidth(Target.pointerByteWidth())
  {
  }

  const Type *ReferenceType::referencedType() const noexcept
  {
    return ReferencedType;
  }

  std::uint64_t ReferenceType::size() const noexcept
  {
    return PointerByteWidth;
  }

  std::uint64_t ReferenceType::alignment() const noexcept
  {
    return PointerByteWidth;
  }

  std::string ReferenceType::toString() const
  {
    return ValueStringBuilder::format(*this);
  }

  void ReferenceType::appendToString(ValueStringBuilder &Builder) const
  {
    Builder.append(isMutable() ? "" : "const ");
    Builder.append("&");
    Builder.pushValue(*ReferencedType);
  }

  bool ReferenceType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Reference;
  }
} // namespace ink::ir
