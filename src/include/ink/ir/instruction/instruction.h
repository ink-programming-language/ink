#pragma once

#include "ink/ir/instruction/address_operand.h"
#include "ink/ir/instruction/opcode.h"

#include <optional>

namespace ink::ir
{
  class Instruction
  {
    public:
      constexpr Opcode getOpcode() const noexcept
      {
        return Operation;
      }

      std::string_view getName() const noexcept
      {
        return opcodeName(Operation);
      }

      std::uint32_t bitWidth() const noexcept
      {
        return opcodeBitWidth(Operation);
      }

      constexpr const std::optional<AddressOperand> &getResult() const noexcept
      {
        return Result;
      }

      Instruction(const Instruction &) = delete;
      Instruction &operator=(const Instruction &) = delete;

    protected:
      constexpr Instruction(Opcode Operation, std::optional<AddressOperand> Result) noexcept
          : Operation(Operation),
            Result(Result)
      {
      }

      ~Instruction() = default;

    private:
      Opcode Operation;
      std::optional<AddressOperand> Result;
  };
} // namespace ink::ir
