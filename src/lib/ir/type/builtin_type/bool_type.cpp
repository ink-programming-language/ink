#include "ink/ir/type/builtin_type/bool_type.h"

namespace ink::ir
{
  BoolType::BoolType(const MetaType &Meta) noexcept
      : Type(ValueKind::Bool, Meta)
  {
  }

  std::uint64_t BoolType::size() const noexcept
  {
    return 1;
  }

  std::uint64_t BoolType::alignment() const noexcept
  {
    return 1;
  }

  bool BoolType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Bool;
  }
} // namespace ink::ir
