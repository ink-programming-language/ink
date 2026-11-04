#pragma once

#include "ink/ir/type/integer_type.h"
#include "ink/ir/value/value.h"

#include <bit>
#include <cstddef>
#include <cstdint>

namespace ink::ir
{
  class IntegerConstant final : public Value
  {
    public:
      struct Key
      {
          std::uint32_t BitWidth;
          bool Signed;
          std::uint64_t Bits;

          std::size_t hash() const noexcept;
          constexpr bool operator==(const Key &) const noexcept = default;
      };

      std::size_t hash() const noexcept;
      bool operator==(const IntegerConstant &Other) const noexcept;
      bool operator==(const Key &Other) const noexcept;

      constexpr IntegerConstant(const IntegerType &ValueType, std::uint64_t Bits) noexcept
          : Value(ValueType),
            Bits(Bits)
      {
      }

      ~IntegerConstant() = default;

      constexpr std::uint64_t bits() const noexcept
      {
        return Bits;
      }

      constexpr std::uint64_t unsignedValue() const noexcept
      {
        return Bits;
      }

      // Interprets the stored width as two's complement, regardless of type signedness.
      constexpr std::int64_t signedValue() const noexcept
      {
        const auto Shift = 64U - static_cast<const IntegerType &>(getType()).bitWidth();
        return std::bit_cast<std::int64_t>(Bits << Shift) >> Shift;
      }

    private:
      std::uint64_t Bits;
  };
} // namespace ink::ir
