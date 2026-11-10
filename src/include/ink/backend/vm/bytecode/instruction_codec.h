#pragma once

#include "ink/backend/vm/bytecode/byte_order.h"
#include "ink/backend/vm/instruction/machine_instruction.h"
#include "ink/backend/vm/instruction/register.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <vector>

namespace ink::vm
{
  enum class EncodedAddressBase : std::uint8_t
  {
    BP = 0,
    SP = 1,
    PC = 2,
    Native = 0xff,
  };

  enum class BytecodeOperandKind : std::uint8_t
  {
    Register,
    Address,
    Immediate,
  };

  // Bits holds an immediate, a signed register-relative displacement's bit pattern, or a native address.
  struct BytecodeOperand
  {
      BytecodeOperandKind Kind = BytecodeOperandKind::Immediate;
      RegisterId Register = RegisterId::BP;
      EncodedAddressBase AddressBase = EncodedAddressBase::BP;
      std::uint64_t Bits = 0;

      bool operator==(const BytecodeOperand &) const = default;
  };

  // A resolved wire instruction: no logical names, basic-block pointers, or ownership links.
  struct BytecodeInstruction
  {
      Opcode Operation = Opcode::Ret;
      std::array<BytecodeOperand, 3> Operands{};
      std::uint8_t OperandCount = 0;
  };

  struct DecodedInstruction
  {
      BytecodeInstruction Instruction;
      std::size_t ByteSize = 0;
  };

  enum class BytecodeError : std::uint8_t
  {
    InvalidByteOrder,
    UnknownOpcode,
    InvalidOperandCount,
    InvalidOperandKind,
    InvalidRegister,
    InvalidAddressBase,
    InvalidBranchBase,
    ImmediateOutOfRange,
    TruncatedInstruction,
  };

  class InstructionCodec final
  {
    public:
      // Opcode metadata determines the complete size, including the two-byte opcode.
      static std::optional<std::size_t> byteSize(Opcode Operation) noexcept;
      // Ordinary narrow immediates encode their low bits; native import indices must fit u32.
      static std::expected<std::vector<std::uint8_t>, BytecodeError> encode(const BytecodeInstruction &Instruction, ByteOrder Order = ByteOrder::Little);
      // Requires a valid opcode, operand layout and byte order; emits fields without validating them.
      static std::vector<std::uint8_t> encodeUnchecked(const BytecodeInstruction &Instruction, ByteOrder Order = ByteOrder::Little);
      // Reads only the first instruction; ByteSize lets callers advance within a code span.
      static std::expected<DecodedInstruction, BytecodeError> decode(std::span<const std::uint8_t> Bytes, ByteOrder Order = ByteOrder::Little) noexcept;
  };
} // namespace ink::vm
