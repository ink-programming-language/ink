#ifndef INK_IR_COMPARE_INSTRUCTION_H
#define INK_IR_COMPARE_INSTRUCTION_H

#include "ink/ir/value.h"

namespace ink::ir
{
  enum class ComparisonPredicate
  {
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
  };

  // Compares identically typed integers, or bools for equality, producing a bool.
  class CompareInstruction final : public Value
  {
    public:
      ComparisonPredicate predicate() const noexcept
      {
        return Predicate;
      }

      const Value &left() const noexcept
      {
        return Left;
      }

      const Value &right() const noexcept
      {
        return Right;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::CompareInstruction;
      }

    private:
      CompareInstruction(const Type &BoolType, ComparisonPredicate Predicate, const Value &Left, const Value &Right) noexcept
          : Value(Left.context(), ValueKind::CompareInstruction, BoolType),
            Predicate(Predicate),
            Left(Left),
            Right(Right)
      {
      }

      ComparisonPredicate Predicate;
      const Value &Left;
      const Value &Right;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
