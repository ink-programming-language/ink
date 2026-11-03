#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace ink::ir
{
  enum class Opcode : std::uint16_t
  {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, AddressCount, ImmediateCount) Kind = Encoding,
#include "ink/ir/instruction/opcode.def"
#undef OPCODE
  };

  inline constexpr std::uint32_t OpcodeEncodingBytes = 2;
  inline constexpr std::uint32_t AddressOperandEncodingBytes = 12;
  inline constexpr std::uint32_t ImmediateOperandEncodingBytes = 8;

  std::string_view opcodeName(Opcode Code) noexcept;

  // Alloca and unrecognized opcodes have no fixed bit width and return zero.
  std::uint32_t opcodeBitWidth(Opcode Code) noexcept;

  // Counts all encoded operands, including a result or store destination where present.
  std::optional<std::uint32_t> opcodeOperandCount(Opcode Code) noexcept;

  // Encoded size is 2 + 12 * address count + 8 * immediate count, without padding.
  // These encoding queries return nullopt for unrecognized opcodes.
  std::optional<std::uint32_t> opcodeEncodedSize(Opcode Code) noexcept;
} // namespace ink::ir
