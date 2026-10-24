#include "ink/core/numeric/integer_bits.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/context.h"

#include <gtest/gtest.h>

#include <concepts>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    template <typename PoolT, typename ConstantT, typename... Args>
    concept CanGetConstant = requires(PoolT &Pool, Args &&... Arguments)
    {
      { Pool.template get<ConstantT>(std::forward<Args>(Arguments)...) } -> std::same_as<const ConstantT *>;
    };

    template <typename ConstantT, typename TypeT>
    concept CanGetBracedHostInteger = requires(ConstantPool &Pool, const TypeT *TypeValue)
    {
      Pool.template get<ConstantT>(TypeValue, {std::uint64_t{0}});
    };
  } // namespace

  // Only singleton queries accept a const pool; numeric families reject unintended host conversions and unsupported argument kinds.
  static_assert(CanGetConstant<const ConstantPool, BoolConstant, bool> && CanGetConstant<const ConstantPool, NullPointerConstant>);
  static_assert(noexcept(std::declval<const ConstantPool &>().get<ink::ir::BoolConstant>(true)) && noexcept(std::declval<const ConstantPool &>().get<ink::ir::NullPointerConstant>()));
  static_assert(CanGetConstant<ConstantPool, IntegerConstant, const IntegerType *, std::int8_t> && CanGetConstant<ConstantPool, IntegerConstant, const IntegerType *, std::uint8_t>);
  static_assert(CanGetConstant<ConstantPool, IntegerConstant, const IntegerType *, std::int64_t> && CanGetConstant<ConstantPool, IntegerConstant, const IntegerType *, std::uint64_t>);
  static_assert(CanGetConstant<ConstantPool, IntegerConstant, const IntegerType *, const core::IntegerBits &>);
  static_assert(!CanGetConstant<ConstantPool, IntegerConstant, const IntegerType *, bool> && !CanGetConstant<ConstantPool, IntegerConstant, const IntegerType *, float>);
  static_assert(!CanGetConstant<ConstantPool, IntegerConstant, const IntegerType *, double> && !CanGetConstant<ConstantPool, IntegerConstant, const IntegerType *, long double>);
  static_assert(!CanGetConstant<const ConstantPool, IntegerConstant, const IntegerType *, std::int64_t>);
  static_assert(!CanGetConstant<const ConstantPool, IntegerConstant, const IntegerType *, const core::IntegerBits &>);
  static_assert(CanGetConstant<ConstantPool, FloatConstant, float> && CanGetConstant<ConstantPool, FloatConstant, double>);
  static_assert(CanGetConstant<ConstantPool, FloatConstant, const FloatType *, const core::IntegerBits &>);
  static_assert(!CanGetConstant<ConstantPool, FloatConstant, int> && !CanGetConstant<ConstantPool, FloatConstant, bool> && !CanGetConstant<ConstantPool, FloatConstant, long double>);
  static_assert(!CanGetConstant<ConstantPool, FloatConstant, const FloatType *, std::uint64_t>);
  static_assert(!CanGetConstant<const ConstantPool, FloatConstant, float> && !CanGetConstant<const ConstantPool, FloatConstant, const FloatType *, const core::IntegerBits &>);
  static_assert(CanGetConstant<ConstantPool, ArrayConstant, const ArrayType *> && CanGetConstant<ConstantPool, ArrayConstant, const ArrayType *, std::vector<const Constant *>>);
  static_assert(CanGetConstant<ConstantPool, StringConstant, std::string>);
  static_assert(!CanGetConstant<const ConstantPool, ArrayConstant, const ArrayType *> && !CanGetConstant<const ConstantPool, StringConstant, std::string>);
  static_assert(!CanGetConstant<ConstantPool, IntegerConstant> && !CanGetConstant<ConstantPool, Constant> && !CanGetConstant<ConstantPool, int>);
  static_assert(!CanGetBracedHostInteger<IntegerConstant, IntegerType> && !CanGetBracedHostInteger<FloatConstant, FloatType>);

  // Small host integers choose numeric signedness before widening, share canonical values with raw bits, and retain sign extension beyond 64 bits.
  TEST(IRConstantPoolApiTest, NumericOverloadsPreserveHostSignednessAndCanonicalValues)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    auto &Constants = Context.constantPool();
    const auto *Signed = Types.get<ink::ir::IntegerType>(128, true);
    const auto *Unsigned = Types.get<ink::ir::IntegerType>(128, false);
    const auto *MinusOne = Constants.get<ink::ir::IntegerConstant>(Signed, -1);
    const auto *Byte = Constants.get<ink::ir::IntegerConstant>(Unsigned, 255U);
    ASSERT_NE(MinusOne, nullptr);
    ASSERT_NE(Byte, nullptr);
    EXPECT_EQ(MinusOne, Constants.get<ink::ir::IntegerConstant>(Signed, std::int8_t{-1}));
    EXPECT_EQ(MinusOne, Constants.get<ink::ir::IntegerConstant>(Signed, std::int16_t{-1}));
    EXPECT_EQ(MinusOne, Constants.get<ink::ir::IntegerConstant>(Signed, std::int32_t{-1}));
    EXPECT_EQ(MinusOne, Constants.get<ink::ir::IntegerConstant>(Signed, std::int64_t{-1}));
    EXPECT_EQ(MinusOne, Constants.get<ink::ir::IntegerConstant>(Signed, MinusOne->bits()));
    EXPECT_EQ(MinusOne->bits().tryGetInt64(), -1);
    EXPECT_TRUE(MinusOne->isNegative());
    EXPECT_EQ(Byte, Constants.get<ink::ir::IntegerConstant>(Unsigned, std::uint8_t{255}));
    EXPECT_EQ(Byte, Constants.get<ink::ir::IntegerConstant>(Unsigned, std::uint16_t{255}));
    EXPECT_EQ(Byte, Constants.get<ink::ir::IntegerConstant>(Unsigned, std::uint32_t{255}));
    EXPECT_EQ(Byte, Constants.get<ink::ir::IntegerConstant>(Unsigned, std::uint64_t{255}));
    const auto WideByteBits = core::IntegerBits::fromUnsigned(128, 255);
    ASSERT_TRUE(WideByteBits.has_value());
    EXPECT_EQ(Byte, Constants.get<ink::ir::IntegerConstant>(Unsigned, *WideByteBits));
    EXPECT_EQ(Byte->tryGetNonNegativeUInt64(), 255U);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Signed, 0U), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Unsigned, 0), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Signed, std::uint8_t{0}), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Unsigned, std::int8_t{0}), nullptr);
    const auto *SignedByte = Types.get<ink::ir::IntegerType>(8, true);
    const auto *SignedByteMinusOne = Constants.get<ink::ir::IntegerConstant>(SignedByte, std::int8_t{-1});
    ASSERT_NE(SignedByteMinusOne, nullptr);
    const auto ByteBits = core::IntegerBits::fromUnsigned(8, 255);
    ASSERT_TRUE(ByteBits.has_value());
    EXPECT_EQ(SignedByteMinusOne, Constants.get<ink::ir::IntegerConstant>(SignedByte, *ByteBits));
  }
} // namespace ink::ir::test
