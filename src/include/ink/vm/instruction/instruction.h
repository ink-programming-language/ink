#pragma once

#include <cstdint>
#include <string_view>

namespace ink::vm
{
  enum class Opcode : std::uint16_t
  {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount) Kind = Encoding,
#include "ink/vm/instruction/opcode.def"
#undef OPCODE
  };

  class Instruction
  {
    public:
      constexpr Opcode getOpcode() const noexcept
      {
        return Operation;
      }

      constexpr std::string_view getName() const noexcept
      {
        switch (Operation)
        {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount) case Opcode::Kind: return Spelling;
#include "ink/vm/instruction/opcode.def"
#undef OPCODE
        }
        return "InvalidOpcode";
      }

      constexpr std::uint32_t bitWidth() const noexcept
      {
        switch (Operation)
        {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount) case Opcode::Kind: return Width;
#include "ink/vm/instruction/opcode.def"
#undef OPCODE
        }
        return 0;
      }

      Instruction(const Instruction &) = delete;
      Instruction &operator=(const Instruction &) = delete;

    protected:
      constexpr explicit Instruction(Opcode Operation) noexcept
          : Operation(Operation)
      {
      }

      ~Instruction() = default;

    private:
      Opcode Operation;
  };
} // namespace ink::vm
