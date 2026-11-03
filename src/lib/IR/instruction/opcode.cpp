#include "ink/ir/instruction/opcode.h"

namespace ink::ir
{
  std::string_view opcodeName(Opcode Code) noexcept
  {
    switch (Code)
    {
#define OPCODE(Kind, Spelling, Form, Width) \
  case Opcode::Kind:                        \
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
#define OPCODE(Kind, Spelling, Form, Width) \
  case Opcode::Kind:                        \
    return Width;
#include "ink/ir/instruction/opcode.def"
#undef OPCODE
    }
    return 0;
  }
} // namespace ink::ir
