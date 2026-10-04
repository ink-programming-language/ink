#ifndef INK_EXECUTION_BYTECODE_INSTRUCTION_H
#define INK_EXECUTION_BYTECODE_INSTRUCTION_H

#include <array>
#include <cstdint>
#include <limits>
#include <string_view>

namespace ink::execution
{
  using SlotId = std::uint32_t;
  inline constexpr SlotId InvalidSlot = std::numeric_limits<SlotId>::max();

  enum class ExecutionPredicate
  {
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
  };

  enum class BytecodeOperandKind
  {
    None,
    WriteSlot,
    ReadSlot,
    Target,
    CallSite,
    Layout,
    Predicate,
    Local,
    DataOffset,
    DataLength,
    Status,
    FieldIndex,
  };

  enum class BytecodeOpcode
  {
#define INK_INSTRUCTION(Name, Operand0Kind, Operand1Kind, Operand2Kind, Operand3Kind) Name,
#include "ink/execution/bytecode/instruction.def"
#undef INK_INSTRUCTION
    Count,
  };

  struct BytecodeInstruction
  {
      BytecodeOpcode Code = BytecodeOpcode::Failure;
      // Each entry is encoded according to the opcode's instruction.def row.
      std::array<std::uint32_t, 4> Operands{};
  };

  struct BytecodeInstructionMetadata
  {
      std::string_view Name;
      std::array<BytecodeOperandKind, 4> Operands;
  };

  const BytecodeInstructionMetadata *bytecodeInstructionMetadata(BytecodeOpcode Code) noexcept;
} // namespace ink::execution

#endif
