#pragma once

#include "ink/vm/instruction/operand.h"

#include <cstdint>

namespace ink::vm
{
  // Stores raw immediate bits; the instruction determines their payload width and interpretation.
  class ImmediateOperand final : public Operand
  {
    public:
      constexpr explicit ImmediateOperand(std::uint64_t Bits) noexcept
          : Operand(OperandKind::Immediate),
            Bits(Bits)
      {
      }

      constexpr std::uint64_t bits() const noexcept
      {
        return Bits;
      }

    private:
      std::uint64_t Bits;
  };
} // namespace ink::vm
