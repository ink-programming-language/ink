#ifndef INK_CORE_COMPARISON_PREDICATE_H
#define INK_CORE_COMPARISON_PREDICATE_H

namespace ink::core
{
  // Comparison relation shared by IR and execution; operand types determine signedness.
  enum class ComparisonPredicate
  {
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
  };
} // namespace ink::core

#endif
