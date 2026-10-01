#include "ink/execution/value/execution_integer.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <limits>
#include <span>

namespace ink::execution::test
{
  namespace
  {
    constexpr std::uint64_t MaxWord = std::numeric_limits<std::uint64_t>::max();

    ExecutionInteger integer(std::uint32_t Width, std::initializer_list<std::uint64_t> Words)
    {
      return ExecutionInteger(ir::IntegerBits(Width, std::span<const std::uint64_t>(Words.begin(), Words.size())));
    }

    void expectWords(const ExecutionInteger &Value, std::uint32_t Width, std::initializer_list<std::uint64_t> Expected)
    {
      ASSERT_TRUE(Value.valid());
      EXPECT_EQ(Value.bitWidth(), Width);
      const ir::IntegerBits Bits = Value.bits();
      ASSERT_EQ(Bits.words().size(), Expected.size());
      std::size_t Index = 0;
      for (const std::uint64_t Word : Expected)
      {
        EXPECT_EQ(Bits.words()[Index], Word) << "word " << Index;
        ++Index;
      }
    }

    std::uint64_t lowWord(const ExecutionInteger &Value)
    {
      return Value.bits().words().front();
    }
  } // namespace

  // Malformed bit widths, word counts and unused high bits remain invalid and cannot overwrite an existing result.
  TEST(ExecutionIntegerTest, RejectsMalformedInputsWithoutChangingOutputs)
  {
    const ExecutionInteger Cases[] = {
        ExecutionInteger(0),
        ExecutionInteger(8, 256),
        integer(128, {1}),
        integer(65, {0, 2}),
    };
    for (const ExecutionInteger &Value : Cases)
    {
      EXPECT_FALSE(Value.valid());
      EXPECT_FALSE(Value.isZero());
      EXPECT_FALSE(Value.isNegative());
      ExecutionInteger Result(8, 73);
      EXPECT_EQ(Value.add(Value, Result), ExecutionStatus::TypeMismatch);
      EXPECT_EQ(Value.negate(Result), ExecutionStatus::TypeMismatch);
      EXPECT_EQ(Value.shiftLeft(0, Result), ExecutionStatus::TypeMismatch);
      EXPECT_EQ(Value.divide(Value, false, Result), ExecutionStatus::TypeMismatch);
      int Comparison = 42;
      EXPECT_EQ(Value.compare(Value, false, Comparison), ExecutionStatus::TypeMismatch);
      EXPECT_EQ(Comparison, 42);
      expectWords(Result, 8, {73});
    }
    ExecutionInteger Result(8, 73);
    EXPECT_EQ(ExecutionInteger(32, 1).multiply(ExecutionInteger(64, 1), Result), ExecutionStatus::TypeMismatch);
    expectWords(Result, 8, {73});
  }

  // Carries and borrows cross 64-bit words and wrap at exact widths including one bit and a partial final word.
  TEST(ExecutionIntegerTest, PropagatesCarryAndBorrowAcrossWords)
  {
    ExecutionInteger Result(128);
    const ExecutionInteger One(128, 1);
    ASSERT_EQ(integer(128, {MaxWord, 0}).add(One, Result), ExecutionStatus::Success);
    expectWords(Result, 128, {0, 1});
    ASSERT_EQ(Result.subtract(One, Result), ExecutionStatus::Success);
    expectWords(Result, 128, {MaxWord, 0});
    ASSERT_EQ(integer(128, {MaxWord, MaxWord}).add(One, Result), ExecutionStatus::Success);
    expectWords(Result, 128, {0, 0});
    ASSERT_EQ(Result.subtract(One, Result), ExecutionStatus::Success);
    expectWords(Result, 128, {MaxWord, MaxWord});
    ASSERT_EQ(integer(65, {MaxWord, 1}).add(ExecutionInteger(65, 1), Result), ExecutionStatus::Success);
    expectWords(Result, 65, {0, 0});
    ASSERT_EQ(ExecutionInteger(1, 1).add(ExecutionInteger(1, 1), Result), ExecutionStatus::Success);
    expectWords(Result, 1, {0});
  }

