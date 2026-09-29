#ifndef INK_IR_ADD_INSTRUCTION_H
#define INK_IR_ADD_INSTRUCTION_H

#include "ink/ir/type/integer_type.h"

namespace ink::ir
{
  // Integer addition modulo 2^bitWidth, for both signed and unsigned types.
  class AddInstruction final : public Value
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
        return ValueObject && ValueObject->kind() == ValueKind::AddInstruction;
      }

    private:
      AddInstruction(const Value &Left, const Value &Right) noexcept
          : Value(Left.context(), ValueKind::AddInstruction, Left.type()),
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
