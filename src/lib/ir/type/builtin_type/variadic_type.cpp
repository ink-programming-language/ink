#include "ink/ir/type/builtin_type/variadic_type.h"

namespace ink::ir
{
  VariadicType::VariadicType() noexcept
      : Type(ValueKind::Variadic)
  {
  }

  bool VariadicType::hasLayout() const noexcept
  {
    return false;
  }

  std::uint64_t VariadicType::size() const noexcept
  {
    return 0;
  }

  std::uint64_t VariadicType::alignment() const noexcept
  {
    return 1;
  }

  bool VariadicType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Variadic;
  }
} // namespace ink::ir
