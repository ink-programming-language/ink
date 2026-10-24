#ifndef INK_CORE_NUMERIC_INTEGER_BITS_H
#define INK_CORE_NUMERIC_INTEGER_BITS_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace ink::core
{
  // A fixed-width bit pattern, independent of target byte order and language signedness.
  // Words are ordered by increasing bit significance; unused high bits are always zero.
  class IntegerBits final
  {
    public:
      // Bound compiler-side storage while allowing every bit width within the limit.
      static constexpr std::uint32_t MaxBitWidth = 1U << 23;

      // Copying rvalues also preserves the source's valid fixed-width representation.
      IntegerBits(const IntegerBits &) = default;
      IntegerBits &operator=(const IntegerBits &) = default;

      static bool isValidWidth(std::uint32_t Width) noexcept;
      // Missing words are zero; additional zero words are accepted. Nonzero excess bits fail.
      [[nodiscard]] static std::optional<IntegerBits> fromWords(std::uint32_t Width, std::span<const std::uint64_t> Words);
      [[nodiscard]] static std::optional<IntegerBits> fromUnsigned(std::uint32_t Width, std::uint64_t Value);
      [[nodiscard]] static std::optional<IntegerBits> fromSigned(std::uint32_t Width, std::int64_t Value);

      std::uint32_t bitWidth() const noexcept;
      std::span<const std::uint64_t> words() const noexcept;
      bool isZero() const noexcept;
      // Interprets the highest effective bit as a two's-complement sign bit.
      bool isNegative() const noexcept;
      // Tests the full unsigned bit pattern or the signed two's-complement value, respectively.
      bool fitsUnsigned(std::uint32_t Width) const noexcept;
      bool fitsSigned(std::uint32_t Width) const noexcept;
      [[nodiscard]] std::optional<std::uint64_t> tryGetUInt64() const noexcept;
      [[nodiscard]] std::optional<std::int64_t> tryGetInt64() const noexcept;

      // Equal widths are allowed; an invalid width or wrong direction returns nullopt.
      [[nodiscard]] std::optional<IntegerBits> zeroExtend(std::uint32_t Width) const;
      [[nodiscard]] std::optional<IntegerBits> signExtend(std::uint32_t Width) const;
      [[nodiscard]] std::optional<IntegerBits> truncate(std::uint32_t Width) const;
      // Computes two's-complement negation modulo 2^bitWidth(), preserving the width.
      IntegerBits negated() const;

      bool operator==(const IntegerBits &Other) const noexcept;
      std::size_t hash() const noexcept;

    private:
      IntegerBits(std::uint32_t Width, std::vector<std::uint64_t> Words);
      bool highBitsEqual(std::uint32_t FirstBit, bool Set) const noexcept;

      std::uint32_t BitWidth;
      std::vector<std::uint64_t> Words;
  };
} // namespace ink::core

#endif
