#ifndef INK_IR_BASIC_BLOCK_H
#define INK_IR_BASIC_BLOCK_H

#include "ink/ir/instruction.h"

#include <memory>
#include <string>
#include <vector>

namespace ink::ir
{
  struct BasicBlock
  {
    std::string Name;
    std::vector<std::unique_ptr<Instruction>> Instructions;
  };
} // namespace ink::ir

#endif
