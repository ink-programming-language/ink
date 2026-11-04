#include "ink/core/support/hash.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>

namespace ink::ir::test
{
  static_assert(!std::is_copy_constructible_v<ConstantPool>);
  static_assert(!std::is_move_constructible_v<ConstantPool>);
  static_assert(!std::is_constructible_v<ConstantPool, TypePool &&>);
  static_assert(std::is_constructible_v<IntegerConstant, const IntegerType &, std::uint64_t>);
  static_assert(std::is_constructible_v<FloatConstant, const FloatType &, std::uint64_t>);
  static_assert(std::is_constructible_v<StringConstant, const ArrayType &, std::string_view>);
  static_assert(!std::is_copy_constructible_v<IntegerConstant>);
  static_assert(!std::is_move_constructible_v<IntegerConstant>);
  static_assert(!std::is_copy_constructible_v<FloatConstant>);
  static_assert(!std::is_move_constructible_v<FloatConstant>);
  static_assert(!std::is_copy_constructible_v<StringConstant>);
  static_assert(!std::is_move_constructible_v<StringConstant>);
  using IntegerFactory = const IntegerConstant *(ConstantPool::*)(const IntegerType &, std::uint64_t);
  static_assert(std::is_convertible_v<decltype(&ConstantPool::getInteger), IntegerFactory>);

  namespace
  {
    struct CollidingConstantHash
    {
      public:
        using is_transparent = void;

        template <typename T>
        std::size_t operator()(const T &) const noexcept
        {
          return 0;
        }
    };
  } // namespace

