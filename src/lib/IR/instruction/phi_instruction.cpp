#include "ink/ir/instruction/phi_instruction.h"

#include "ink/core/support/core_define.h"

namespace ink::ir
{
  void PhiInstruction::addIncoming(ValueRef IncomingValue, BasicBlock &Predecessor)
  {
    IncomingValues.push_back({&IncomingValue.get(), &Predecessor});
  }

  const Value &PhiInstruction::incomingValue(std::size_t Index) const noexcept
  {
    checkMsgf(Index < IncomingValues.size(), "Phi incoming index is out of range");
    return *IncomingValues[Index].IncomingValue;
  }

  BasicBlock &PhiInstruction::incomingBlockAt(std::size_t Index) const noexcept
  {
    checkMsgf(Index < IncomingValues.size(), "Phi incoming index is out of range");
    return *IncomingValues[Index].Predecessor;
  }
} // namespace ink::ir
