#include "ink/core/numeric/integer_bits.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::core::test
{
  static_assert(!std::is_default_constructible_v<IntegerBits>);
  static_assert(!std::is_convertible_v<IntegerBits, std::uint64_t>);

  namespace
  {
    constexpr auto AllBits = std::numeric_limits<std::uint64_t>::max();
    constexpr auto SignedMinimum = std::numeric_limits<std::int64_t>::min();
    constexpr auto SignedMaximum = std::numeric_limits<std::int64_t>::max();
    constexpr std::uint32_t Widths[] = {
        1,
        7,
        63,
        64,
        65,
        127,
        128,
        129,
        256,
        257,
    };
  } // namespace

  // Word inputs preserve mathematical bit positions and canonicalize omitted or additional zero words.
  TEST(IntegerBitsTest, ConstructsCanonicalWordPatterns)
  {
    const std::uint64_t Input[] = {5, 1, 0};
    const auto Wide = IntegerBits::fromWords(128, Input);
    const auto Trimmed = IntegerBits::fromWords(128, std::span(Input).first(2));
    ASSERT_TRUE(Wide);
    ASSERT_TRUE(Trimmed);
    EXPECT_EQ(*Wide, *Trimmed);
    EXPECT_EQ(Wide->hash(), Trimmed->hash());
    EXPECT_EQ(Wide->bitWidth(), 128U);
    ASSERT_EQ(Wide->words().size(), 2U);
    EXPECT_EQ(Wide->words()[0], 5U);
    EXPECT_EQ(Wide->words()[1], 1U);
    EXPECT_FALSE(Wide->tryGetUInt64());

    const std::uint64_t Low[] = {5};
    const auto Padded = IntegerBits::fromWords(257, Low);
    const auto Numeric = IntegerBits::fromUnsigned(257, 5);
    ASSERT_TRUE(Padded);
    ASSERT_TRUE(Numeric);
    EXPECT_EQ(*Padded, *Numeric);
    EXPECT_EQ(Padded->tryGetUInt64(), 5U);
    const auto Zero = IntegerBits::fromWords(129, {});
    ASSERT_TRUE(Zero);
    EXPECT_TRUE(Zero->isZero());
    EXPECT_FALSE(Zero->isNegative());
  }

  // Raw construction rejects any nonzero bit outside the declared width, including complete extra words.
  TEST(IntegerBitsTest, RejectsNonzeroExcessBitsWithoutTruncation)
  {
    for (const auto Width : Widths)
    {
      SCOPED_TRACE(Width);
      std::vector<std::uint64_t> Words((Width + 63U) / 64U, 0);
      Words.back() = std::uint64_t{1} << ((Width - 1) % 64U);
      const auto HighestBit = IntegerBits::fromWords(Width, Words);
      ASSERT_TRUE(HighestBit);
      EXPECT_TRUE(HighestBit->isNegative());
      if (Width % 64U != 0)
      {
        Words.back() |= std::uint64_t{1} << (Width % 64U);
        EXPECT_FALSE(IntegerBits::fromWords(Width, Words));
      }
      else
      {
        Words.push_back(1);
        EXPECT_FALSE(IntegerBits::fromWords(Width, Words));
      }
    }
  }

  // All factories reject invalid widths before allocation, and the documented upper bound remains accepted.
  TEST(IntegerBitsTest, ValidatesWidthBounds)
  {
    for (const auto Width : {0U, IntegerBits::MaxBitWidth + 1, std::numeric_limits<std::uint32_t>::max()})
    {
      SCOPED_TRACE(Width);
      EXPECT_FALSE(IntegerBits::isValidWidth(Width));
      EXPECT_FALSE(IntegerBits::fromWords(Width, {}));
      EXPECT_FALSE(IntegerBits::fromUnsigned(Width, 0));
      EXPECT_FALSE(IntegerBits::fromSigned(Width, 0));
    }
    const auto MaximumWidth = IntegerBits::fromUnsigned(IntegerBits::MaxBitWidth, 1);
    ASSERT_TRUE(MaximumWidth);
    EXPECT_EQ(MaximumWidth->bitWidth(), IntegerBits::MaxBitWidth);
    EXPECT_EQ(MaximumWidth->tryGetUInt64(), 1U);
    EXPECT_FALSE(MaximumWidth->isNegative());
  }

  // Signed numeric construction accepts exact endpoints without first forcing them through a positive magnitude.
  TEST(IntegerBitsTest, ConstructsSignedNumericBoundaries)
  {
    struct Case
    {
        std::uint32_t Width;
        std::int64_t Minimum;
        std::int64_t Maximum;
    };
    constexpr Case Cases[] = {
        {1, -1, 0},
        {7, -64, 63},
        {63, -(std::int64_t{1} << 62), (std::int64_t{1} << 62) - 1},
        {64, SignedMinimum, SignedMaximum},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Width);
      const auto Minimum = IntegerBits::fromSigned(Entry.Width, Entry.Minimum);
      const auto Maximum = IntegerBits::fromSigned(Entry.Width, Entry.Maximum);
      ASSERT_TRUE(Minimum);
      ASSERT_TRUE(Maximum);
      EXPECT_EQ(Minimum->tryGetInt64(), Entry.Minimum);
      EXPECT_EQ(Maximum->tryGetInt64(), Entry.Maximum);
      EXPECT_TRUE(Minimum->isNegative());
      EXPECT_FALSE(Maximum->isNegative());
      if (Entry.Width < 64)
      {
        EXPECT_FALSE(IntegerBits::fromSigned(Entry.Width, Entry.Minimum - 1));
        EXPECT_FALSE(IntegerBits::fromSigned(Entry.Width, Entry.Maximum + 1));
      }
    }
  }

  // Unsigned numeric construction accepts a set sign bit but never discards overflow above the declared width.
  TEST(IntegerBitsTest, ConstructsUnsignedNumericBoundaries)
  {
    for (const auto Width : Widths)
    {
      SCOPED_TRACE(Width);
      const auto Maximum = Width < 64 ? (std::uint64_t{1} << Width) - 1 : AllBits;
      const auto Value = IntegerBits::fromUnsigned(Width, Maximum);
      ASSERT_TRUE(Value);
      EXPECT_EQ(Value->tryGetUInt64(), Maximum);
      EXPECT_EQ(Value->isNegative(), Width <= 64);
      if (Width < 64)
      {
        EXPECT_FALSE(IntegerBits::fromUnsigned(Width, Maximum + 1));
      }
    }
  }

  // Wide numeric values retain sign extension and reject host extraction whenever significant high bits would be lost.
  TEST(IntegerBitsTest, ExtractsHostIntegersOnlyWhenRepresentable)
  {
    for (const auto Width : {65U, 127U, 128U, 129U, 256U, 257U})
    {
      SCOPED_TRACE(Width);
      const auto Negative = IntegerBits::fromSigned(Width, -1);
      const auto Minimum = IntegerBits::fromSigned(Width, SignedMinimum);
      const auto Maximum = IntegerBits::fromUnsigned(Width, AllBits);
      ASSERT_TRUE(Negative);
      ASSERT_TRUE(Minimum);
      ASSERT_TRUE(Maximum);
      EXPECT_EQ(Negative->tryGetInt64(), -1);
      EXPECT_EQ(Minimum->tryGetInt64(), SignedMinimum);
      EXPECT_FALSE(Negative->tryGetUInt64());
      EXPECT_FALSE(Minimum->tryGetUInt64());
      EXPECT_FALSE(Maximum->tryGetInt64());
      EXPECT_EQ(Maximum->tryGetUInt64(), AllBits);
      EXPECT_TRUE(Negative->isNegative());
      EXPECT_FALSE(Maximum->isNegative());
    }
    const std::uint64_t TooNegativeWords[] = {0, 1};
    const auto TooNegative = IntegerBits::fromWords(65, TooNegativeWords);
    ASSERT_TRUE(TooNegative);
    EXPECT_FALSE(TooNegative->tryGetInt64());
    EXPECT_FALSE(TooNegative->tryGetUInt64());
  }

  // Signed and unsigned fit checks use their respective interpretations and include the prospective sign bit.
  TEST(IntegerBitsTest, ChecksSignedAndUnsignedTargetRanges)
  {
    const auto Negative = IntegerBits::fromSigned(129, -1);
    const auto Positive = IntegerBits::fromUnsigned(129, 255);
    const auto Minimum = IntegerBits::fromSigned(129, -128);
    ASSERT_TRUE(Negative);
    ASSERT_TRUE(Positive);
    ASSERT_TRUE(Minimum);
    EXPECT_TRUE(Negative->fitsSigned(1));
    EXPECT_FALSE(Negative->fitsUnsigned(128));
    EXPECT_TRUE(Negative->fitsUnsigned(129));
    EXPECT_TRUE(Positive->fitsUnsigned(8));
    EXPECT_FALSE(Positive->fitsSigned(8));
    EXPECT_TRUE(Positive->fitsSigned(9));
    EXPECT_TRUE(Minimum->fitsSigned(8));
    EXPECT_FALSE(Minimum->fitsSigned(7));
    EXPECT_FALSE(Positive->fitsSigned(0));
    EXPECT_FALSE(Positive->fitsUnsigned(0));
    EXPECT_FALSE(Positive->fitsSigned(IntegerBits::MaxBitWidth + 1));
    EXPECT_FALSE(Positive->fitsUnsigned(IntegerBits::MaxBitWidth + 1));
  }

  // Zero and sign extension differ for negative values and remain correct across partial and complete word boundaries.
  TEST(IntegerBitsTest, ExtendsAcrossWordBoundaries)
  {
    for (const auto Width : Widths)
    {
      SCOPED_TRACE(Width);
      const auto Negative = IntegerBits::fromSigned(Width, -1);
      ASSERT_TRUE(Negative);
      for (const auto DestinationWidth : Widths)
      {
        if (DestinationWidth < Width)
        {
          continue;
        }
        SCOPED_TRACE(DestinationWidth);
        const auto Signed = Negative->signExtend(DestinationWidth);
        const auto Unsigned = Negative->zeroExtend(DestinationWidth);
        const auto ExpectedSigned = IntegerBits::fromSigned(DestinationWidth, -1);
        ASSERT_TRUE(Signed);
        ASSERT_TRUE(Unsigned);
        ASSERT_TRUE(ExpectedSigned);
        EXPECT_EQ(*Signed, *ExpectedSigned);
        EXPECT_EQ(Signed->tryGetInt64(), -1);
        EXPECT_EQ(Unsigned->isNegative(), Width == DestinationWidth);
        const auto Restored = Unsigned->truncate(Width);
        ASSERT_TRUE(Restored);
        EXPECT_EQ(*Restored, *Negative);
      }
    }
  }

  // Truncation explicitly drops high bits and masks the new last word rather than changing the low-bit pattern.
  TEST(IntegerBitsTest, TruncatesToExactBitWidths)
  {
    const auto Wide = IntegerBits::fromSigned(257, -1);
    ASSERT_TRUE(Wide);
    for (const auto Width : Widths)
    {
      SCOPED_TRACE(Width);
      const auto Narrow = Wide->truncate(Width);
      const auto Expected = IntegerBits::fromSigned(Width, -1);
      ASSERT_TRUE(Narrow);
      ASSERT_TRUE(Expected);
      EXPECT_EQ(*Narrow, *Expected);
    }
    const auto Value = IntegerBits::fromUnsigned(128, 300);
    ASSERT_TRUE(Value);
    const auto Byte = Value->truncate(8);
    ASSERT_TRUE(Byte);
    EXPECT_EQ(Byte->tryGetUInt64(), 44U);
  }

  // Width-changing operations reject invalid destinations and direction mismatches while preserving their source.
  TEST(IntegerBitsTest, RejectsInvalidWidthChanges)
  {
    const auto Value = IntegerBits::fromUnsigned(65, 13);
    ASSERT_TRUE(Value);
    EXPECT_FALSE(Value->zeroExtend(64));
    EXPECT_FALSE(Value->signExtend(64));
    EXPECT_FALSE(Value->truncate(66));
    for (const auto Width : {0U, IntegerBits::MaxBitWidth + 1})
    {
      EXPECT_FALSE(Value->zeroExtend(Width));
      EXPECT_FALSE(Value->signExtend(Width));
      EXPECT_FALSE(Value->truncate(Width));
    }
    EXPECT_EQ(Value->tryGetUInt64(), 13U);
    EXPECT_EQ(Value->bitWidth(), 65U);
  }

  // Negation carries across words, preserves width, and wraps signed minimum and zero according to the bit-pattern contract.
  TEST(IntegerBitsTest, NegatesModuloDeclaredWidth)
  {
    for (const auto Width : Widths)
    {
      SCOPED_TRACE(Width);
      const auto Zero = IntegerBits::fromUnsigned(Width, 0);
      const auto One = IntegerBits::fromUnsigned(Width, 1);
      const auto NegativeOne = IntegerBits::fromSigned(Width, -1);
      ASSERT_TRUE(Zero);
      ASSERT_TRUE(One);
      ASSERT_TRUE(NegativeOne);
      EXPECT_EQ(Zero->negated(), *Zero);
      EXPECT_EQ(One->negated(), *NegativeOne);
      EXPECT_EQ(NegativeOne->negated(), *One);
      std::vector<std::uint64_t> Words((Width + 63U) / 64U, 0);
      Words.back() = std::uint64_t{1} << ((Width - 1) % 64U);
      const auto Minimum = IntegerBits::fromWords(Width, Words);
      ASSERT_TRUE(Minimum);
      EXPECT_EQ(Minimum->negated(), *Minimum);
    }
    const std::uint64_t HighWord[] = {0, 1};
    const auto Value = IntegerBits::fromWords(129, HighWord);
    ASSERT_TRUE(Value);
    const auto Negative = Value->negated();
    ASSERT_EQ(Negative.words().size(), 3U);
    EXPECT_EQ(Negative.words()[0], 0U);
    EXPECT_EQ(Negative.words()[1], AllBits);
    EXPECT_EQ(Negative.words()[2], 1U);
    EXPECT_EQ(Negative.negated(), *Value);
  }

  // Equality includes the width and every significant word, and copying rvalues leaves the original value usable.
  TEST(IntegerBitsTest, PreservesValueIdentityAndCopyValidity)
  {
    const std::uint64_t FirstWords[] = {3, 7};
    const std::uint64_t SecondWords[] = {3, 8};
    auto First = IntegerBits::fromWords(128, FirstWords);
    const auto DifferentHighWord = IntegerBits::fromWords(128, SecondWords);
    const auto DifferentWidth = IntegerBits::fromWords(129, FirstWords);
    ASSERT_TRUE(First);
    ASSERT_TRUE(DifferentHighWord);
    ASSERT_TRUE(DifferentWidth);
    EXPECT_NE(*First, *DifferentHighWord);
    EXPECT_NE(*First, *DifferentWidth);
    IntegerBits Copy(std::move(*First));
    EXPECT_EQ(Copy, *First);
    EXPECT_EQ(Copy.hash(), First->hash());
    EXPECT_EQ(First->bitWidth(), 128U);
    EXPECT_FALSE(First->isNegative());
    Copy = std::move(*First);
    EXPECT_EQ(Copy, *First);
  }
} // namespace ink::core::test
