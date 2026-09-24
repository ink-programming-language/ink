#ifndef INK_SEMANTIC_MODEL_FLOAT_BITS_H
#define INK_SEMANTIC_MODEL_FLOAT_BITS_H

#include <cstdint>

namespace ink::semantic
{
  // Exact IEEE binary16/32/64 encoding, independent of host floating-point types.
  // This is a storage representation; arithmetic, conversion and rounding are separate.
  class FloatBits final
  {
    public:
      constexpr FloatBits(std::uint32_t BitWidth, std::uint64_t Bits) noexcept
          : BitWidth(BitWidth),
            Bits(Bits)
      {
      }

      constexpr std::uint32_t bitWidth() const noexcept
      {
        return BitWidth;
      }

      constexpr std::uint64_t bits() const noexcept
      {
        return Bits;
      }

      // Reject unsupported formats and excess high bits instead of silently truncating.
      constexpr bool valid() const noexcept
      {
        switch (BitWidth)
        {
        case 16:
        case 32:
          return (Bits >> BitWidth) == 0;
        case 64:
          return true;
        default:
          return false;
        }
      }

      constexpr bool operator==(const FloatBits &) const noexcept = default;

    private:
      std::uint32_t BitWidth;
      std::uint64_t Bits;
  };
} // namespace ink::semantic

#endif
