#include "ink/backend/vm/module/machine_instruction_builder.h"

#include "ink/core/support/core_define.h"

namespace ink::vm
{
  void MachineInstructionBuilder::invalidBranchTarget() noexcept
  {
    panic("VM branch target must belong to the insertion block's function");
  }

  void MachineInstructionBuilder::setInsertPoint(MachineBasicBlock &Block) noexcept
  {
    InsertBlock = &Block;
    InsertPosition = Block.Instructions.end();
  }

  bool MachineInstructionBuilder::setInsertPoint(MachineInstruction &Before) noexcept
  {
    MachineBasicBlock *Block = Before.parent();
    if (!Block)
    {
      return false;
    }
    for (auto Position = Block->Instructions.begin(); Position != Block->Instructions.end(); ++Position)
    {
      if (Position->get() == &Before)
      {
        InsertBlock = Block;
        InsertPosition = Position;
        return true;
      }
    }
    return false;
  }

  void MachineInstructionBuilder::clearInsertPoint() noexcept
  {
    InsertBlock = nullptr;
    InsertPosition = {};
  }

  MachineBasicBlock *MachineInstructionBuilder::getInsertBlock() const noexcept
  {
    return InsertBlock;
  }

  MachineInstruction *MachineInstructionBuilder::getInsertBefore() const noexcept
  {
    if (!InsertBlock || InsertPosition == InsertBlock->Instructions.end())
    {
      return nullptr;
    }
    return InsertPosition->get();
  }

  void MachineInstructionBuilder::checkInsertPoint() const noexcept
  {
    if (!InsertBlock)
    {
      panic("VM MachineInstructionBuilder requires an insertion point");
    }
  }
} // namespace ink::vm
