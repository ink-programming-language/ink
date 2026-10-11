#include "ink/ir/type/void_type.h"

namespace ink::ir
{
  VoidType::VoidType() noexcept
      : Type(ValueKind::Void)
  {
  }

  std::uint64_t VoidType::size() const noexcept
  {
    return 0;
  }

  std::uint64_t VoidType::alignment() const noexcept
  {
    return 1;
  }

  bool VoidType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Void;
  }
} // namespace ink::ir
