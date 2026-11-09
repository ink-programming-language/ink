#include "ink/backend/vm/instruction/machine_instructions.h"

#include <cstddef>

namespace ink::vm
{
  const Operand *MachineInstruction::getOperand(std::size_t Index) const noexcept
  {
    switch (Operation)
    {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Condition) \
  case Opcode::Kind:                                                                                                    \
    return static_cast<const MachineInstructionImpl<Opcode::Kind> &>(*this).getOperand(Index);
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
    }
    return nullptr;
  }
} // namespace ink::vm
