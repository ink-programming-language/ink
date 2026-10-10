#include "ink/ir/type/reference_type.h"

namespace ink::ir
{
  ReferenceType::ReferenceType(const core::TargetContext &Target, const Type *ReferencedType) noexcept
      : Type(ValueKind::Reference),
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

  bool ReferenceType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Reference;
  }
} // namespace ink::ir
