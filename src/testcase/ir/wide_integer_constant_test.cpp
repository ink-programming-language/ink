#include "ink/core/numeric/integer_bits.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/context.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  // Wide raw constants retain every word and interpret the sign bit using their type, independently of target byte order.
  TEST(IRWideIntegerConstantTest, PreservesAllWordsAcrossWidthsAndByteOrders)
  {
    for (const auto Order : {target::ByteOrder::LittleEndian, target::ByteOrder::BigEndian})
    {
      IRContext Context(target::TargetContext(target::PointerWidth::Bits64, Order));
      for (const std::uint32_t Width : {65U, 128U, 257U})
      {
        SCOPED_TRACE(Width);
        std::vector<std::uint64_t> Words((Width + 63U) / 64U, 0);
        Words.front() = 5;
        Words.back() = std::uint64_t{1} << ((Width - 1U) % 64U);
        const auto Bits = core::IntegerBits::fromWords(Width, Words);
        ASSERT_TRUE(Bits.has_value());
        for (const bool Signed : {false, true})
        {
          const auto *Type = Context.typePool().get<ink::ir::IntegerType>(Width, Signed);
          ASSERT_NE(Type, nullptr);
          const auto *Value = Context.constantPool().get<ink::ir::IntegerConstant>(Type, *Bits);
          ASSERT_NE(Value, nullptr);
          EXPECT_EQ(Value->type(), Type);
          EXPECT_EQ(Value->bits().bitWidth(), Width);
          EXPECT_TRUE(std::ranges::equal(Value->bits().words(), Words));
          EXPECT_EQ(Value->bits(), *Bits);
          EXPECT_EQ(Value->isNegative(), Signed);
          EXPECT_EQ(Value->tryGetNonNegativeUInt64(), std::nullopt);
          EXPECT_TRUE(Context.constantPool().owns(Value));
        }
      }
    }
  }

  // Interning includes high words, signedness and top-level qualifiers while repeated equal values retain identity.
  TEST(IRWideIntegerConstantTest, InternsByCompleteBitsAndExactType)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    auto &Constants = Context.constantPool();
    const std::array<std::uint64_t, 2> FirstWords{5, 1};
    const std::array<std::uint64_t, 2> SecondWords{5, 2};
    const auto FirstBits = core::IntegerBits::fromWords(128, FirstWords);
    const auto EqualBits = core::IntegerBits::fromWords(128, FirstWords);
    const auto SecondBits = core::IntegerBits::fromWords(128, SecondWords);
    ASSERT_TRUE(FirstBits.has_value());
    ASSERT_TRUE(EqualBits.has_value());
    ASSERT_TRUE(SecondBits.has_value());
    const auto *UnsignedType = Types.get<ink::ir::IntegerType>(128, false);
    const auto *SignedType = Types.get<ink::ir::IntegerType>(128, true);
    const auto *ConstType = Types.get<ink::ir::IntegerType>(128, false, false);
    const auto *First = Constants.get<ink::ir::IntegerConstant>(UnsignedType, *FirstBits);
    const auto *Second = Constants.get<ink::ir::IntegerConstant>(UnsignedType, *SecondBits);
    const auto *Signed = Constants.get<ink::ir::IntegerConstant>(SignedType, *FirstBits);
    const auto *Const = Constants.get<ink::ir::IntegerConstant>(ConstType, *FirstBits);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Signed, nullptr);
    ASSERT_NE(Const, nullptr);
    EXPECT_EQ(First, Constants.get<ink::ir::IntegerConstant>(UnsignedType, *EqualBits));
    EXPECT_NE(First, Second);
    EXPECT_NE(First, Signed);
    EXPECT_NE(First, Const);
    EXPECT_EQ(Signed->bits(), First->bits());
    EXPECT_EQ(Const->bits(), First->bits());
    EXPECT_EQ(Const->type(), ConstType);
    EXPECT_TRUE(Constants.owns(Const));
  }

  // Typed raw input rejects mismatched widths even for zero, and foreign types cannot enter a local constant pool.
  TEST(IRWideIntegerConstantTest, RejectsWidthMismatchAndForeignTypes)
  {
    IRContext Context(Target);
    IRContext Other(Target);
    auto &Constants = Context.constantPool();
    const auto Bits64 = core::IntegerBits::fromUnsigned(64, 0);
    const auto Bits65 = core::IntegerBits::fromUnsigned(65, 0);
    const auto Bits128 = core::IntegerBits::fromUnsigned(128, 0);
    ASSERT_TRUE(Bits64.has_value());
    ASSERT_TRUE(Bits65.has_value());
    ASSERT_TRUE(Bits128.has_value());
    const auto *Type = Context.typePool().get<ink::ir::IntegerType>(65, false);
    const auto *ForeignType = Other.typePool().get<ink::ir::IntegerType>(65, false);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(nullptr, *Bits65), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Type, *Bits64), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Type, *Bits128), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(ForeignType, *Bits65), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(ForeignType, static_cast<std::uint64_t>(0)), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Other.typePool().get<ink::ir::IntegerType>(65, true), static_cast<std::int64_t>(0)), nullptr);
    const auto *Local = Constants.get<ink::ir::IntegerConstant>(Type, *Bits65);
    const auto *Foreign = Other.constantPool().get<ink::ir::IntegerConstant>(ForeignType, *Bits65);
    ASSERT_NE(Local, nullptr);
    ASSERT_NE(Foreign, nullptr);
    EXPECT_NE(Local, Foreign);
    EXPECT_FALSE(Constants.owns(Foreign));
    EXPECT_TRUE(Other.constantPool().owns(Foreign));
  }

  // Host-sized numeric inputs sign-extend negative values, while explicitly widened raw bits preserve their nonnegative values.
  TEST(IRWideIntegerConstantTest, ExtendsHostSizedNumericValuesWithoutTruncation)
  {
    IRContext Context(Target);
    auto &Constants = Context.constantPool();
    for (const std::uint32_t Width : {65U, 128U, 257U})
    {
      SCOPED_TRACE(Width);
      const auto *SignedType = Context.typePool().get<ink::ir::IntegerType>(Width, true);
      const auto *UnsignedType = Context.typePool().get<ink::ir::IntegerType>(Width, false);
      for (const std::int64_t Number : {std::numeric_limits<std::int64_t>::min(), std::int64_t{-1}, std::int64_t{0}, std::numeric_limits<std::int64_t>::max()})
      {
        const auto *Value = Constants.get<ink::ir::IntegerConstant>(SignedType, static_cast<std::int64_t>(Number));
        ASSERT_NE(Value, nullptr);
        EXPECT_EQ(Value->bits().bitWidth(), Width);
        EXPECT_EQ(Value->bits().tryGetInt64(), Number);
        EXPECT_EQ(Value->isNegative(), Number < 0);
      }
      const auto *MinusOne = Constants.get<ink::ir::IntegerConstant>(SignedType, static_cast<std::int64_t>(-1));
      ASSERT_NE(MinusOne, nullptr);
      const auto Words = MinusOne->bits().words();
      for (std::size_t Index = 0; Index + 1 < Words.size(); ++Index)
      {
        EXPECT_EQ(Words[Index], std::numeric_limits<std::uint64_t>::max());
      }
      const auto LastBits = Width % 64U;
      EXPECT_EQ(Words.back(), LastBits == 0 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << LastBits) - 1);
      const auto RawBits = core::IntegerBits::fromUnsigned(Width, std::numeric_limits<std::uint64_t>::max());
      ASSERT_TRUE(RawBits.has_value());
      const auto *Raw = Constants.get<ink::ir::IntegerConstant>(SignedType, *RawBits);
      const auto *Unsigned = Constants.get<ink::ir::IntegerConstant>(UnsignedType, static_cast<std::uint64_t>(std::numeric_limits<std::uint64_t>::max()));
      ASSERT_NE(Raw, nullptr);
      ASSERT_NE(Unsigned, nullptr);
      EXPECT_FALSE(Raw->isNegative());
      EXPECT_NE(Raw, MinusOne);
      EXPECT_EQ(Raw->tryGetNonNegativeUInt64(), std::numeric_limits<std::uint64_t>::max());
      EXPECT_EQ(Unsigned->tryGetNonNegativeUInt64(), std::numeric_limits<std::uint64_t>::max());
      for (const auto Word : Raw->bits().words().subspan(1))
      {
        EXPECT_EQ(Word, 0U);
      }
    }
  }

  // One-bit signed and unsigned types have different numeric ranges while sharing the same raw bit patterns.
  TEST(IRWideIntegerConstantTest, EnforcesOneBitNumericRanges)
  {
    IRContext Context(Target);
    auto &Constants = Context.constantPool();
    const auto *Signed = Context.typePool().get<ink::ir::IntegerType>(1, true);
    const auto *Unsigned = Context.typePool().get<ink::ir::IntegerType>(1, false);
    const auto *MinusOne = Constants.get<ink::ir::IntegerConstant>(Signed, static_cast<std::int64_t>(-1));
    const auto *One = Constants.get<ink::ir::IntegerConstant>(Unsigned, static_cast<std::uint64_t>(1));
    const auto *SignedZero = Constants.get<ink::ir::IntegerConstant>(Signed, static_cast<std::int64_t>(0));
    const auto *UnsignedZero = Constants.get<ink::ir::IntegerConstant>(Unsigned, static_cast<std::uint64_t>(0));
    ASSERT_NE(MinusOne, nullptr);
    ASSERT_NE(One, nullptr);
    ASSERT_NE(SignedZero, nullptr);
    ASSERT_NE(UnsignedZero, nullptr);
    EXPECT_EQ(SignedZero->tryGetNonNegativeUInt64(), 0U);
    EXPECT_EQ(UnsignedZero->tryGetNonNegativeUInt64(), 0U);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Signed, static_cast<std::int64_t>(1)), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Signed, static_cast<std::int64_t>(-2)), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Unsigned, static_cast<std::uint64_t>(2)), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Unsigned, static_cast<std::int64_t>(-1)), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Signed, static_cast<std::uint64_t>(0)), nullptr);
    const auto TwoBits = core::IntegerBits::fromUnsigned(2, 2);
    const auto OneBit = core::IntegerBits::fromUnsigned(1, 1);
    ASSERT_TRUE(TwoBits.has_value());
    ASSERT_TRUE(OneBit.has_value());
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Signed, *TwoBits), nullptr);
    EXPECT_EQ(MinusOne, Constants.get<ink::ir::IntegerConstant>(Signed, *OneBit));
    EXPECT_EQ(MinusOne->bits(), One->bits());
    EXPECT_TRUE(MinusOne->isNegative());
    EXPECT_FALSE(One->isNegative());
    EXPECT_EQ(MinusOne->tryGetNonNegativeUInt64(), std::nullopt);
    EXPECT_EQ(One->tryGetNonNegativeUInt64(), 1U);
  }

  // A positive high word is rejected by the bounded numeric query instead of being silently discarded.
  TEST(IRWideIntegerConstantTest, RejectsPositiveValuesBeyondUInt64DuringExtraction)
  {
    IRContext Context(Target);
    auto &Constants = Context.constantPool();
    const std::array<std::uint64_t, 2> OverflowWords{7, 1};
    const auto Overflow = core::IntegerBits::fromWords(128, OverflowWords);
    const auto SmallBits = core::IntegerBits::fromUnsigned(128, 7);
    ASSERT_TRUE(Overflow.has_value());
    ASSERT_TRUE(SmallBits.has_value());
    for (const bool Signed : {false, true})
    {
      const auto *Type = Context.typePool().get<ink::ir::IntegerType>(128, Signed);
      const auto *TooLarge = Constants.get<ink::ir::IntegerConstant>(Type, *Overflow);
      const auto *Small = Constants.get<ink::ir::IntegerConstant>(Type, *SmallBits);
      ASSERT_NE(TooLarge, nullptr);
      ASSERT_NE(Small, nullptr);
      EXPECT_FALSE(TooLarge->isNegative());
      EXPECT_EQ(TooLarge->tryGetNonNegativeUInt64(), std::nullopt);
      EXPECT_EQ(Small->tryGetNonNegativeUInt64(), 7U);
      EXPECT_NE(TooLarge, Small);
    }
  }
} // namespace ink::ir::test
