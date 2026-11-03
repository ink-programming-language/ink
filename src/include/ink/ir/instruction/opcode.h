#pragma once

#include <cstdint>
#include <string_view>

namespace ink::ir
{
  enum class Opcode : std::uint8_t
  {
#define OPCODE(Kind, Spelling, Form, Width) Kind,
#include "ink/ir/instruction/opcode.def"
#undef OPCODE
  };

  std::string_view opcodeName(Opcode Code) noexcept;

  // Alloca and unrecognized opcodes have no fixed bit width and return zero.
  std::uint32_t opcodeBitWidth(Opcode Code) noexcept;
} // namespace ink::ir
