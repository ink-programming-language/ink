#ifndef INK_IR_CONDITIONAL_BRANCH_INSTRUCTION_H
#define INK_IR_CONDITIONAL_BRANCH_INSTRUCTION_H

#include "ink/ir/function/basic_block.h"

namespace ink::ir
{
  // Evaluates one bool operand and selects exactly one successor in the same function.
  class ConditionalBranchInstruction final : public Value
  {
    public:
      const Value &condition() const noexcept
      {
        return Condition;
      }

      const BasicBlock &trueTarget() const noexcept
      {
        return TrueTarget;
      }

      const BasicBlock &falseTarget() const noexcept
      {
        return FalseTarget;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::ConditionalBranchInstruction;
      }

    private:
      ConditionalBranchInstruction(const Type &VoidType, const Value &Condition, const BasicBlock &TrueTarget, const BasicBlock &FalseTarget) noexcept
          : Value(Condition.context(), ValueKind::ConditionalBranchInstruction, VoidType),
            Condition(Condition),
            TrueTarget(TrueTarget),
            FalseTarget(FalseTarget)
      {
      }

      const Value &Condition;
      const BasicBlock &TrueTarget;
      const BasicBlock &FalseTarget;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
