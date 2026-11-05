#pragma once

#include <cstdint>
#include <string_view>

namespace ink::ir
{
  enum class InstructionKind : std::uint8_t
  {
#define INSTRUCTION(Kind, Number, Class, Spelling) Kind = Number,
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION
  };

  std::string_view instructionKindName(InstructionKind Kind) noexcept;
} // namespace ink::ir
