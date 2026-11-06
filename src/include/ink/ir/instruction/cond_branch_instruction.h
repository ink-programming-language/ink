#pragma once

#include "ink/ir/instruction/instruction.h"

namespace ink::ir
{
  class BasicBlock;

  // Borrows its void result type, condition, and target blocks; all must outlive this instruction.
  // The caller must provide VoidType, a boolean condition, and valid targets; construction does not validate operands.
  class CondBranchInstruction final : public Instruction
  {
    public:
      static constexpr InstructionKind Kind = InstructionKind::CondBranch;

      constexpr CondBranchInstruction(ValueRef ResultType, ValueRef Condition, BasicBlock &TrueTarget, BasicBlock &FalseTarget) noexcept
          : Instruction(Kind, ResultType),
            Condition(Condition.get()),
            TrueTarget(TrueTarget),
            FalseTarget(FalseTarget)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr const Value &getCondition() const noexcept
      {
        return Condition;
      }

      constexpr auto &getTrueTarget(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.TrueTarget);
      }

      constexpr auto &getFalseTarget(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.FalseTarget);
      }

    private:
      const Value &Condition;
      BasicBlock &TrueTarget;
      BasicBlock &FalseTarget;
  };
} // namespace ink::ir
