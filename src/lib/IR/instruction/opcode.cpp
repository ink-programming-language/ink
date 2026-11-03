#include "ink/ir/instruction/opcode.h"

namespace ink::ir
{
  std::string_view opcodeName(Opcode Code) noexcept
  {
    switch (Code)
    {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, AddressCount, ImmediateCount) \
  case Opcode::Kind:                                                                \
    return Spelling;
#include "ink/ir/instruction/opcode.def"
#undef OPCODE
    }
    return "InvalidOpcode";
  }

  std::uint32_t opcodeBitWidth(Opcode Code) noexcept
  {
    switch (Code)
    {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, AddressCount, ImmediateCount) \
  case Opcode::Kind:                                                                \
    return Width;
#include "ink/ir/instruction/opcode.def"
#undef OPCODE
    }
    return 0;
  }

  std::optional<std::uint32_t> opcodeOperandCount(Opcode Code) noexcept
  {
    switch (Code)
    {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, AddressCount, ImmediateCount) \
  case Opcode::Kind:                                                                \
    return AddressCount + ImmediateCount;
#include "ink/ir/instruction/opcode.def"
#undef OPCODE
    }
    return std::nullopt;
  }

  std::optional<std::uint32_t> opcodeEncodedSize(Opcode Code) noexcept
  {
    switch (Code)
    {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, AddressCount, ImmediateCount) \
  case Opcode::Kind:                                                                \
    return OpcodeEncodingBytes + AddressOperandEncodingBytes * AddressCount + ImmediateOperandEncodingBytes * ImmediateCount;
#include "ink/ir/instruction/opcode.def"
#undef OPCODE
    }
    return std::nullopt;
  }
} // namespace ink::ir
