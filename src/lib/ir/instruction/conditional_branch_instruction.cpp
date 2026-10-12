#include "ink/ir/instruction/conditional_branch_instruction.h"

#include "ink/ir/type/builtin_type/void_type.h"

namespace ink::ir
{
  ConditionalBranchInstruction::ConditionalBranchInstruction(const VoidType &ResultType, const Value &Condition, BasicBlock &TrueTarget, BasicBlock &FalseTarget) noexcept
      : Instruction(ValueKind::ConditionalBranchInstruction, &ResultType),
        Condition(&Condition),
        TrueTarget(&TrueTarget),
        FalseTarget(&FalseTarget)
  {
  }

  const Value *ConditionalBranchInstruction::condition() const noexcept
  {
    return Condition;
  }

  const BasicBlock *ConditionalBranchInstruction::trueTarget() const noexcept
  {
    return TrueTarget;
  }

  const BasicBlock *ConditionalBranchInstruction::falseTarget() const noexcept
  {
    return FalseTarget;
  }

  std::size_t ConditionalBranchInstruction::successorCount() const noexcept
  {
    return 2;
  }

  const BasicBlock *ConditionalBranchInstruction::successor(std::size_t Index) const noexcept
  {
    if (Index == 0)
    {
      return TrueTarget;
    }
    return Index == 1 ? FalseTarget : nullptr;
  }

  bool ConditionalBranchInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::ConditionalBranchInstruction;
  }
} // namespace ink::ir
