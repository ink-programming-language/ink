#ifndef INK_IR_LOGICAL_NOT_INSTRUCTION_H
#define INK_IR_LOGICAL_NOT_INSTRUCTION_H

#include "ink/ir/value.h"

namespace ink::ir
{
  class LogicalNotInstruction final : public Value
  {
    public:
      const Value &operand() const noexcept
      {
        return Operand;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::LogicalNotInstruction;
      }

    private:
      explicit LogicalNotInstruction(const Value &Operand) noexcept
          : Value(Operand.context(), ValueKind::LogicalNotInstruction, Operand.type()),
            Operand(Operand)
      {
      }

      const Value &Operand;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
