#pragma once

#include "ink/ir/type/float_type.h"
#include "ink/ir/value/value.h"

#include <bit>
#include <cstddef>
#include <cstdint>

namespace ink::ir
{
  // Identity uses the IEEE bit pattern, preserving signed zero and NaN payloads.
  class FloatConstant final : public Value
  {
    public:
      struct Key
      {
          std::uint32_t BitWidth;
          std::uint64_t Bits;

          std::size_t hash() const noexcept;
          constexpr bool operator==(const Key &) const noexcept = default;
      };

      std::size_t hash() const noexcept;
      bool operator==(const FloatConstant &Other) const noexcept;
      bool operator==(const Key &Other) const noexcept;

      constexpr FloatConstant(const FloatType &ValueType, std::uint64_t Bits) noexcept
          : Value(ValueType),
            Bits(Bits)
      {
      }

      ~FloatConstant() = default;

      constexpr std::uint64_t bits() const noexcept
      {
        return Bits;
      }

      // Float32 is widened to double; use bits() when the exact NaN representation matters.
      double value() const noexcept
      {
        if (static_cast<const FloatType &>(getType()).bitWidth() == 32)
        {
          return static_cast<double>(std::bit_cast<float>(static_cast<std::uint32_t>(Bits)));
        }
        return std::bit_cast<double>(Bits);
      }

    private:
      std::uint64_t Bits;
  };
} // namespace ink::ir
