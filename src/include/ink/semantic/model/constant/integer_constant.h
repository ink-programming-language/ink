#ifndef INK_SEMANTIC_MODEL_CONSTANT_INTEGER_CONSTANT_H
#define INK_SEMANTIC_MODEL_CONSTANT_INTEGER_CONSTANT_H

#include "ink/semantic/model/constant/constant.h"
#include "ink/semantic/model/type/integer_type.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>
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

  class IntegerConstant final : public Constant
  {
    public:
      // Bits have exactly the declared integer width; signedness is in the type.
      const IntegerBits &value() const noexcept
      {
        return Payload;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::IntegerConstant;
      }

    private:
      IntegerConstant(const IntegerType &ValueType, IntegerBits Payload)
          : Constant(ValueKind::IntegerConstant, ValueType),
            Payload(std::move(Payload))
      {
      }

      IntegerBits Payload;

      friend class ConstantPool;
  };
} // namespace ink::semantic

#endif