  // Schoolbook multiplication handles full 32-bit limb products and retains only bits inside 96, 128, 129 and 192-bit results.
  TEST(ExecutionIntegerTest, MultipliesAcrossLimbAndWordBoundaries)
  {
    ExecutionInteger Result(128);
    const ExecutionInteger LowMax = integer(128, {MaxWord, 0});
    ASSERT_EQ(LowMax.multiply(LowMax, Result), ExecutionStatus::Success);
    expectWords(Result, 128, {1, MaxWord - 1});
    const ExecutionInteger FullMax = integer(128, {MaxWord, MaxWord});
    ASSERT_EQ(FullMax.multiply(FullMax, Result), ExecutionStatus::Success);
    expectWords(Result, 128, {1, 0});
    const ExecutionInteger Around64 = integer(129, {1, 1, 0});
    ASSERT_EQ(Around64.multiply(Around64, Result), ExecutionStatus::Success);
    expectWords(Result, 129, {1, 2, 1});
    const ExecutionInteger LowLimb(96, 0xFFFFFFFF);
    ASSERT_EQ(LowLimb.multiply(LowLimb, Result), ExecutionStatus::Success);
    expectWords(Result, 96, {0xFFFFFFFE00000001ULL, 0});
    ASSERT_EQ(integer(192, {3, 0, 1}).multiply(integer(192, {2, 1, 0}), Result), ExecutionStatus::Success);
    expectWords(Result, 192, {6, 3, 2});
    Result = integer(255, {MaxWord, MaxWord, MaxWord, MaxWord >> 1});
    ASSERT_EQ(Result.multiply(Result, Result), ExecutionStatus::Success);
    expectWords(Result, 255, {1, 0, 0, 0});
  }

