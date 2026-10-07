#include "ink/ir/type/type.h"

namespace ink::ir
{
  Type::Type(ValueKind Kind) noexcept
      : Value(Kind)
  {
  }

  Type::~Type() noexcept = default;
} // namespace ink::ir
