#include "ink/execution/bytecode/instruction.h"

#include <cstddef>

namespace ink::execution
{
  namespace
  {
    constexpr BytecodeInstructionMetadata Metadata[] = {
#define INK_INSTRUCTION(Name, Operand0Kind, Operand1Kind, Operand2Kind, Operand3Kind) {#Name, {BytecodeOperandKind::Operand0Kind, BytecodeOperandKind::Operand1Kind, BytecodeOperandKind::Operand2Kind, BytecodeOperandKind::Operand3Kind}},
#include "ink/execution/bytecode/instruction.def"
#undef INK_INSTRUCTION
    };

    static_assert(std::size(Metadata) == static_cast<std::size_t>(BytecodeOpcode::Count));
  } // namespace

  const BytecodeInstructionMetadata *bytecodeInstructionMetadata(BytecodeOpcode Code) noexcept
  {
    const auto Index = static_cast<std::size_t>(Code);
    return Index < std::size(Metadata) ? &Metadata[Index] : nullptr;
  }
} // namespace ink::execution