  // Wide unsigned division preserves trial-remainder carry for high-bit divisors and returns exact quotient and remainder words.
  TEST(ExecutionIntegerTest, DividesWideUnsignedValuesWithoutLosingHighBits)
  {
    struct Case
    {
        ExecutionInteger Dividend;
        ExecutionInteger Divisor;
        ExecutionInteger Quotient;
        ExecutionInteger Remainder;
    };
    const Case Cases[] = {
        {integer(128, {MaxWord, MaxWord}), integer(128, {1, 1}), integer(128, {MaxWord, 0}), ExecutionInteger(128)},
        {integer(128, {MaxWord, MaxWord}), integer(128, {1, std::uint64_t{1} << 63}), ExecutionInteger(128, 1), integer(128, {MaxWord - 1, MaxWord >> 1})},
        {integer(128, {0, 1}), ExecutionInteger(128, 3), ExecutionInteger(128, 0x5555555555555555ULL), ExecutionInteger(128, 1)},
        {integer(65, {MaxWord, 1}), integer(65, {1, 1}), ExecutionInteger(65, 1), ExecutionInteger(65, MaxWord - 1)},
        {integer(129, {17, 0, 1}), integer(129, {3, 1, 0}), ExecutionInteger(129, MaxWord - 2), ExecutionInteger(129, 26)},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Dividend.bitWidth());
      ExecutionInteger Result(1);
      ASSERT_EQ(Entry.Dividend.divide(Entry.Divisor, false, Result), ExecutionStatus::Success);
      EXPECT_EQ(Result.bits(), Entry.Quotient.bits());
      ASSERT_EQ(Entry.Dividend.remainder(Entry.Divisor, false, Result), ExecutionStatus::Success);
      EXPECT_EQ(Result.bits(), Entry.Remainder.bits());
    }
  }

  // Signed division truncates toward zero, follows the dividend's remainder sign and wraps minimum divided by negative one.
  TEST(ExecutionIntegerTest, PreservesSignedDivisionAndRemainderRules)
  {
    const ExecutionInteger Negative17 = integer(128, {MaxWord - 16, MaxWord});
    const ExecutionInteger Negative5 = integer(128, {MaxWord - 4, MaxWord});
    const ExecutionInteger Minimum = integer(128, {0, std::uint64_t{1} << 63});
    ExecutionInteger Result(128);
    ASSERT_EQ(Negative17.divide(ExecutionInteger(128, 5), true, Result), ExecutionStatus::Success);
    expectWords(Result, 128, {MaxWord - 2, MaxWord});
    ASSERT_EQ(Negative17.remainder(ExecutionInteger(128, 5), true, Result), ExecutionStatus::Success);
    expectWords(Result, 128, {MaxWord - 1, MaxWord});
    ASSERT_EQ(ExecutionInteger(128, 17).divide(Negative5, true, Result), ExecutionStatus::Success);
    expectWords(Result, 128, {MaxWord - 2, MaxWord});
    ASSERT_EQ(ExecutionInteger(128, 17).remainder(Negative5, true, Result), ExecutionStatus::Success);
    expectWords(Result, 128, {2, 0});
    ASSERT_EQ(Minimum.divide(integer(128, {MaxWord, MaxWord}), true, Result), ExecutionStatus::Success);
    EXPECT_EQ(Result.bits(), Minimum.bits());
    ASSERT_EQ(Minimum.remainder(integer(128, {MaxWord, MaxWord}), true, Result), ExecutionStatus::Success);
    expectWords(Result, 128, {0, 0});
    ASSERT_EQ(ExecutionInteger(1, 1).divide(ExecutionInteger(1, 1), true, Result), ExecutionStatus::Success);
    expectWords(Result, 1, {1});
  }

  // Zero divisors and incompatible widths return explicit failures while leaving the caller's existing value untouched.
  TEST(ExecutionIntegerTest, RejectsInvalidDivisionWithoutChangingResult)
  {
    ExecutionInteger Result(8, 91);
    const ExecutionInteger Value(128, 17);
    EXPECT_EQ(Value.divide(ExecutionInteger(128), true, Result), ExecutionStatus::DivisionByZero);
    EXPECT_EQ(Value.remainder(ExecutionInteger(128), false, Result), ExecutionStatus::DivisionByZero);
    EXPECT_EQ(Value.divide(ExecutionInteger(64, 1), false, Result), ExecutionStatus::TypeMismatch);
    expectWords(Result, 8, {91});
  }

  // Shifts at 63, 64 and 65 bits move whole words correctly and arithmetic shifts fill only the declared sign bits.
  TEST(ExecutionIntegerTest, ShiftsAcrossWordsAndExtendsPartialWidthSigns)
  {
    const ExecutionInteger Value = integer(192, {1, 2, 4});
    ExecutionInteger Result(192);
    ASSERT_EQ(Value.shiftLeft(63, Result), ExecutionStatus::Success);
    expectWords(Result, 192, {std::uint64_t{1} << 63, 0, 1});
    ASSERT_EQ(Value.shiftLeft(64, Result), ExecutionStatus::Success);
    expectWords(Result, 192, {0, 1, 2});
    ASSERT_EQ(Value.shiftLeft(65, Result), ExecutionStatus::Success);
    expectWords(Result, 192, {0, 2, 4});
    ASSERT_EQ(Value.shiftRight(63, false, Result), ExecutionStatus::Success);
    expectWords(Result, 192, {4, 8, 0});
    ASSERT_EQ(Value.shiftRight(64, false, Result), ExecutionStatus::Success);
    expectWords(Result, 192, {2, 4, 0});
    ASSERT_EQ(Value.shiftRight(65, false, Result), ExecutionStatus::Success);
    expectWords(Result, 192, {1, 2, 0});
    const ExecutionInteger Sign = integer(65, {0, 1});
    ASSERT_EQ(Sign.shiftRight(1, false, Result), ExecutionStatus::Success);
    expectWords(Result, 65, {std::uint64_t{1} << 63, 0});
    ASSERT_EQ(Sign.shiftRight(1, true, Result), ExecutionStatus::Success);
    expectWords(Result, 65, {std::uint64_t{1} << 63, 1});
    ASSERT_EQ(Sign.shiftRight(64, true, Result), ExecutionStatus::Success);
    expectWords(Result, 65, {MaxWord, 1});
    ASSERT_EQ(integer(129, {1, 1, 1}).shiftRight(65, false, Result), ExecutionStatus::Success);
    expectWords(Result, 129, {std::uint64_t{1} << 63, 0, 0});
    ASSERT_EQ(integer(129, {1, 1, 1}).shiftLeft(65, Result), ExecutionStatus::Success);
    expectWords(Result, 129, {0, 2, 0});
    ASSERT_EQ(Value.shiftRight(0, true, Result), ExecutionStatus::Success);
    EXPECT_EQ(Result.bits(), Value.bits());
  }

  // Negative, oversized and multiword shift counts fail before narrowing and cannot mutate either kind of output.
  TEST(ExecutionIntegerTest, RejectsOutOfRangeShiftCounts)
  {
    ExecutionInteger Result(8, 91);
    EXPECT_EQ(ExecutionInteger(65, 1).shiftLeft(65, Result), ExecutionStatus::InvalidShift);
    EXPECT_EQ(ExecutionInteger(65, 1).shiftRight(65, true, Result), ExecutionStatus::InvalidShift);
    expectWords(Result, 8, {91});
    std::uint32_t Amount = 17;
    EXPECT_EQ(ExecutionInteger(128, 128).shiftAmount(128, false, Amount), ExecutionStatus::InvalidShift);
    EXPECT_EQ(integer(128, {0, 1}).shiftAmount(128, false, Amount), ExecutionStatus::InvalidShift);
    EXPECT_EQ(ExecutionInteger(8, 255).shiftAmount(256, true, Amount), ExecutionStatus::InvalidShift);
    EXPECT_EQ(Amount, 17U);
    EXPECT_EQ(ExecutionInteger(128, 127).shiftAmount(128, true, Amount), ExecutionStatus::Success);
    EXPECT_EQ(Amount, 127U);
  }

  // Negation and bitwise operations preserve unused high bits and support aliasing an input as the output.
  TEST(ExecutionIntegerTest, MasksUnaryAndBitwiseResultsToTheirWidth)
  {
    ExecutionInteger Result(70);
    ASSERT_EQ(Result.invert(Result), ExecutionStatus::Success);
    expectWords(Result, 70, {MaxWord, 63});
    ASSERT_EQ(Result.negate(Result), ExecutionStatus::Success);
    expectWords(Result, 70, {1, 0});
    const ExecutionInteger Left = integer(129, {0xF0, 0x0F, 1});
    const ExecutionInteger Right = integer(129, {0xCC, 0x33, 0});
    ASSERT_EQ(Left.bitwiseAnd(Right, Result), ExecutionStatus::Success);
    expectWords(Result, 129, {0xC0, 3, 0});
    ASSERT_EQ(Left.bitwiseOr(Right, Result), ExecutionStatus::Success);
    expectWords(Result, 129, {0xFC, 0x3F, 1});
    ASSERT_EQ(Left.bitwiseXor(Right, Result), ExecutionStatus::Success);
    expectWords(Result, 129, {0x3C, 0x3C, 1});
    Result = integer(128, {0, std::uint64_t{1} << 63});
    const ir::IntegerBits Minimum = Result.bits();
    ASSERT_EQ(Result.negate(Result), ExecutionStatus::Success);
    EXPECT_EQ(Result.bits(), Minimum);
  }

  // Signed comparisons treat the declared high bit as the sign and unsigned comparisons still inspect every word.
  TEST(ExecutionIntegerTest, ComparesSignedAndUnsignedWideValues)
  {
    const ExecutionInteger Minimum = integer(129, {0, 0, 1});
    const ExecutionInteger Zero(129);
    int Result = 0;
    ASSERT_EQ(Minimum.compare(Zero, true, Result), ExecutionStatus::Success);
    EXPECT_EQ(Result, -1);
    ASSERT_EQ(Minimum.compare(Zero, false, Result), ExecutionStatus::Success);
    EXPECT_EQ(Result, 1);
    ASSERT_EQ(integer(129, {MaxWord - 1, MaxWord, 1}).compare(integer(129, {MaxWord, MaxWord, 1}), true, Result), ExecutionStatus::Success);
    EXPECT_EQ(Result, -1);
    ASSERT_EQ(integer(129, {0, 1, 0}).compare(integer(129, {MaxWord, 0, 0}), false, Result), ExecutionStatus::Success);
    EXPECT_EQ(Result, 1);
    ASSERT_EQ(Minimum.compare(Minimum, true, Result), ExecutionStatus::Success);
    EXPECT_EQ(Result, 0);
  }

  // Every pair of eight-bit operands agrees with safe native wider arithmetic, including all signed division edge cases.
  TEST(ExecutionIntegerTest, MatchesExhaustiveEightBitArithmetic)
  {
    for (std::uint32_t Left = 0; Left < 256; ++Left)
    {
      SCOPED_TRACE(Left);
      const ExecutionInteger First(8, Left);
      for (std::uint32_t Right = 0; Right < 256; ++Right)
      {
        SCOPED_TRACE(Right);
        const ExecutionInteger Second(8, Right);
        ExecutionInteger Result(8);
        ASSERT_EQ(First.add(Second, Result), ExecutionStatus::Success);
        EXPECT_EQ(lowWord(Result), (Left + Right) & 255U);
        ASSERT_EQ(First.subtract(Second, Result), ExecutionStatus::Success);
        EXPECT_EQ(lowWord(Result), (Left - Right) & 255U);
        ASSERT_EQ(First.multiply(Second, Result), ExecutionStatus::Success);
        EXPECT_EQ(lowWord(Result), (Left * Right) & 255U);
        if (Right == 0)
        {
          continue;
        }
        ASSERT_EQ(First.divide(Second, false, Result), ExecutionStatus::Success);
        EXPECT_EQ(lowWord(Result), Left / Right);
        ASSERT_EQ(First.remainder(Second, false, Result), ExecutionStatus::Success);
        EXPECT_EQ(lowWord(Result), Left % Right);
        const int SignedLeft = static_cast<int>(Left) - (Left >= 128 ? 256 : 0);
        const int SignedRight = static_cast<int>(Right) - (Right >= 128 ? 256 : 0);
        ASSERT_EQ(First.divide(Second, true, Result), ExecutionStatus::Success);
        EXPECT_EQ(lowWord(Result), static_cast<std::uint32_t>(SignedLeft / SignedRight) & 255U);
        ASSERT_EQ(First.remainder(Second, true, Result), ExecutionStatus::Success);
        EXPECT_EQ(lowWord(Result), static_cast<std::uint32_t>(SignedLeft % SignedRight) & 255U);
      }
    }
  }
} // namespace ink::execution::test
