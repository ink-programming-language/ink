#ifndef INK_IR_LOGICAL_OR_INSTRUCTION_H
#define INK_IR_LOGICAL_OR_INSTRUCTION_H

#include "ink/ir/value.h"

namespace ink::ir
{
  // Both bool operands must already be available; source short-circuiting uses control flow.
  class LogicalOrInstruction final : public Value
  {
    public:
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
        return ValueObject && ValueObject->kind() == ValueKind::LogicalOrInstruction;
      }

    private:
      LogicalOrInstruction(const Value &Left, const Value &Right) noexcept
          : Value(Left.context(), ValueKind::LogicalOrInstruction, Left.type()),
            Left(Left),
            Right(Right)
      {
      }

      const Value &Left;
      const Value &Right;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
