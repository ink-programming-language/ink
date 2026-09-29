#ifndef INK_IR_RETURN_INSTRUCTION_H
#define INK_IR_RETURN_INSTRUCTION_H

#include "ink/ir/value.h"

namespace ink::ir
{
  class Function;

  // Terminates the current invocation. A void return has no returned operand.
  class ReturnInstruction final : public Value
  {
    public:
      // Derived from outer() -> BasicBlock -> Function; null while detached.
      const Function *function() const noexcept;

      const Value *returnedValue() const noexcept
      {
        return ReturnedValue;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::ReturnInstruction;
      }

    private:
      ReturnInstruction(const IRContext &Context, const Value *ReturnedValue) noexcept;

      const Value *ReturnedValue;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
