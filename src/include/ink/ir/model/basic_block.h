#ifndef INK_IR_BASIC_BLOCK_H
#define INK_IR_BASIC_BLOCK_H

#include "ink/ir/instruction/instruction.h"
#include "ink/ir/model/name.h"

#include <memory>
#include <vector>

namespace ink::ir
{
  struct BasicBlock
  {
      ink::ir::Name Name;
      std::vector<std::unique_ptr<Instruction>> Instructions;
  };
} // namespace ink::ir

#endif
