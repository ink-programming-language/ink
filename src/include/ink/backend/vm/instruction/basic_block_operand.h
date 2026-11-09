#pragma once

#include "ink/backend/vm/instruction/operand.h"

namespace ink::vm
{
  class MachineBasicBlock;

  // Borrows a logical target block; object emission resolves its final PC-relative displacement.
  // The target's owning function must outlive every instruction that references it.
  class BasicBlockOperand final : public Operand
  {
    public:
      constexpr explicit BasicBlockOperand(const MachineBasicBlock *Block) noexcept
          : Operand(OperandKind::BasicBlock),
            Block(Block)
      {
      }

      constexpr const MachineBasicBlock *getBlock() const noexcept
      {
        return Block;
      }

    private:
      const MachineBasicBlock *Block;
  };
} // namespace ink::vm
