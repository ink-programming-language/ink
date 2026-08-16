#include "ink/ir/control_flow.h"

namespace ink::ir
{
  const char *comparePredicateName(ComparePredicate Predicate) noexcept
  {
    switch (Predicate)
    {
    case ComparePredicate::Equal:
      return "eq";
    case ComparePredicate::NotEqual:
      return "ne";
    case ComparePredicate::LessThan:
      return "lt";
    case ComparePredicate::LessEqual:
      return "le";
    case ComparePredicate::GreaterThan:
      return "gt";
    case ComparePredicate::GreaterEqual:
      return "ge";
    case ComparePredicate::Count:
      return "unknown";
    }
    return "unknown";
  }
} // namespace ink::ir
