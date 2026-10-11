#include "ink/target/target_context.h"
#include "ink/ir/constant/constant_context.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    template <typename ConstantType>
    constexpr bool HasContextOwnedIdentity = !std::is_default_constructible_v<ConstantType> && !std::is_copy_constructible_v<ConstantType> && !std::is_move_constructible_v<ConstantType> && !std::is_copy_assignable_v<ConstantType> && !std::is_move_assignable_v<ConstantType>;

    static_assert(!std::is_copy_constructible_v<ConstantContext>);
    static_assert(!std::is_move_constructible_v<ConstantContext>);
    static_assert(!std::is_copy_assignable_v<ConstantContext>);
    static_assert(!std::is_move_assignable_v<ConstantContext>);
    static_assert(std::is_constructible_v<ConstantContext, const TypeContext &>);
    static_assert(!std::is_constructible_v<ConstantContext, TypeContext &&>);
    static_assert(!std::is_constructible_v<ConstantContext, const TypeContext &&>);
    static_assert(std::is_same_v<decltype(std::declval<const ConstantContext &>().getBoolConstant(true)), const BoolConstant *>);
    static_assert(noexcept(std::declval<const ConstantContext &>().getBoolConstant(true)));

    static_assert(HasContextOwnedIdentity<BoolConstant>);
    static_assert(HasContextOwnedIdentity<IntegerConstant>);
    static_assert(HasContextOwnedIdentity<FloatConstant>);
    static_assert(HasContextOwnedIdentity<NullPointerConstant>);
    static_assert(HasContextOwnedIdentity<StringConstant>);
    static_assert(HasContextOwnedIdentity<ArrayConstant>);
    static_assert(!std::is_constructible_v<BoolConstant, const BoolType *, bool>);
    static_assert(!std::is_constructible_v<IntegerConstant, const IntegerType *, std::uint64_t>);
    static_assert(!std::is_constructible_v<FloatConstant, const FloatType *, std::uint64_t>);
    static_assert(!std::is_constructible_v<NullPointerConstant, const PointerType *>);
    static_assert(!std::is_constructible_v<StringConstant, const StringType *, std::string>);
    static_assert(!std::is_constructible_v<ArrayConstant, const ArrayType *, std::vector<const Constant *>>);
  } // namespace

  // Each context has exactly two canonical booleans and recognizes only its own constants.
  TEST(IRConstantContextTest, BooleanLookupsRetainContextOwnership)
  {
    const TypeContext Types(Target);
    const ConstantContext Constants(Types);
    const ConstantContext Other(Types);
    const BoolConstant *True = Constants.getBoolConstant(true);
    const BoolConstant *False = Constants.getBoolConstant(false);
    ASSERT_NE(True, nullptr);
    ASSERT_NE(False, nullptr);
    EXPECT_EQ(&Constants.typeContext(), &Types);
    EXPECT_EQ(True, Constants.getBoolConstant(true));
    EXPECT_EQ(False, Constants.getBoolConstant(false));
    EXPECT_NE(True, False);
    EXPECT_TRUE(True->value());
    EXPECT_FALSE(False->value());
    EXPECT_EQ(True->type(), Types.getBoolType());
    EXPECT_EQ(False->type(), Types.getBoolType());
    EXPECT_TRUE(Constants.owns(True));
    EXPECT_TRUE(Constants.owns(False));
    EXPECT_FALSE(Constants.owns(nullptr));
    EXPECT_FALSE(Constants.owns(Other.getBoolConstant(true)));
    EXPECT_NE(True, Other.getBoolConstant(true));
  }

  // Numeric integer lookups normalize to raw bits while signedness and width remain part of identity.
  TEST(IRConstantContextTest, IntegerLookupsReuseNormalizedBitsPerType)
  {
    const TypeContext Types(Target);
    ConstantContext Constants(Types);
    const IntegerConstant *Signed32 = Constants.getSignedIntegerConstant(Types.getInt32Type(), -1);
    const IntegerConstant *Unsigned32 = Constants.getUnsignedIntegerConstant(Types.getUInt32Type(), std::numeric_limits<std::uint32_t>::max());
    const IntegerConstant *Signed64 = Constants.getSignedIntegerConstant(Types.getInt64Type(), -1);
    ASSERT_NE(Signed32, nullptr);
    ASSERT_NE(Unsigned32, nullptr);
    ASSERT_NE(Signed64, nullptr);
    EXPECT_EQ(Signed32, Constants.getIntegerConstantFromBits(Types.getInt32Type(), 0xffffffffULL));
    EXPECT_EQ(Unsigned32, Constants.getIntegerConstantFromBits(Types.getUInt32Type(), 0xffffffffULL));
    EXPECT_EQ(Signed64, Constants.getIntegerConstantFromBits(Types.getInt64Type(), 0xffffffffffffffffULL));
    EXPECT_NE(Signed32, Unsigned32);
    EXPECT_NE(Signed32, Signed64);
    EXPECT_NE(Constants.getSignedIntegerConstant(Types.getInt32Type(), 0), Constants.getSignedIntegerConstant(Types.getInt64Type(), 0));
    EXPECT_NE(Constants.getSignedIntegerConstant(Types.getInt32Type(), 0), Constants.getUnsignedIntegerConstant(Types.getUInt32Type(), 0));
    EXPECT_NE(Signed32, Constants.getSignedIntegerConstant(Types.getInt32Type(), 1));
    EXPECT_EQ(Signed32->bits(), 0xffffffffULL);
    EXPECT_TRUE(Constants.owns(Signed32));
    EXPECT_TRUE(Constants.owns(Unsigned32));
    EXPECT_TRUE(Constants.owns(Signed64));
  }

  // Numeric floating-point lookups infer the local width and share exact-bit identity without collapsing signed zero or NaN payloads.
  TEST(IRConstantContextTest, FloatLookupsReuseExactBitsWithoutCollapsingSpecialValues)
  {
    const TypeContext Types(Target);
    ConstantContext Constants(Types);
    const FloatConstant *Float32 = Constants.getFloatConstant(1.25f);
    const FloatConstant *Float64 = Constants.getFloatConstant(1.25);
    const FloatConstant *PositiveZero = Constants.getFloatConstant(0.0f);
    const FloatConstant *NegativeZero = Constants.getFloatConstant(-0.0f);
    const FloatConstant *FirstNaN = Constants.getFloatConstantFromBits(Types.getFloat32Type(), 0x7fc00001ULL);
    const FloatConstant *SecondNaN = Constants.getFloatConstantFromBits(Types.getFloat32Type(), 0x7fc00002ULL);
    ASSERT_NE(Float32, nullptr);
    ASSERT_NE(Float64, nullptr);
    ASSERT_NE(PositiveZero, nullptr);
    ASSERT_NE(NegativeZero, nullptr);
    ASSERT_NE(FirstNaN, nullptr);
    ASSERT_NE(SecondNaN, nullptr);
    EXPECT_EQ(Float32->type(), Types.getFloat32Type());
    EXPECT_EQ(Float64->type(), Types.getFloat64Type());
    EXPECT_NE(Float32, Float64);
    EXPECT_EQ(Float32, Constants.getFloatConstantFromBits(Types.getFloat32Type(), 0x3fa00000ULL));
    EXPECT_EQ(Float64, Constants.getFloatConstantFromBits(Types.getFloat64Type(), 0x3ff4000000000000ULL));
    EXPECT_EQ(PositiveZero, Constants.getFloatConstantFromBits(Types.getFloat32Type(), 0));
    EXPECT_EQ(NegativeZero, Constants.getFloatConstantFromBits(Types.getFloat32Type(), 0x80000000ULL));
    EXPECT_NE(PositiveZero, NegativeZero);
    EXPECT_NE(PositiveZero, Constants.getFloatConstant(0.0));
    EXPECT_EQ(FirstNaN, Constants.getFloatConstantFromBits(Types.getFloat32Type(), 0x7fc00001ULL));
    EXPECT_NE(FirstNaN, SecondNaN);
    EXPECT_EQ(FirstNaN->bits(), 0x7fc00001ULL);
    EXPECT_EQ(SecondNaN->bits(), 0x7fc00002ULL);
    EXPECT_TRUE(Constants.owns(Float32));
    EXPECT_TRUE(Constants.owns(Float64));
  }

  // Typed null pointers are reused only when their canonical pointer types match.
  TEST(IRConstantContextTest, NullPointerLookupUsesPointerTypeIdentity)
  {
    TypeContext Types(Target);
    ConstantContext Constants(Types);
    const PointerType *BoolPointer = Types.getPointerType(Types.getBoolType());
    const PointerType *IntegerPointer = Types.getPointerType(Types.getInt32Type());
    const NullPointerConstant *BoolNull = Constants.getNullPointerConstant(BoolPointer);
    const NullPointerConstant *IntegerNull = Constants.getNullPointerConstant(IntegerPointer);
    ASSERT_NE(BoolNull, nullptr);
    ASSERT_NE(IntegerNull, nullptr);
    EXPECT_EQ(BoolNull, Constants.getNullPointerConstant(BoolPointer));
    EXPECT_EQ(IntegerNull, Constants.getNullPointerConstant(IntegerPointer));
    EXPECT_NE(BoolNull, IntegerNull);
    EXPECT_EQ(BoolNull->type(), BoolPointer);
    EXPECT_EQ(IntegerNull->type(), IntegerPointer);
    EXPECT_TRUE(Constants.owns(BoolNull));
    EXPECT_TRUE(Constants.owns(IntegerNull));
  }

  // Strings infer the local type, preserve exact UTF-8 identity and keep borrowed views valid across cache growth.
  TEST(IRConstantContextTest, StringLookupsPreserveBytesAndStableViews)
  {
    const TypeContext Types(Target);
    ConstantContext Constants(Types);
    const StringType *String = Types.getStringType();
    const std::string EmbeddedNul("a\0b", 3);
    const StringConstant *BinarySafe = Constants.getStringConstant(EmbeddedNul);
    const StringConstant *Composed = Constants.getStringConstant("\xC3\xA9");
    const StringConstant *Decomposed = Constants.getStringConstant("e\xCC\x81");
    const std::string StableText(128, 'x');
    const StringConstant *Stable = Constants.getStringConstant(StableText);
    ASSERT_NE(BinarySafe, nullptr);
    ASSERT_NE(Composed, nullptr);
    ASSERT_NE(Decomposed, nullptr);
    ASSERT_NE(Stable, nullptr);
    const std::string_view View = Stable->value();
    const char *Data = View.data();
    EXPECT_EQ(BinarySafe->type(), String);
    EXPECT_EQ(Stable->type(), String);
    EXPECT_EQ(BinarySafe, Constants.getStringConstant(std::string("a\0b", 3)));
    EXPECT_EQ(BinarySafe->value(), std::string_view(EmbeddedNul));
    EXPECT_NE(BinarySafe, Constants.getStringConstant("a"));
    EXPECT_EQ(Composed, Constants.getStringConstant("\xC3\xA9"));
    EXPECT_EQ(Decomposed, Constants.getStringConstant("e\xCC\x81"));
    EXPECT_NE(Composed, Decomposed);
    for (std::uint32_t Index = 0; Index < 256; ++Index)
    {
      ASSERT_NE(Constants.getStringConstant(std::to_string(Index)), nullptr);
      ASSERT_NE(Constants.getUnsignedIntegerConstant(Types.getUInt32Type(), Index), nullptr);
    }
    EXPECT_EQ(Stable, Constants.getStringConstant(StableText));
    EXPECT_EQ(View, StableText);
    EXPECT_EQ(Stable->value().data(), Data);
    EXPECT_TRUE(Constants.owns(BinarySafe));
    EXPECT_TRUE(Constants.owns(Stable));
  }

  // Inferred string and floating-point types follow each target while constants remain isolated even when contexts share types.
  TEST(IRConstantContextTest, InferredTypeLookupsRetainContextOwnershipAndTargetLayout)
  {
    constexpr target::TargetContext NarrowTarget(target::PointerWidth::Bits32, target::ByteOrder::BigEndian);
    const TypeContext Types(Target);
    const TypeContext OtherTypes(Target);
    const TypeContext NarrowTypes(NarrowTarget);
    ConstantContext Constants(Types);
    ConstantContext SharedTypes(Types);
    ConstantContext Other(OtherTypes);
    ConstantContext Narrow(NarrowTypes);
    struct ContextCase
    {
        const TypeContext *Types;
        ConstantContext *Constants;
    };
    const ContextCase Cases[] = {
        {&Types, &Constants},
        {&Types, &SharedTypes},
        {&OtherTypes, &Other},
        {&NarrowTypes, &Narrow},
    };
    for (const ContextCase &Case : Cases)
    {
      const auto String = Case.Constants->getStringConstant("same text");
      const auto Float32 = Case.Constants->getFloatConstant(1.25f);
      const auto Float64 = Case.Constants->getFloatConstant(1.25);
      ASSERT_NE(String, nullptr);
      ASSERT_NE(Float32, nullptr);
      ASSERT_NE(Float64, nullptr);
      EXPECT_EQ(String->type(), Case.Types->getStringType());
      EXPECT_EQ(Float32->type(), Case.Types->getFloat32Type());
      EXPECT_EQ(Float64->type(), Case.Types->getFloat64Type());
      EXPECT_EQ(String->type()->size(), Case.Types->targetContext().pointerByteWidth() * 2);
      EXPECT_EQ(Float64->type()->alignment(), Case.Types->targetContext().floatAlignment(64));
      EXPECT_TRUE(Case.Constants->owns(String));
      EXPECT_TRUE(Case.Constants->owns(Float32));
      EXPECT_TRUE(Case.Constants->owns(Float64));
      for (const ContextCase &OtherCase : Cases)
      {
        if (Case.Constants == OtherCase.Constants)
        {
          continue;
        }
        EXPECT_NE(String, OtherCase.Constants->getStringConstant("same text"));
        EXPECT_NE(Float32, OtherCase.Constants->getFloatConstant(1.25f));
        EXPECT_NE(Float64, OtherCase.Constants->getFloatConstant(1.25));
        EXPECT_FALSE(OtherCase.Constants->owns(String));
        EXPECT_FALSE(OtherCase.Constants->owns(Float32));
        EXPECT_FALSE(OtherCase.Constants->owns(Float64));
      }
    }
  }

  // Array identity includes the array type and ordered element list, including empty and nested arrays.
  TEST(IRConstantContextTest, ArrayLookupsReuseCompleteOrderedKeys)
  {
    TypeContext Types(Target);
    ConstantContext Constants(Types);
    const BoolConstant *True = Constants.getBoolConstant(true);
    const BoolConstant *False = Constants.getBoolConstant(false);
    const ArrayType *PairType = Types.getArrayType(Types.getBoolType(), 2);
    const ArrayConstant *Forward = Constants.getArrayConstant(PairType, {True, False});
    const ArrayConstant *Reverse = Constants.getArrayConstant(PairType, {False, True});
    ASSERT_NE(Forward, nullptr);
    ASSERT_NE(Reverse, nullptr);
    EXPECT_EQ(Forward, Constants.getArrayConstant(PairType, {True, False}));
    EXPECT_NE(Forward, Reverse);
    EXPECT_NE(Forward, Constants.getArrayConstant(PairType, {True, True}));
    EXPECT_EQ(Forward->elements()[0], True);
    EXPECT_EQ(Forward->elements()[1], False);
    const ArrayType *EmptyBoolType = Types.getArrayType(Types.getBoolType(), 0);
    const ArrayType *EmptyIntegerType = Types.getArrayType(Types.getInt32Type(), 0);
    const ArrayConstant *EmptyBool = Constants.getArrayConstant(EmptyBoolType);
    const ArrayConstant *EmptyInteger = Constants.getArrayConstant(EmptyIntegerType);
    ASSERT_NE(EmptyBool, nullptr);
    ASSERT_NE(EmptyInteger, nullptr);
    EXPECT_EQ(EmptyBool, Constants.getArrayConstant(EmptyBoolType, {}));
    EXPECT_NE(EmptyBool, EmptyInteger);
    const ArrayType *NestedType = Types.getArrayType(PairType, 2);
    const ArrayConstant *Nested = Constants.getArrayConstant(NestedType, {Forward, Reverse});
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(Nested, Constants.getArrayConstant(NestedType, {Forward, Reverse}));
    EXPECT_NE(Nested, Constants.getArrayConstant(NestedType, {Reverse, Forward}));
    EXPECT_TRUE(Constants.owns(Forward));
    EXPECT_TRUE(Constants.owns(EmptyBool));
    EXPECT_TRUE(Constants.owns(Nested));
  }

  // Every typed lookup rejects missing types and types belonging to a different TypeContext.
  TEST(IRConstantContextTest, TypedLookupsRejectMissingAndForeignTypes)
  {
    const TypeContext Types(Target);
    ConstantContext Constants(Types);
    TypeContext Other(Target);
    EXPECT_EQ(Constants.getIntegerConstantFromBits(Other.getInt32Type(), 0), nullptr);
    EXPECT_EQ(Constants.getSignedIntegerConstant(Other.getInt32Type(), 0), nullptr);
    EXPECT_EQ(Constants.getUnsignedIntegerConstant(Other.getUInt32Type(), 0), nullptr);
    EXPECT_EQ(Constants.getFloatConstantFromBits(Other.getFloat32Type(), 0), nullptr);
    EXPECT_EQ(Constants.getNullPointerConstant(Other.getPointerType(Other.getBoolType())), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(Other.getArrayType(Other.getBoolType(), 0)), nullptr);
    EXPECT_EQ(Constants.getIntegerConstantFromBits(nullptr, 0), nullptr);
    EXPECT_EQ(Constants.getSignedIntegerConstant(nullptr, 0), nullptr);
    EXPECT_EQ(Constants.getUnsignedIntegerConstant(nullptr, 0), nullptr);
    EXPECT_EQ(Constants.getFloatConstantFromBits(nullptr, 0), nullptr);
    EXPECT_EQ(Constants.getNullPointerConstant(nullptr), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(nullptr), nullptr);
  }

  // Cached arrays still reject foreign element ownership even when both constant contexts share types.
  TEST(IRConstantContextTest, ArraysRejectOtherConstantContextsWithSharedTypes)
  {
    TypeContext Types(Target);
    ConstantContext Constants(Types);
    ConstantContext Other(Types);
    const BoolConstant *True = Constants.getBoolConstant(true);
    const BoolConstant *False = Constants.getBoolConstant(false);
    const BoolConstant *OtherTrue = Other.getBoolConstant(true);
    const BoolConstant *OtherFalse = Other.getBoolConstant(false);
    EXPECT_EQ(True->type(), OtherTrue->type());
    EXPECT_FALSE(Constants.owns(OtherTrue));
    EXPECT_FALSE(Other.owns(True));
    const ArrayType *PairType = Types.getArrayType(Types.getBoolType(), 2);
    const ArrayConstant *Pair = Constants.getArrayConstant(PairType, {True, False});
    const ArrayConstant *OtherPair = Other.getArrayConstant(PairType, {OtherTrue, OtherFalse});
    ASSERT_NE(Pair, nullptr);
    ASSERT_NE(OtherPair, nullptr);
    EXPECT_EQ(Constants.getArrayConstant(PairType, {OtherTrue, OtherFalse}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(PairType, {True, OtherFalse}), nullptr);
    EXPECT_EQ(Other.getArrayConstant(PairType, {True, False}), nullptr);
    const ArrayType *NestedType = Types.getArrayType(PairType, 1);
    ASSERT_NE(Constants.getArrayConstant(NestedType, {Pair}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(NestedType, {OtherPair}), nullptr);
  }
} // namespace ink::ir::test
