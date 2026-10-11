#include "ink/ir/type/pointer_type.h"

namespace ink::ir
{
  PointerType::PointerType(const target::TargetContext &Target, const Type *PointeeType) noexcept
      : Type(ValueKind::Pointer),
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

  bool PointerType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Pointer;
  }
} // namespace ink::ir
