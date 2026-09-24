#ifndef INK_SEMANTIC_MODEL_INTEGER_BITS_H
#define INK_SEMANTIC_MODEL_INTEGER_BITS_H

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ink::semantic
{
  // Owned integer representation, with the least significant 64-bit word first.
  // Signedness belongs to IntegerType; signed values use two's-complement bits.
  // Construction preserves input bits. Check valid() before consuming a payload.
  class IntegerBits final
  {
    public:
      // Zero-extend LowWord for widths above 64; never truncate overflowing bits.
      explicit IntegerBits(std::uint32_t BitWidth, std::uint64_t LowWord = 0)
          : BitWidth(BitWidth),
            Words(wordCount(BitWidth), 0)
      {
        if (!Words.empty())
        {
          Words.front() = LowWord;
        }
      }

      // The complete word count and unused high bits are validated, not normalized.
      IntegerBits(std::uint32_t BitWidth, std::span<const std::uint64_t> Words)
          : BitWidth(BitWidth),
            Words(Words.begin(), Words.end())
      {
      }

      std::uint32_t bitWidth() const noexcept
      {
        return BitWidth;
      }

      std::span<const std::uint64_t> words() const noexcept
      {
        return Words;
      }

      bool valid() const noexcept
      {
        if (BitWidth == 0 || Words.size() != wordCount(BitWidth))
        {
          return false;
        }
        const unsigned HighBits = BitWidth % 64;
        return HighBits == 0 || (Words.back() >> HighBits) == 0;
      }

      bool operator==(const IntegerBits &) const noexcept = default;

    private:
      static std::size_t wordCount(std::uint32_t Width) noexcept
      {
        return static_cast<std::size_t>(Width / 64) + (Width % 64 != 0);
      }

      std::uint32_t BitWidth;
      std::vector<std::uint64_t> Words;
  };
} // namespace ink::semantic

#endif