  // Integer bit patterns are interned by both width and signedness without changing signed negative bits.
  TEST(IRConstantPoolTest, InternsIntegerPatternsWithTheirCompleteType)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const auto &Int8 = Types.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Signed);
    const auto &UInt8 = Types.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned);
    const auto &Int16 = Types.getIntegerType(IntegerType::Width::Bits16, IntegerType::Signedness::Signed);
    const auto *Negative = Constants.getInteger(Int8, 255);
    const auto *Unsigned = Constants.getInteger(UInt8, 255);
    const auto *Wider = Constants.getInteger(Int16, 255);
    ASSERT_NE(Negative, nullptr);
    ASSERT_NE(Unsigned, nullptr);
    ASSERT_NE(Wider, nullptr);
    EXPECT_EQ(Constants.getInteger(Int8, 255), Negative);
    EXPECT_EQ(Constants.getSignedInteger(Int8, -1), Negative);
    EXPECT_EQ(Constants.getUnsignedInteger(UInt8, 255), Unsigned);
    EXPECT_NE(Negative, Unsigned);
    EXPECT_NE(Unsigned, Wider);
    EXPECT_EQ(&Negative->getType(), &Int8);
    EXPECT_EQ(Negative->bits(), 255U);
    EXPECT_EQ(Negative->signedValue(), -1);
    EXPECT_EQ(Unsigned->unsignedValue(), 255U);
  }

  // Numeric factories accept each integer endpoint and reject overflow or negative values for unsigned types.
  TEST(IRConstantPoolTest, ChecksNumericRangesForEveryIntegerWidth)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const IntegerType::Width Widths[] = {
        IntegerType::Width::Bits8,
        IntegerType::Width::Bits16,
        IntegerType::Width::Bits32,
        IntegerType::Width::Bits64,
    };
    for (const auto Width : Widths)
    {
      const auto &Signed = Types.getIntegerType(Width, IntegerType::Signedness::Signed);
      const auto &Unsigned = Types.getIntegerType(Width, IntegerType::Signedness::Unsigned);
      const auto Bits = Signed.bitWidth();
      SCOPED_TRACE(Bits);
      const auto SignedMaximum = Bits == 64 ? std::numeric_limits<std::int64_t>::max() : (std::int64_t{1} << (Bits - 1)) - 1;
      const auto SignedMinimum = -SignedMaximum - 1;
      const auto UnsignedMaximum = Bits == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << Bits) - 1;
      const auto *Minimum = Constants.getSignedInteger(Signed, SignedMinimum);
      const auto *Maximum = Constants.getSignedInteger(Signed, SignedMaximum);
      const auto *AllBits = Constants.getUnsignedInteger(Unsigned, UnsignedMaximum);
      ASSERT_NE(Minimum, nullptr);
      ASSERT_NE(Maximum, nullptr);
      ASSERT_NE(AllBits, nullptr);
      EXPECT_EQ(Minimum->signedValue(), SignedMinimum);
      EXPECT_EQ(Maximum->signedValue(), SignedMaximum);
      EXPECT_EQ(AllBits->unsignedValue(), UnsignedMaximum);
      EXPECT_EQ(AllBits->signedValue(), -1);
      EXPECT_EQ(Minimum->bits(), std::uint64_t{1} << (Bits - 1));
      const auto *NegativeOne = Constants.getSignedInteger(Signed, -1);
      ASSERT_NE(NegativeOne, nullptr);
      EXPECT_EQ(NegativeOne->bits(), UnsignedMaximum);
      EXPECT_EQ(NegativeOne->signedValue(), -1);
      EXPECT_EQ(Constants.getInteger(Signed, UnsignedMaximum), NegativeOne);
      EXPECT_EQ(Constants.getUnsignedInteger(Signed, static_cast<std::uint64_t>(SignedMaximum)), Maximum);
      EXPECT_EQ(Constants.getUnsignedInteger(Signed, static_cast<std::uint64_t>(SignedMaximum) + 1), nullptr);
      EXPECT_EQ(Constants.getSignedInteger(Unsigned, -1), nullptr);
      EXPECT_EQ(Constants.getSignedInteger(Unsigned, 0), Constants.getUnsignedInteger(Unsigned, 0));
      if (Bits < 64)
      {
        EXPECT_EQ(Constants.getSignedInteger(Signed, SignedMinimum - 1), nullptr);
        EXPECT_EQ(Constants.getSignedInteger(Signed, SignedMaximum + 1), nullptr);
        EXPECT_EQ(Constants.getUnsignedInteger(Unsigned, UnsignedMaximum + 1), nullptr);
      }
    }
  }

  // Raw patterns reject bits above 8, 16, or 32 bits while all 64-bit patterns remain representable.
  TEST(IRConstantPoolTest, RejectsExcessIntegerPatternBits)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const IntegerType::Width Widths[] = {
        IntegerType::Width::Bits8,
        IntegerType::Width::Bits16,
        IntegerType::Width::Bits32,
    };
    for (const auto Width : Widths)
    {
      const auto &Signed = Types.getIntegerType(Width, IntegerType::Signedness::Signed);
      const auto &Unsigned = Types.getIntegerType(Width, IntegerType::Signedness::Unsigned);
      const auto FirstExcessBit = std::uint64_t{1} << Signed.bitWidth();
      SCOPED_TRACE(Signed.bitWidth());
      EXPECT_NE(Constants.getInteger(Signed, FirstExcessBit - 1), nullptr);
      EXPECT_NE(Constants.getInteger(Unsigned, FirstExcessBit - 1), nullptr);
      EXPECT_EQ(Constants.getInteger(Signed, FirstExcessBit), nullptr);
      EXPECT_EQ(Constants.getInteger(Unsigned, FirstExcessBit | 1), nullptr);
      EXPECT_EQ(Constants.getInteger(Signed, std::numeric_limits<std::uint64_t>::max()), nullptr);
    }
    const auto &Int64 = Types.getIntegerType(IntegerType::Width::Bits64, IntegerType::Signedness::Signed);
    const auto *AllBits = Constants.getInteger(Int64, std::numeric_limits<std::uint64_t>::max());
    ASSERT_NE(AllBits, nullptr);
    EXPECT_EQ(AllBits->bits(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(AllBits->unsignedValue(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(AllBits->signedValue(), -1);
  }

  // Invalid integer widths fail every factory before range arithmetic or canonical type lookup occurs.
  TEST(IRConstantPoolTest, RejectsInvalidIntegerTypeWidths)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    for (const auto Width : {0U, 65U, 128U})
    {
      SCOPED_TRACE(Width);
      for (const auto Sign : {IntegerType::Signedness::Signed, IntegerType::Signedness::Unsigned})
      {
        const IntegerType Invalid(static_cast<IntegerType::Width>(Width), Sign);
        EXPECT_EQ(Constants.getInteger(Invalid, 0), nullptr);
        EXPECT_EQ(Constants.getSignedInteger(Invalid, 0), nullptr);
        EXPECT_EQ(Constants.getUnsignedInteger(Invalid, 0), nullptr);
      }
    }
  }

  // Types supplied by temporary scopes are canonicalized into the associated type pool before storage.
  TEST(IRConstantPoolTest, CanonicalizesBorrowedNumericTypes)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const IntegerConstant *Integer = nullptr;
    const FloatConstant *Float = nullptr;
    {
      const IntegerType ExternalInteger(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
      const FloatType ExternalFloat(FloatType::Width::Bits64);
      Integer = Constants.getSignedInteger(ExternalInteger, 123);
      Float = Constants.getFloatBits(ExternalFloat, 0x3ff0000000000000ULL);
    }
    ASSERT_NE(Integer, nullptr);
    ASSERT_NE(Float, nullptr);
    EXPECT_EQ(&Integer->getType(), &Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed));
    EXPECT_EQ(&Float->getType(), &Types.getFloatType(FloatType::Width::Bits64));
    EXPECT_EQ(Integer->signedValue(), 123);
    EXPECT_EQ(Float->value(), 1.0);
  }

  // Host float and double inputs retain their own precision and share entries with identical raw bits.
  TEST(IRConstantPoolTest, InternsFloatValuesByPrecisionAndBits)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const auto &Float32 = Constants.getFloat(1.5F);
    const auto &Float64 = Constants.getFloat(1.5);
    EXPECT_EQ(&Float32, &Constants.getFloat(1.5F));
    EXPECT_EQ(&Float64, &Constants.getFloat(1.5));
    EXPECT_NE(&Float32, &Float64);
    EXPECT_EQ(Float32.bits(), 0x3fc00000ULL);
    EXPECT_EQ(Float64.bits(), 0x3ff8000000000000ULL);
    EXPECT_EQ(Float32.value(), 1.5);
    EXPECT_EQ(Float64.value(), 1.5);
    EXPECT_EQ(&Float32.getType(), &Types.getFloatType(FloatType::Width::Bits32));
    EXPECT_EQ(&Float64.getType(), &Types.getFloatType(FloatType::Width::Bits64));
    EXPECT_EQ(Constants.getFloatBits(Types.getFloatType(FloatType::Width::Bits32), Float32.bits()), &Float32);
    EXPECT_EQ(Constants.getFloatBits(Types.getFloatType(FloatType::Width::Bits64), Float64.bits()), &Float64);
  }

  // Positive and negative zero remain separate constants although floating point equality considers them equal.
  TEST(IRConstantPoolTest, DistinguishesSignedFloatingPointZeros)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const auto &Positive32 = Constants.getFloat(0.0F);
    const auto &Negative32 = Constants.getFloat(-0.0F);
    const auto &Positive64 = Constants.getFloat(0.0);
    const auto &Negative64 = Constants.getFloat(-0.0);
    EXPECT_NE(&Positive32, &Negative32);
    EXPECT_NE(&Positive64, &Negative64);
    EXPECT_EQ(Positive32.bits(), 0U);
    EXPECT_EQ(Negative32.bits(), 0x80000000ULL);
    EXPECT_EQ(Positive64.bits(), 0U);
    EXPECT_EQ(Negative64.bits(), 0x8000000000000000ULL);
    EXPECT_FALSE(std::signbit(Positive32.value()));
    EXPECT_TRUE(std::signbit(Negative32.value()));
    EXPECT_FALSE(std::signbit(Positive64.value()));
    EXPECT_TRUE(std::signbit(Negative64.value()));
  }

  // NaNs are interned by their payload instead of numeric equality and infinities retain their exact bits.
  TEST(IRConstantPoolTest, PreservesNaNPayloadsAndInfinityPatterns)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const auto &Float32 = Types.getFloatType(FloatType::Width::Bits32);
    const auto &Float64 = Types.getFloatType(FloatType::Width::Bits64);
    const auto *NaN32 = Constants.getFloatBits(Float32, 0x7fc00001ULL);
    const auto *OtherNaN32 = Constants.getFloatBits(Float32, 0x7fc00002ULL);
    const auto *NaN64 = Constants.getFloatBits(Float64, 0x7ff8000000000001ULL);
    const auto *OtherNaN64 = Constants.getFloatBits(Float64, 0x7ff8000000000002ULL);
    const auto *Infinity = Constants.getFloatBits(Float64, 0x7ff0000000000000ULL);
    ASSERT_NE(NaN32, nullptr);
    ASSERT_NE(OtherNaN32, nullptr);
    ASSERT_NE(NaN64, nullptr);
    ASSERT_NE(OtherNaN64, nullptr);
    ASSERT_NE(Infinity, nullptr);
    EXPECT_EQ(Constants.getFloatBits(Float32, 0x7fc00001ULL), NaN32);
    EXPECT_EQ(Constants.getFloatBits(Float64, 0x7ff8000000000001ULL), NaN64);
    EXPECT_NE(NaN32, OtherNaN32);
    EXPECT_NE(NaN64, OtherNaN64);
    EXPECT_EQ(NaN32->bits(), 0x7fc00001ULL);
    EXPECT_EQ(NaN64->bits(), 0x7ff8000000000001ULL);
    EXPECT_TRUE(std::isnan(NaN32->value()));
    EXPECT_TRUE(std::isnan(NaN64->value()));
    EXPECT_TRUE(std::isinf(Infinity->value()));
    EXPECT_EQ(Infinity->bits(), 0x7ff0000000000000ULL);
  }

  // A 32-bit floating point pattern rejects nonzero high bits instead of silently truncating them.
  TEST(IRConstantPoolTest, RejectsExcessBitsForFloat32)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const auto &Float32 = Types.getFloatType(FloatType::Width::Bits32);
    EXPECT_EQ(Constants.getFloatBits(Float32, 0x100000000ULL), nullptr);
    EXPECT_EQ(Constants.getFloatBits(Float32, 0x13f800000ULL), nullptr);
    const auto *AllBits = Constants.getFloatBits(Float32, 0xffffffffULL);
    ASSERT_NE(AllBits, nullptr);
    EXPECT_EQ(AllBits->bits(), 0xffffffffULL);
  }

  // String constants own exact decoded bytes including empty strings, embedded NUL, and UTF-8 sequences.
  TEST(IRConstantPoolTest, OwnsAndInternsStringBytesWithoutATerminator)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const auto &Empty = Constants.getString({});
    EXPECT_TRUE(Empty.value().empty());
    EXPECT_EQ(&Constants.getString(""), &Empty);
    std::string Input("a\0b", 3);
    const auto &Embedded = Constants.getString(Input);
    const auto View = Embedded.value();
    Input.assign("overwritten input storage");
    EXPECT_EQ(View, std::string_view("a\0b", 3));
    EXPECT_EQ(&Embedded, &Constants.getString(std::string_view("a\0b", 3)));
    EXPECT_NE(&Embedded, &Constants.getString("a"));
    const std::string_view Unicode = "\xe4\xb8\xad\xe6\x96\x87";
    const auto &Text = Constants.getString(Unicode);
    EXPECT_EQ(Text.value(), Unicode);
    const auto &Byte = Types.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned);
    EXPECT_EQ(&Empty.getType(), &Types.getArrayType(Byte, 0));
    EXPECT_EQ(&Embedded.getType(), &Types.getArrayType(Byte, 3));
    EXPECT_EQ(&Text.getType(), &Types.getArrayType(Byte, 6));
  }

  // Growing every constant category preserves existing object addresses and borrowed string views.
  TEST(IRConstantPoolTest, PreservesConstantsAndViewsAcrossPoolGrowth)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const auto &UInt64 = Types.getIntegerType(IntegerType::Width::Bits64, IntegerType::Signedness::Unsigned);
    const auto *Integer = Constants.getUnsignedInteger(UInt64, 42);
    const auto &Float = Constants.getFloat(42.5);
    const auto &Text = Constants.getString("first");
    const auto View = Text.value();
    const auto *Characters = View.data();
    ASSERT_NE(Integer, nullptr);
    for (std::uint64_t Index = 0; Index < 512; ++Index)
    {
      ASSERT_NE(Constants.getUnsignedInteger(UInt64, Index), nullptr);
      Constants.getFloat(static_cast<double>(Index));
      Constants.getString(std::to_string(Index));
    }
    EXPECT_EQ(Constants.getUnsignedInteger(UInt64, 42), Integer);
    EXPECT_EQ(&Constants.getFloat(42.5), &Float);
    EXPECT_EQ(&Constants.getString("first"), &Text);
    EXPECT_EQ(Text.value().data(), Characters);
    EXPECT_EQ(View, "first");
    EXPECT_EQ(Integer->unsignedValue(), 42U);
    EXPECT_EQ(Float.value(), 42.5);
  }

  // Separate constant pools retain separate ownership while sharing canonical types from the same type pool.
  TEST(IRConstantPoolTest, KeepsConstantIdentityLocalToEachPool)
  {
    TypePool Types;
    ConstantPool First(Types);
    ConstantPool Second(Types);
    const auto &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const auto *FirstInteger = First.getSignedInteger(Int32, 7);
    const auto *SecondInteger = Second.getSignedInteger(Int32, 7);
    ASSERT_NE(FirstInteger, nullptr);
    ASSERT_NE(SecondInteger, nullptr);
    EXPECT_NE(FirstInteger, SecondInteger);
    EXPECT_EQ(&FirstInteger->getType(), &SecondInteger->getType());
    EXPECT_NE(&First.getFloat(7.0), &Second.getFloat(7.0));
    EXPECT_NE(&First.getString("same"), &Second.getString("same"));
    EXPECT_EQ(&First.getString("same").getType(), &Second.getString("same").getType());
  }

  // Integer keys share hashes with stored constants and remain distinct under forced hash collisions and rehashing.
  TEST(IRConstantPoolTest, FindsIntegerConstantsWithTransparentKeysAndCollisions)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const auto &UInt8 = Types.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned);
    const auto &Int8 = Types.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Signed);
    const auto &Int16 = Types.getIntegerType(IntegerType::Width::Bits16, IntegerType::Signedness::Signed);
    struct Case
    {
        const IntegerConstant *Constant;
        IntegerConstant::Key Key;
    };
    const Case Cases[] = {
        {Constants.getInteger(UInt8, 1), {8, false, 1}},
        {Constants.getInteger(UInt8, 255), {8, false, 255}},
        {Constants.getInteger(Int8, 1), {8, true, 1}},
        {Constants.getInteger(Int16, 0), {16, true, 0}},
    };
    const core::Hash<IntegerConstant> Hash;
    std::unordered_set<IntegerConstant, CollidingConstantHash, std::equal_to<>> Integers;
    for (std::size_t Left = 0; Left < std::size(Cases); ++Left)
    {
      ASSERT_NE(Cases[Left].Constant, nullptr);
      EXPECT_EQ(Cases[Left].Constant->hash(), Cases[Left].Key.hash());
      EXPECT_EQ(Hash(*Cases[Left].Constant), Cases[Left].Constant->hash());
      EXPECT_EQ(Hash(Cases[Left].Key), Cases[Left].Key.hash());
      const auto &ValueType = static_cast<const IntegerType &>(Cases[Left].Constant->getType());
      EXPECT_TRUE(Integers.emplace(ValueType, Cases[Left].Key.Bits).second);
      for (std::size_t Right = 0; Right < std::size(Cases); ++Right)
      {
        ASSERT_NE(Cases[Right].Constant, nullptr);
        EXPECT_EQ(Cases[Left].Key == Cases[Right].Key, Left == Right);
        EXPECT_EQ(*Cases[Left].Constant == *Cases[Right].Constant, Left == Right);
        EXPECT_EQ(*Cases[Left].Constant == Cases[Right].Key, Left == Right);
        EXPECT_EQ(Cases[Left].Key == *Cases[Right].Constant, Left == Right);
      }
    }
    EXPECT_EQ(Integers.size(), std::size(Cases));
    EXPECT_FALSE(Integers.emplace(UInt8, 1).second);
    for (const auto &Entry : Cases)
    {
      const auto Found = Integers.find(Entry.Key);
      ASSERT_NE(Found, Integers.end());
      EXPECT_TRUE(*Found == Entry.Key);
      EXPECT_FALSE(Entry.Key != *Found);
    }
    EXPECT_EQ(Integers.find(IntegerConstant::Key{8, true, 2}), Integers.end());
    EXPECT_TRUE(*Cases[0].Constant != Cases[1].Key);
    EXPECT_TRUE(Cases[1].Key != *Cases[0].Constant);
    const auto *Original = &*Integers.find(Cases[0].Key);
    Integers.rehash(Integers.bucket_count() * 2 + 1);
    const auto Rehashed = Integers.find(Cases[0].Key);
    ASSERT_NE(Rehashed, Integers.end());
    EXPECT_EQ(&*Rehashed, Original);
  }

  // Floating point hash lookup distinguishes precisions, signed zeros, and NaN payloads using their exact bits.
  TEST(IRConstantPoolTest, FindsFloatConstantsWithTransparentBitPatternKeys)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const auto &Float32 = Types.getFloatType(FloatType::Width::Bits32);
    const auto &Float64 = Types.getFloatType(FloatType::Width::Bits64);
    struct Case
    {
        const FloatConstant *Constant;
        FloatConstant::Key Key;
    };
    const Case Cases[] = {
        {Constants.getFloatBits(Float32, 0), {32, 0}},
        {Constants.getFloatBits(Float32, 0x7fc00001ULL), {32, 0x7fc00001ULL}},
        {Constants.getFloatBits(Float32, 0x7fc00002ULL), {32, 0x7fc00002ULL}},
        {Constants.getFloatBits(Float32, 0x80000000ULL), {32, 0x80000000ULL}},
        {Constants.getFloatBits(Float64, 0), {64, 0}},
        {Constants.getFloatBits(Float64, 0x7ff8000000000001ULL), {64, 0x7ff8000000000001ULL}},
        {Constants.getFloatBits(Float64, 0x7ff8000000000002ULL), {64, 0x7ff8000000000002ULL}},
        {Constants.getFloatBits(Float64, 0x8000000000000000ULL), {64, 0x8000000000000000ULL}},
    };
    const core::Hash<FloatConstant> Hash;
    std::unordered_set<FloatConstant, core::Hash<FloatConstant>, std::equal_to<>> Floats;
    for (std::size_t Left = 0; Left < std::size(Cases); ++Left)
    {
      ASSERT_NE(Cases[Left].Constant, nullptr);
      EXPECT_EQ(Cases[Left].Constant->hash(), Cases[Left].Key.hash());
      EXPECT_EQ(Hash(*Cases[Left].Constant), Cases[Left].Constant->hash());
      EXPECT_EQ(Hash(Cases[Left].Key), Cases[Left].Key.hash());
      const auto &ValueType = static_cast<const FloatType &>(Cases[Left].Constant->getType());
      EXPECT_TRUE(Floats.emplace(ValueType, Cases[Left].Key.Bits).second);
      for (std::size_t Right = 0; Right < std::size(Cases); ++Right)
      {
        ASSERT_NE(Cases[Right].Constant, nullptr);
        EXPECT_EQ(Cases[Left].Key == Cases[Right].Key, Left == Right);
        EXPECT_EQ(*Cases[Left].Constant == *Cases[Right].Constant, Left == Right);
        EXPECT_EQ(*Cases[Left].Constant == Cases[Right].Key, Left == Right);
        EXPECT_EQ(Cases[Left].Key == *Cases[Right].Constant, Left == Right);
      }
    }
    EXPECT_EQ(Floats.size(), std::size(Cases));
    EXPECT_FALSE(Floats.emplace(Float32, 0x7fc00001ULL).second);
    for (const auto &Entry : Cases)
    {
      const auto Found = Floats.find(Entry.Key);
      ASSERT_NE(Found, Floats.end());
      EXPECT_TRUE(*Found == Entry.Key);
      EXPECT_FALSE(Entry.Key != *Found);
    }
    EXPECT_EQ(Floats.find(FloatConstant::Key{64, 0x7ff8000000000003ULL}), Floats.end());
    EXPECT_TRUE(*Cases[0].Constant != Cases[3].Key);
    EXPECT_TRUE(Cases[1].Key != *Cases[2].Constant);
    const auto *Original = &*Floats.find(Cases[0].Key);
    Floats.rehash(Floats.bucket_count() * 2 + 1);
    const auto Rehashed = Floats.find(Cases[0].Key);
    ASSERT_NE(Rehashed, Floats.end());
    EXPECT_EQ(&*Rehashed, Original);
  }

  // String hash lookup uses every byte and rehashing preserves constants and views containing embedded NULs.
  TEST(IRConstantPoolTest, FindsStringConstantsWithTransparentViewsAcrossRehashing)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    std::string Long(1024, 'x');
    Long[512] = '\0';
    auto Later = Long;
    Later.back() = 'y';
    const std::string_view Views[] = {
        "",
        "a",
        std::string_view("a\0b", 3),
        std::string_view("a\0c", 3),
        Long,
        Later,
    };
    const core::Hash<StringConstant> Hash;
    std::unordered_set<StringConstant, core::Hash<StringConstant>, std::equal_to<>> Strings;
    for (std::size_t Left = 0; Left < std::size(Views); ++Left)
    {
      const auto &LeftConstant = Constants.getString(Views[Left]);
      EXPECT_EQ(LeftConstant.hash(), Hash(Views[Left]));
      EXPECT_EQ(Hash(LeftConstant), LeftConstant.hash());
      EXPECT_TRUE(Strings.emplace(static_cast<const ArrayType &>(LeftConstant.getType()), Views[Left]).second);
      for (std::size_t Right = 0; Right < std::size(Views); ++Right)
      {
        const auto &RightConstant = Constants.getString(Views[Right]);
        EXPECT_EQ(LeftConstant == RightConstant, Left == Right);
        EXPECT_EQ(LeftConstant == Views[Right], Left == Right);
        EXPECT_EQ(Views[Left] == RightConstant, Left == Right);
        EXPECT_EQ(&LeftConstant == &RightConstant, Left == Right);
      }
    }
    EXPECT_EQ(Strings.size(), std::size(Views));
    for (const auto View : Views)
    {
      const auto Found = Strings.find(View);
      ASSERT_NE(Found, Strings.end());
      EXPECT_EQ(Found->value(), View);
      EXPECT_FALSE(*Found != View);
      EXPECT_FALSE(View != *Found);
      EXPECT_FALSE(Strings.emplace(static_cast<const ArrayType &>(Found->getType()), View).second);
    }
    EXPECT_EQ(Strings.find(std::string_view("a\0d", 3)), Strings.end());
    const auto ByView = Strings.find(std::string_view("a"));
    ASSERT_NE(ByView, Strings.end());
    EXPECT_EQ(Strings.find("a"), ByView);
    EXPECT_EQ(Strings.find(std::string("a")), ByView);
    EXPECT_EQ(Hash("a"), Hash(std::string_view("a")));
    EXPECT_EQ(Hash(std::string("a")), Hash(std::string_view("a")));
    EXPECT_TRUE(Constants.getString(Views[2]) != Views[3]);
    EXPECT_TRUE(Views[3] != Constants.getString(Views[2]));
    const auto *Original = &*Strings.find(Views[4]);
    const auto SavedView = Original->value();
    const auto *Characters = SavedView.data();
    Strings.rehash(Strings.bucket_count() * 2 + 1);
    const auto Rehashed = Strings.find(Views[4]);
    ASSERT_NE(Rehashed, Strings.end());
    EXPECT_EQ(&*Rehashed, Original);
    EXPECT_EQ(Original->value().data(), Characters);
    EXPECT_EQ(SavedView, Views[4]);
  }
} // namespace ink::ir::test
