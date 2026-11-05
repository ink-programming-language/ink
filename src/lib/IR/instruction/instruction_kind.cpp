#include "ink/ir/instruction/instruction_kind.h"

namespace ink::ir
{
  std::string_view instructionKindName(InstructionKind Kind) noexcept
  {
    switch (Kind)
    {
#define INSTRUCTION(Kind, Number, Class, Spelling) \
  case InstructionKind::Kind:                     \
    return Spelling;
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION
    }
    return "InvalidInstructionKind";
  }
} // namespace ink::ir
