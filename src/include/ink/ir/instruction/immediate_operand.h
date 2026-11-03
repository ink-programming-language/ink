#pragma once

#include <bit>
#include <cstdint>

namespace ink::ir
{
  // The instruction determines the interpretation and storage width of the raw bits.
  class ImmediateOperand
  {
    public:
      constexpr explicit ImmediateOperand(std::uint64_t Bits) noexcept
          : Bits(Bits)
      {
      }

      static constexpr ImmediateOperand fromSigned(std::int64_t Value) noexcept
      {
        return ImmediateOperand(std::bit_cast<std::uint64_t>(Value));
      }

      static constexpr ImmediateOperand fromFloat(float Value) noexcept
      {
        return ImmediateOperand(std::bit_cast<std::uint32_t>(Value));
      }

      static constexpr ImmediateOperand fromDouble(double Value) noexcept
      {
        return ImmediateOperand(std::bit_cast<std::uint64_t>(Value));
      }

      constexpr std::uint64_t bits() const noexcept
      {
        return Bits;
      }

    private:
      std::uint64_t Bits;
  };
} // namespace ink::ir
