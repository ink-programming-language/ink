#pragma once

#include "ink/vm/instruction/operand.h"

#include <cstddef>
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

  template <Opcode OperationValue>
  class InstructionImpl;

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
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount) \
  case Opcode::Kind:                                                                               \
    return Spelling;
#include "ink/vm/instruction/opcode.def"
#undef OPCODE
        }
        return "InvalidOpcode";
      }

      constexpr std::uint32_t bitWidth() const noexcept
      {
        switch (Operation)
        {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount) \
  case Opcode::Kind:                                                                               \
    return Width;
#include "ink/vm/instruction/opcode.def"
#undef OPCODE
        }
        return 0;
      }

      constexpr std::size_t operandCount() const noexcept
      {
        switch (Operation)
        {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount) \
  case Opcode::Kind:                                                                               \
    return RegisterCount + AddressCount + ImmediateCount;
#include "ink/vm/instruction/opcode.def"
#undef OPCODE
        }
        return 0;
      }

      // Borrows an operand in semantic order; an index outside operandCount() returns nullptr.
      const Operand *getOperand(std::size_t Index) const noexcept;

      Instruction(const Instruction &) = delete;
      Instruction &operator=(const Instruction &) = delete;

    protected:
      // Instructions are destroyed through their concrete class, never through Instruction.
      ~Instruction() = default;

    private:
      template <Opcode OperationValue>
      friend class InstructionImpl;

      // Only the final opcode-specific implementation may establish an opcode and its matching layout.
      constexpr explicit Instruction(Opcode Operation) noexcept
          : Operation(Operation)
      {
      }

      Opcode Operation;
  };
} // namespace ink::vm
