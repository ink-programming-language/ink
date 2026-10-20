#include "ink/ir/type/builtin_type/null_pointer_type.h"
#include "ink/ir/context.h"

namespace ink::ir
{
  NullPointerType::NullPointerType(const MetaType &Meta, const IRContext &Owner, bool Mutable) noexcept
      : Type(ValueKind::NullPointer, Meta, Mutable),
        Owner(Owner),
        PointerByteWidth(Owner.targetContext().pointerByteWidth())
  {
  }

  const IRContext &NullPointerType::irContext() const noexcept
  {
    return Owner;
  }

  std::uint64_t NullPointerType::size() const noexcept
  {
    return PointerByteWidth;
  }

  std::uint64_t NullPointerType::alignment() const noexcept
  {
    return PointerByteWidth;
  }

  bool NullPointerType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::NullPointer;
  }
} // namespace ink::ir
