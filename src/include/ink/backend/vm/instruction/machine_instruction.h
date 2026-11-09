#pragma once

#include "ink/backend/vm/instruction/operand.h"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>

namespace ink::vm
{
  class MachineBasicBlock;
  class MachineInstructionBuilder;

  enum class Opcode : std::uint16_t
  {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount) Kind = Encoding,
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
  };

  template <Opcode OperationValue>
  class MachineInstructionImpl;

  class MachineInstruction
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
#include "ink/backend/vm/instruction/opcode.def"
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
#include "ink/backend/vm/instruction/opcode.def"
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
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
        }
        return 0;
      }

      // Borrows an operand in semantic order; an index outside operandCount() returns nullptr.
      const Operand *getOperand(std::size_t Index) const noexcept;

      constexpr auto *parent(this auto &&Self) noexcept
      {
        return Self.Parent ? &std::forward_like<decltype(Self) &>(*Self.Parent) : nullptr;
      }

      MachineInstruction(const MachineInstruction &) = delete;
      MachineInstruction &operator=(const MachineInstruction &) = delete;

    protected:
      // Instructions are destroyed through their concrete class, never through MachineInstruction.
      ~MachineInstruction() = default;

    private:
      friend class MachineInstructionBuilder;

      template <Opcode OperationValue>
      friend class MachineInstructionImpl;

      // Only the final opcode-specific implementation may establish an opcode and its matching layout.
      constexpr explicit MachineInstruction(Opcode Operation) noexcept
          : Operation(Operation)
      {
      }

      Opcode Operation;
      MachineBasicBlock *Parent = nullptr;
  };
} // namespace ink::vm
