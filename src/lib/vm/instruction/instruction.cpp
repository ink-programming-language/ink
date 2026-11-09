#include "ink/vm/instruction/instructions.h"

#include <cstddef>

namespace ink::vm
{
  const Operand *Instruction::getOperand(std::size_t Index) const noexcept
  {
    switch (Operation)
    {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount) \
  case Opcode::Kind:                                                                               \
    return static_cast<const InstructionImpl<Opcode::Kind> &>(*this).getOperand(Index);
#include "ink/vm/instruction/opcode.def"
#undef OPCODE
    }
    return nullptr;
  }
} // namespace ink::vm
