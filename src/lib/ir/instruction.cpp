#include "ink/ir/instruction.h"

namespace ink::ir
{
  Instruction::~Instruction() = default;

  const char *instructionKindName(InstructionKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator) \
  case InstructionKind::Name:                          \
    return #Name;
#include "ink/ir/ir.def"
    }
    return "Unknown";
  }

  const char *instructionMnemonic(InstructionKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator) \
  case InstructionKind::Name:                          \
    return Mnemonic;
#include "ink/ir/ir.def"
    }
    return "unknown";
  }

  bool isTerminator(InstructionKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator) \
  case InstructionKind::Name:                          \
    return Terminator;
#include "ink/ir/ir.def"
    }
    return false;
  }
} // namespace ink::ir
