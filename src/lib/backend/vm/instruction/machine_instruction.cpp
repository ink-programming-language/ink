#include "ink/backend/vm/instruction/machine_instruction_visitor.h"

#include <cstddef>

namespace ink::vm
{
  const Operand *MachineInstruction::getOperand(std::size_t Index) const noexcept
  {
    const auto Lookup = [Index](const auto &Instruction) noexcept -> const Operand *
    {
      return Instruction.getOperand(Index);
    };
    return visitInstruction(*this, Lookup);
  }
} // namespace ink::vm
