#include "ink/target/target_context.h"
#include "ink/core/numeric/integer_bits.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/module/module.h"

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
    constexpr bool HasPoolOwnedIdentity = !std::is_default_constructible_v<ConstantType> && !std::is_copy_constructible_v<ConstantType> && !std::is_move_constructible_v<ConstantType> && !std::is_copy_assignable_v<ConstantType> && !std::is_move_assignable_v<ConstantType>;

    static_assert(!std::is_copy_constructible_v<ConstantPool>);
    static_assert(!std::is_move_constructible_v<ConstantPool>);
    static_assert(!std::is_copy_assignable_v<ConstantPool>);
    static_assert(!std::is_move_assignable_v<ConstantPool>);
    static_assert(!std::is_constructible_v<ConstantPool, IRContext &>);
    static_assert(!std::is_constructible_v<ConstantPool, const IRContext &>);
    static_assert(!std::is_destructible_v<ConstantPool>);
    static_assert(!std::is_constructible_v<ConstantPool, IRContext &&>);
    static_assert(!std::is_constructible_v<ConstantPool, const IRContext &&>);
    static_assert(std::is_same_v<decltype(std::declval<const ConstantPool &>().get<ink::ir::BoolConstant>(true)), const BoolConstant *>);
    static_assert(noexcept(std::declval<const ConstantPool &>().get<ink::ir::BoolConstant>(true)));
    static_assert(std::is_same_v<decltype(std::declval<const ConstantPool &>().get<ink::ir::NullPointerConstant>()), const NullPointerConstant *>);
    static_assert(noexcept(std::declval<const ConstantPool &>().get<ink::ir::NullPointerConstant>()));

    static_assert(HasPoolOwnedIdentity<BoolConstant>);
    static_assert(HasPoolOwnedIdentity<IntegerConstant>);
    static_assert(HasPoolOwnedIdentity<FloatConstant>);
    static_assert(HasPoolOwnedIdentity<NullPointerConstant>);
    static_assert(HasPoolOwnedIdentity<StringConstant>);
    static_assert(HasPoolOwnedIdentity<ArrayConstant>);
    static_assert(!std::is_constructible_v<BoolConstant, const BoolType *, bool>);
    static_assert(!std::is_constructible_v<IntegerConstant, const IntegerType *, std::uint64_t>);
    static_assert(!std::is_constructible_v<FloatConstant, const FloatType *, std::uint64_t>);
    static_assert(!std::is_constructible_v<NullPointerConstant, const NullPointerType *>);
    static_assert(!std::is_constructible_v<StringConstant, const ArrayType *, std::string>);
    static_assert(!std::is_constructible_v<ArrayConstant, const ArrayType *, std::vector<const Constant *>>);
  } // namespace

  // Each constant pool has exactly two canonical booleans and recognizes only its own constants.
  TEST(IRConstantPoolTest, BooleanLookupsRetainPoolOwnership)
  {
    IRContext TypesContext(Target);
    const Module Types(TypesContext);
    IRContext OtherTypesContext(Target);
    const Module OtherTypes(OtherTypesContext);
    const auto &Constants = Types.constantPool();
    const auto &Other = OtherTypes.constantPool();
    const BoolConstant *True = Constants.get<ink::ir::BoolConstant>(true);
    const BoolConstant *False = Constants.get<ink::ir::BoolConstant>(false);
    ASSERT_NE(True, nullptr);
    ASSERT_NE(False, nullptr);
    EXPECT_EQ(&Constants.irContext(), &Types.irContext());
    EXPECT_EQ(&Constants, &Types.constantPool());
    EXPECT_EQ(True, Constants.get<ink::ir::BoolConstant>(true));
    EXPECT_EQ(False, Constants.get<ink::ir::BoolConstant>(false));
    EXPECT_NE(True, False);
    EXPECT_TRUE(True->value());
    EXPECT_FALSE(False->value());
    EXPECT_EQ(True->type(), Types.typePool().get<ink::ir::BoolType>());
    EXPECT_EQ(False->type(), Types.typePool().get<ink::ir::BoolType>());
    EXPECT_TRUE(Constants.owns(True));
    EXPECT_TRUE(Constants.owns(False));
    EXPECT_FALSE(Constants.owns(nullptr));
    EXPECT_FALSE(Constants.owns(Other.get<ink::ir::BoolConstant>(true)));
    EXPECT_NE(True, Other.get<ink::ir::BoolConstant>(true));
  }

  // Numeric integer lookups normalize to raw bits while signedness and width remain part of identity.
  TEST(IRConstantPoolTest, IntegerLookupsReuseNormalizedBitsPerType)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const IntegerConstant *Signed32 = Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(-1));
    const IntegerConstant *Unsigned32 = Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, false), static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()));
    const IntegerConstant *Signed64 = Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(64, true), static_cast<std::int64_t>(-1));
    ASSERT_NE(Signed32, nullptr);
    ASSERT_NE(Unsigned32, nullptr);
    ASSERT_NE(Signed64, nullptr);
    const auto Bits32 = core::IntegerBits::fromUnsigned(32, 0xffffffffULL);
    const auto Bits64 = core::IntegerBits::fromUnsigned(64, 0xffffffffffffffffULL);
    ASSERT_TRUE(Bits32.has_value());
    ASSERT_TRUE(Bits64.has_value());
    EXPECT_EQ(Signed32, Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), *Bits32));
    EXPECT_EQ(Unsigned32, Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, false), *Bits32));
    EXPECT_EQ(Signed64, Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(64, true), *Bits64));
    EXPECT_NE(Signed32, Unsigned32);
    EXPECT_NE(Signed32, Signed64);
    EXPECT_NE(Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0)), Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(64, true), static_cast<std::int64_t>(0)));
    EXPECT_NE(Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0)), Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, false), static_cast<std::uint64_t>(0)));
    EXPECT_NE(Signed32, Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(1)));
    EXPECT_EQ(Signed32->bits().tryGetUInt64(), 0xffffffffULL);
    EXPECT_TRUE(Constants.owns(Signed32));
    EXPECT_TRUE(Constants.owns(Unsigned32));
    EXPECT_TRUE(Constants.owns(Signed64));
  }

  // Numeric floating-point lookups infer the local width and share exact-bit identity without collapsing signed zero or NaN payloads.
  TEST(IRConstantPoolTest, FloatLookupsReuseExactBitsWithoutCollapsingSpecialValues)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const FloatConstant *Float32 = Constants.get<ink::ir::FloatConstant>(1.25f);
    const FloatConstant *Float64 = Constants.get<ink::ir::FloatConstant>(1.25);
    const FloatConstant *PositiveZero = Constants.get<ink::ir::FloatConstant>(0.0f);
    const FloatConstant *NegativeZero = Constants.get<ink::ir::FloatConstant>(-0.0f);
    const auto FirstNaNBits = core::IntegerBits::fromUnsigned(32, 0x7fc00001ULL);
    const auto SecondNaNBits = core::IntegerBits::fromUnsigned(32, 0x7fc00002ULL);
    const auto Float32Bits = core::IntegerBits::fromUnsigned(32, 0x3fa00000ULL);
    const auto Float64Bits = core::IntegerBits::fromUnsigned(64, 0x3ff4000000000000ULL);
    const auto PositiveZeroBits = core::IntegerBits::fromUnsigned(32, 0);
    const auto NegativeZeroBits = core::IntegerBits::fromUnsigned(32, 0x80000000ULL);
    ASSERT_TRUE(FirstNaNBits.has_value());
    ASSERT_TRUE(SecondNaNBits.has_value());
    ASSERT_TRUE(Float32Bits.has_value());
    ASSERT_TRUE(Float64Bits.has_value());
    ASSERT_TRUE(PositiveZeroBits.has_value());
    ASSERT_TRUE(NegativeZeroBits.has_value());
    const FloatConstant *FirstNaN = Constants.get<ink::ir::FloatConstant>(Types.typePool().get<ink::ir::FloatType>(32), *FirstNaNBits);
    const FloatConstant *SecondNaN = Constants.get<ink::ir::FloatConstant>(Types.typePool().get<ink::ir::FloatType>(32), *SecondNaNBits);
    ASSERT_NE(Float32, nullptr);
    ASSERT_NE(Float64, nullptr);
    ASSERT_NE(PositiveZero, nullptr);
    ASSERT_NE(NegativeZero, nullptr);
    ASSERT_NE(FirstNaN, nullptr);
    ASSERT_NE(SecondNaN, nullptr);
    EXPECT_EQ(Float32->type(), Types.typePool().get<ink::ir::FloatType>(32));
    EXPECT_EQ(Float64->type(), Types.typePool().get<ink::ir::FloatType>(64));
    EXPECT_NE(Float32, Float64);
    EXPECT_EQ(Float32, Constants.get<ink::ir::FloatConstant>(Types.typePool().get<ink::ir::FloatType>(32), *Float32Bits));
    EXPECT_EQ(Float64, Constants.get<ink::ir::FloatConstant>(Types.typePool().get<ink::ir::FloatType>(64), *Float64Bits));
    EXPECT_EQ(PositiveZero, Constants.get<ink::ir::FloatConstant>(Types.typePool().get<ink::ir::FloatType>(32), *PositiveZeroBits));
    EXPECT_EQ(NegativeZero, Constants.get<ink::ir::FloatConstant>(Types.typePool().get<ink::ir::FloatType>(32), *NegativeZeroBits));
    EXPECT_NE(PositiveZero, NegativeZero);
    EXPECT_NE(PositiveZero, Constants.get<ink::ir::FloatConstant>(0.0));
    EXPECT_EQ(FirstNaN, Constants.get<ink::ir::FloatConstant>(Types.typePool().get<ink::ir::FloatType>(32), *FirstNaNBits));
    EXPECT_NE(FirstNaN, SecondNaN);
    EXPECT_EQ(FirstNaN->bits(), 0x7fc00001ULL);
    EXPECT_EQ(SecondNaN->bits(), 0x7fc00002ULL);
    EXPECT_TRUE(Constants.owns(Float32));
    EXPECT_TRUE(Constants.owns(Float64));
  }

  // Every pool owns one null constant with its own literal type regardless of the destination pointer type.
  TEST(IRConstantPoolTest, NullPointerLookupUsesOnePoolOwnedLiteral)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext OtherTypesContext(Target);
    const Module OtherTypes(OtherTypesContext);
    const auto &Constants = Types.constantPool();
    const auto &Other = OtherTypes.constantPool();
    const PointerType *BoolPointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::BoolType>());
    const PointerType *IntegerPointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true));
    const NullPointerConstant *Null = Constants.get<ink::ir::NullPointerConstant>();
    ASSERT_NE(Null, nullptr);
    EXPECT_EQ(Null, Constants.get<ink::ir::NullPointerConstant>());
    EXPECT_EQ(Null->type(), Types.typePool().get<ink::ir::NullPointerType>());
    EXPECT_FALSE(PointerType::classof(Null->type()));
    EXPECT_TRUE(NullPointerType::classof(Null->type()));
    EXPECT_TRUE(Null->canInitialize(BoolPointer));
    EXPECT_TRUE(Null->canInitialize(IntegerPointer));
    EXPECT_TRUE(Constants.owns(Null));
    EXPECT_FALSE(Other.owns(Null));
    EXPECT_NE(Null, Other.get<ink::ir::NullPointerConstant>());
    EXPECT_NE(Null->type(), Other.get<ink::ir::NullPointerConstant>()->type());
    EXPECT_FALSE(Constants.owns(Other.get<ink::ir::NullPointerConstant>()));
  }

  // Strings infer local byte-array types, preserve exact UTF-8 identity and keep borrowed views valid across cache growth.
  TEST(IRConstantPoolTest, StringLookupsPreserveBytesAndStableViews)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const std::string EmbeddedNul("a\0b", 3);
    const StringConstant *BinarySafe = Constants.get<ink::ir::StringConstant>(EmbeddedNul);
    const StringConstant *Composed = Constants.get<ink::ir::StringConstant>("\xC3\xA9");
    const StringConstant *Decomposed = Constants.get<ink::ir::StringConstant>("e\xCC\x81");
    const std::string StableText(128, 'x');
    const StringConstant *Stable = Constants.get<ink::ir::StringConstant>(StableText);
    ASSERT_NE(BinarySafe, nullptr);
    ASSERT_NE(Composed, nullptr);
    ASSERT_NE(Decomposed, nullptr);
    ASSERT_NE(Stable, nullptr);
    const std::string_view View = Stable->value();
    const char *Data = View.data();
    EXPECT_EQ(BinarySafe->type(), Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(8, false), 3));
    EXPECT_EQ(Stable->type(), Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(8, false), 128));
    EXPECT_EQ(BinarySafe, Constants.get<ink::ir::StringConstant>(std::string("a\0b", 3)));
    EXPECT_EQ(BinarySafe->value(), std::string_view(EmbeddedNul));
    EXPECT_NE(BinarySafe, Constants.get<ink::ir::StringConstant>("a"));
    EXPECT_EQ(Composed, Constants.get<ink::ir::StringConstant>("\xC3\xA9"));
    EXPECT_EQ(Decomposed, Constants.get<ink::ir::StringConstant>("e\xCC\x81"));
    EXPECT_NE(Composed, Decomposed);
    for (std::uint32_t Index = 0; Index < 256; ++Index)
    {
      ASSERT_NE(Constants.get<ink::ir::StringConstant>(std::to_string(Index)), nullptr);
      ASSERT_NE(Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, false), static_cast<std::uint64_t>(Index)), nullptr);
    }
    EXPECT_EQ(Stable, Constants.get<ink::ir::StringConstant>(StableText));
    EXPECT_EQ(View, StableText);
    EXPECT_EQ(Stable->value().data(), Data);
    EXPECT_TRUE(Constants.owns(BinarySafe));
    EXPECT_TRUE(Constants.owns(Stable));
  }

  // Byte-array string layouts are target-independent while floating-point layouts and constant identities belong to each context's pools.
  TEST(IRConstantPoolTest, InferredTypeLookupsRetainContextOwnershipAndTargetLayout)
  {
    constexpr target::TargetContext NarrowTarget(target::PointerWidth::Bits32, target::ByteOrder::BigEndian);
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext OtherTypesContext(Target);
    Module OtherTypes(OtherTypesContext);
    IRContext NarrowTypesContext(NarrowTarget);
    Module NarrowTypes(NarrowTypesContext);
    auto &Constants = Types.constantPool();
    auto &SharedTypes = Types.constantPool();
    auto &Other = OtherTypes.constantPool();
    auto &Narrow = NarrowTypes.constantPool();
    struct ModuleCase
    {
        Module *Types;
        ConstantPool *Constants;
    };
    const ModuleCase Cases[] = {
        {&Types, &Constants},
        {&OtherTypes, &Other},
        {&NarrowTypes, &Narrow},
    };
    EXPECT_EQ(&Constants, &SharedTypes);
    EXPECT_EQ(Constants.get<ink::ir::StringConstant>("same text"), SharedTypes.get<ink::ir::StringConstant>("same text"));
    EXPECT_EQ(Constants.get<ink::ir::FloatConstant>(1.25f), SharedTypes.get<ink::ir::FloatConstant>(1.25f));
    EXPECT_EQ(Constants.get<ink::ir::FloatConstant>(1.25), SharedTypes.get<ink::ir::FloatConstant>(1.25));
    for (const ModuleCase &Case : Cases)
    {
      const auto String = Case.Constants->get<ink::ir::StringConstant>("same text");
      const auto Float32 = Case.Constants->get<ink::ir::FloatConstant>(1.25f);
      const auto Float64 = Case.Constants->get<ink::ir::FloatConstant>(1.25);
      ASSERT_NE(String, nullptr);
      ASSERT_NE(Float32, nullptr);
      ASSERT_NE(Float64, nullptr);
      EXPECT_EQ(String->type(), Case.Types->typePool().get<ink::ir::ArrayType>(Case.Types->typePool().get<ink::ir::IntegerType>(8, false), 9));
      EXPECT_EQ(Float32->type(), Case.Types->typePool().get<ink::ir::FloatType>(32));
      EXPECT_EQ(Float64->type(), Case.Types->typePool().get<ink::ir::FloatType>(64));
      EXPECT_EQ(String->type()->size(), 9U);
      EXPECT_EQ(String->type()->alignment(), 1U);
      EXPECT_EQ(Float64->type()->alignment(), Case.Types->targetContext().floatAlignment(64));
      EXPECT_TRUE(Case.Constants->owns(String));
      EXPECT_TRUE(Case.Constants->owns(Float32));
      EXPECT_TRUE(Case.Constants->owns(Float64));
      for (const ModuleCase &OtherCase : Cases)
      {
        if (Case.Constants == OtherCase.Constants)
        {
          continue;
        }
        EXPECT_NE(String, OtherCase.Constants->get<ink::ir::StringConstant>("same text"));
        EXPECT_NE(Float32, OtherCase.Constants->get<ink::ir::FloatConstant>(1.25f));
        EXPECT_NE(Float64, OtherCase.Constants->get<ink::ir::FloatConstant>(1.25));
        EXPECT_FALSE(OtherCase.Constants->owns(String));
        EXPECT_FALSE(OtherCase.Constants->owns(Float32));
        EXPECT_FALSE(OtherCase.Constants->owns(Float64));
      }
    }
  }

  // Array identity includes the array type and ordered element list, including empty and nested arrays.
  TEST(IRConstantPoolTest, ArrayLookupsReuseCompleteOrderedKeys)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const BoolConstant *True = Constants.get<ink::ir::BoolConstant>(true);
    const BoolConstant *False = Constants.get<ink::ir::BoolConstant>(false);
    const ArrayType *PairType = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::BoolType>(), 2);
    const ArrayConstant *Forward = Constants.get<ink::ir::ArrayConstant>(PairType, {True, False});
    const ArrayConstant *Reverse = Constants.get<ink::ir::ArrayConstant>(PairType, {False, True});
    ASSERT_NE(Forward, nullptr);
    ASSERT_NE(Reverse, nullptr);
    EXPECT_EQ(Forward, Constants.get<ink::ir::ArrayConstant>(PairType, {True, False}));
    EXPECT_NE(Forward, Reverse);
    EXPECT_NE(Forward, Constants.get<ink::ir::ArrayConstant>(PairType, {True, True}));
    EXPECT_EQ(Forward->elements()[0], True);
    EXPECT_EQ(Forward->elements()[1], False);
    const ArrayType *EmptyBoolType = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::BoolType>(), 0);
    const ArrayType *EmptyIntegerType = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(32, true), 0);
    const ArrayConstant *EmptyBool = Constants.get<ink::ir::ArrayConstant>(EmptyBoolType);
    const ArrayConstant *EmptyInteger = Constants.get<ink::ir::ArrayConstant>(EmptyIntegerType);
    ASSERT_NE(EmptyBool, nullptr);
    ASSERT_NE(EmptyInteger, nullptr);
    EXPECT_EQ(EmptyBool, Constants.get<ink::ir::ArrayConstant>(EmptyBoolType, {}));
    EXPECT_NE(EmptyBool, EmptyInteger);
    const ArrayType *NestedType = Types.typePool().get<ink::ir::ArrayType>(PairType, 2);
    const ArrayConstant *Nested = Constants.get<ink::ir::ArrayConstant>(NestedType, {Forward, Reverse});
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(Nested, Constants.get<ink::ir::ArrayConstant>(NestedType, {Forward, Reverse}));
    EXPECT_NE(Nested, Constants.get<ink::ir::ArrayConstant>(NestedType, {Reverse, Forward}));
    EXPECT_TRUE(Constants.owns(Forward));
    EXPECT_TRUE(Constants.owns(EmptyBool));
    EXPECT_TRUE(Constants.owns(Nested));
  }

  // Every typed lookup rejects missing types and types belonging to a different IRContext.
  TEST(IRConstantPoolTest, TypedLookupsRejectMissingAndForeignTypes)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    IRContext OtherContext(Target);
    Module Other(OtherContext);
    const auto ZeroBits = core::IntegerBits::fromUnsigned(32, 0);
    ASSERT_TRUE(ZeroBits.has_value());
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Other.typePool().get<ink::ir::IntegerType>(32, true), *ZeroBits), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Other.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0)), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Other.typePool().get<ink::ir::IntegerType>(32, false), static_cast<std::uint64_t>(0)), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::FloatConstant>(Other.typePool().get<ink::ir::FloatType>(32), *ZeroBits), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(Other.typePool().get<ink::ir::ArrayType>(Other.typePool().get<ink::ir::BoolType>(), 0)), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(nullptr, *ZeroBits), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(nullptr, static_cast<std::int64_t>(0)), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(nullptr, static_cast<std::uint64_t>(0)), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::FloatConstant>(nullptr, *ZeroBits), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(nullptr), nullptr);
  }

  // Cached arrays reject foreign elements and nested arrays from a different IRContext.
  TEST(IRConstantPoolTest, ArraysRejectConstantsFromOtherContexts)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext OtherTypesContext(Target);
    Module OtherTypes(OtherTypesContext);
    auto &Constants = Types.constantPool();
    auto &Other = OtherTypes.constantPool();
    const BoolConstant *True = Constants.get<ink::ir::BoolConstant>(true);
    const BoolConstant *False = Constants.get<ink::ir::BoolConstant>(false);
    const BoolConstant *OtherTrue = Other.get<ink::ir::BoolConstant>(true);
    const BoolConstant *OtherFalse = Other.get<ink::ir::BoolConstant>(false);
    EXPECT_NE(True->type(), OtherTrue->type());
    EXPECT_FALSE(Constants.owns(OtherTrue));
    EXPECT_FALSE(Other.owns(True));
    const ArrayType *PairType = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::BoolType>(), 2);
    const ArrayType *OtherPairType = OtherTypes.typePool().get<ink::ir::ArrayType>(OtherTypes.typePool().get<ink::ir::BoolType>(), 2);
    const ArrayConstant *Pair = Constants.get<ink::ir::ArrayConstant>(PairType, {True, False});
    const ArrayConstant *OtherPair = Other.get<ink::ir::ArrayConstant>(OtherPairType, {OtherTrue, OtherFalse});
    ASSERT_NE(Pair, nullptr);
    ASSERT_NE(OtherPair, nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(PairType, {OtherTrue, OtherFalse}), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(PairType, {True, OtherFalse}), nullptr);
    EXPECT_EQ(Other.get<ink::ir::ArrayConstant>(OtherPairType, {True, False}), nullptr);
    const ArrayType *NestedType = Types.typePool().get<ink::ir::ArrayType>(PairType, 1);
    ASSERT_NE(Constants.get<ink::ir::ArrayConstant>(NestedType, {Pair}), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(NestedType, {OtherPair}), nullptr);
  }
} // namespace ink::ir::test
