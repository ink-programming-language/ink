#include "ink/ir/type/builtin_type/meta_type.h"

namespace ink::ir
{
  // Type only records this reference during base construction; it does not access the unfinished MetaType.
  MetaType::MetaType() noexcept
      : Type(ValueKind::MetaType, *this)
  {
  }

  bool MetaType::hasLayout() const noexcept
  {
    return false;
  }

  std::uint64_t MetaType::size() const noexcept
  {
    return 0;
  }

  std::uint64_t MetaType::alignment() const noexcept
  {
    return 1;
  }

  bool MetaType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::MetaType;
  }
} // namespace ink::ir
