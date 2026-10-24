#include "ink/core/numeric/integer_bits.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <utility>

namespace ink::core
{
  namespace
  {
    constexpr std::uint64_t AllBits = std::numeric_limits<std::uint64_t>::max();

    std::size_t wordCount(std::uint32_t Width) noexcept
    {
      return (static_cast<std::size_t>(Width) + 63U) / 64U;
    }

    std::uint64_t lastWordMask(std::uint32_t Width) noexcept
    {
      const auto Remainder = Width % 64U;
      return Remainder == 0 ? AllBits : (std::uint64_t{1} << Remainder) - 1;
    }
  } // namespace

  IntegerBits::IntegerBits(std::uint32_t Width, std::vector<std::uint64_t> Words)
      : BitWidth(Width),
        Words(std::move(Words))
  {
  }

  bool IntegerBits::isValidWidth(std::uint32_t Width) noexcept
  {
    return Width != 0 && Width <= MaxBitWidth;
  }

  std::optional<IntegerBits> IntegerBits::fromWords(std::uint32_t Width, std::span<const std::uint64_t> Words)
  {
    if (!isValidWidth(Width))
    {
      return std::nullopt;
    }
    const auto Count = wordCount(Width);
    for (std::size_t Index = Count; Index < Words.size(); ++Index)
    {
      if (Words[Index] != 0)
      {
        return std::nullopt;
      }
    }
    if (Words.size() >= Count && (Words[Count - 1] & ~lastWordMask(Width)) != 0)
    {
      return std::nullopt;
    }
    std::vector<std::uint64_t> Storage(Count, 0);
    std::copy_n(Words.begin(), std::min(Count, Words.size()), Storage.begin());
    return IntegerBits(Width, std::move(Storage));
  }

  std::optional<IntegerBits> IntegerBits::fromUnsigned(std::uint32_t Width, std::uint64_t Value)
  {
    if (!isValidWidth(Width) || (Width < 64 && (Value >> Width) != 0))
    {
      return std::nullopt;
    }
    std::vector<std::uint64_t> Storage(wordCount(Width), 0);
    Storage[0] = Value;
    return IntegerBits(Width, std::move(Storage));
  }

  std::optional<IntegerBits> IntegerBits::fromSigned(std::uint32_t Width, std::int64_t Value)
  {
    if (!isValidWidth(Width))
    {
      return std::nullopt;
    }
    if (Width < 64)
    {
      const auto Minimum = -(std::int64_t{1} << (Width - 1));
      const auto Maximum = (std::int64_t{1} << (Width - 1)) - 1;
      if (Value < Minimum || Value > Maximum)
      {
        return std::nullopt;
      }
    }
    std::vector<std::uint64_t> Storage(wordCount(Width), Value < 0 ? AllBits : 0);
    Storage[0] = static_cast<std::uint64_t>(Value);
    Storage.back() &= lastWordMask(Width);
    return IntegerBits(Width, std::move(Storage));
  }

  std::uint32_t IntegerBits::bitWidth() const noexcept
  {
    return BitWidth;
  }

  std::span<const std::uint64_t> IntegerBits::words() const noexcept
  {
    return Words;
  }

  bool IntegerBits::isZero() const noexcept
  {
    return std::all_of(Words.begin(), Words.end(), [](std::uint64_t Word)
    {
      return Word == 0;
    });
  }

  bool IntegerBits::isNegative() const noexcept
  {
    return (Words.back() & (std::uint64_t{1} << ((BitWidth - 1) % 64U))) != 0;
  }

  bool IntegerBits::highBitsEqual(std::uint32_t FirstBit, bool Set) const noexcept
  {
    if (FirstBit >= BitWidth)
    {
      return true;
    }
    const std::size_t FirstWord = FirstBit / 64U;
    for (std::size_t Index = FirstWord; Index < Words.size(); ++Index)
    {
      std::uint64_t Mask = Index == FirstWord ? AllBits << (FirstBit % 64U) : AllBits;
      if (Index + 1 == Words.size())
      {
        Mask &= lastWordMask(BitWidth);
      }
      if ((Words[Index] & Mask) != (Set ? Mask : 0))
      {
        return false;
      }
    }
    return true;
  }

  bool IntegerBits::fitsUnsigned(std::uint32_t Width) const noexcept
  {
    return isValidWidth(Width) && highBitsEqual(Width, false);
  }

  bool IntegerBits::fitsSigned(std::uint32_t Width) const noexcept
  {
    return isValidWidth(Width) && (Width >= BitWidth || highBitsEqual(Width - 1, isNegative()));
  }

  std::optional<std::uint64_t> IntegerBits::tryGetUInt64() const noexcept
  {
    if (!fitsUnsigned(64))
    {
      return std::nullopt;
    }
    return Words[0];
  }

  std::optional<std::int64_t> IntegerBits::tryGetInt64() const noexcept
  {
    if (!fitsSigned(64))
    {
      return std::nullopt;
    }
    std::uint64_t Bits = Words[0];
    if (BitWidth < 64 && isNegative())
    {
      Bits |= AllBits << BitWidth;
    }
    if (Bits <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
    {
      return static_cast<std::int64_t>(Bits);
    }
    return -1 - static_cast<std::int64_t>(~Bits);
  }

  std::optional<IntegerBits> IntegerBits::zeroExtend(std::uint32_t Width) const
  {
    if (!isValidWidth(Width) || Width < BitWidth)
    {
      return std::nullopt;
    }
    std::vector<std::uint64_t> Storage(wordCount(Width), 0);
    std::copy(Words.begin(), Words.end(), Storage.begin());
    return IntegerBits(Width, std::move(Storage));
  }

  std::optional<IntegerBits> IntegerBits::signExtend(std::uint32_t Width) const
  {
    if (!isValidWidth(Width) || Width < BitWidth)
    {
      return std::nullopt;
    }
    const bool Negative = isNegative();
    std::vector<std::uint64_t> Storage(wordCount(Width), Negative ? AllBits : 0);
    std::copy(Words.begin(), Words.end(), Storage.begin());
    if (Negative)
    {
      Storage[Words.size() - 1] |= ~lastWordMask(BitWidth);
    }
    Storage.back() &= lastWordMask(Width);
    return IntegerBits(Width, std::move(Storage));
  }

  std::optional<IntegerBits> IntegerBits::truncate(std::uint32_t Width) const
  {
    if (!isValidWidth(Width) || Width > BitWidth)
    {
      return std::nullopt;
    }
    std::vector<std::uint64_t> Storage(Words.begin(), Words.begin() + wordCount(Width));
    Storage.back() &= lastWordMask(Width);
    return IntegerBits(Width, std::move(Storage));
  }

  IntegerBits IntegerBits::negated() const
  {
    std::vector<std::uint64_t> Storage(Words.size());
    std::uint64_t Carry = 1;
    for (std::size_t Index = 0; Index < Words.size(); ++Index)
    {
      Storage[Index] = ~Words[Index] + Carry;
      Carry = Carry != 0 && Storage[Index] == 0;
    }
    Storage.back() &= lastWordMask(BitWidth);
    return IntegerBits(BitWidth, std::move(Storage));
  }

  bool IntegerBits::operator==(const IntegerBits &Other) const noexcept
  {
    return BitWidth == Other.BitWidth && Words == Other.Words;
  }

  std::size_t IntegerBits::hash() const noexcept
  {
    std::size_t Hash = std::hash<std::uint32_t>{}(BitWidth);
    for (const auto Word : Words)
    {
      Hash ^= std::hash<std::uint64_t>{}(Word) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    }
    return Hash;
  }
} // namespace ink::core
