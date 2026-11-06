#include "ink/ir/function/basic_block.h"

#include "ink/core/support/core_define.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/cond_branch_instruction.h"

namespace ink::ir
{
  Instruction *BasicBlock::terminatorAt() const noexcept
  {
    return !Instructions.empty() && Instructions.back()->isTerminator() ? Instructions.back().get() : nullptr;
  }

  std::size_t BasicBlock::successorCount() const noexcept
  {
    const Instruction *Last = terminatorAt();
    if (BranchInstruction::classof(Last))
    {
      return 1;
    }
    return CondBranchInstruction::classof(Last) ? 2 : 0;
  }

  BasicBlock &BasicBlock::successorAt(std::size_t Index) const noexcept
  {
    checkMsgf(Index < successorCount(), "BasicBlock successor index is out of range");
    Instruction *Last = terminatorAt();
    if (BranchInstruction::classof(Last))
    {
      return static_cast<BranchInstruction &>(*Last).getTarget();
    }
    auto &Branch = static_cast<CondBranchInstruction &>(*Last);
    return Index == 0 ? Branch.getTrueTarget() : Branch.getFalseTarget();
  }

  std::vector<BasicBlock *> BasicBlock::predecessorBlocks() const
  {
    std::vector<BasicBlock *> Blocks;
    if (!Parent)
    {
      return Blocks;
    }
    for (BasicBlock &Candidate : Parent->basicBlocks())
    {
      for (std::size_t Index = 0; Index < Candidate.successorCount(); ++Index)
      {
        if (&Candidate.successorAt(Index) == this)
        {
          Blocks.push_back(&Candidate);
        }
      }
    }
    return Blocks;
  }
} // namespace ink::ir
