#include "ink/target/target_context.h"
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
    static_assert(std::is_same_v<decltype(std::declval<const ConstantPool &>().getBoolConstant(true)), const BoolConstant *>);
    static_assert(noexcept(std::declval<const ConstantPool &>().getBoolConstant(true)));
    static_assert(std::is_same_v<decltype(std::declval<const ConstantPool &>().getNullPointerConstant()), const NullPointerConstant *>);
    static_assert(noexcept(std::declval<const ConstantPool &>().getNullPointerConstant()));

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
    const BoolConstant *True = Constants.getBoolConstant(true);
    const BoolConstant *False = Constants.getBoolConstant(false);
    ASSERT_NE(True, nullptr);
    ASSERT_NE(False, nullptr);
    EXPECT_EQ(&Constants.irContext(), &Types.irContext());
    EXPECT_EQ(&Constants, &Types.constantPool());
    EXPECT_EQ(True, Constants.getBoolConstant(true));
    EXPECT_EQ(False, Constants.getBoolConstant(false));
    EXPECT_NE(True, False);
    EXPECT_TRUE(True->value());
    EXPECT_FALSE(False->value());
    EXPECT_EQ(True->type(), Types.typePool().getBoolType());
    EXPECT_EQ(False->type(), Types.typePool().getBoolType());
    EXPECT_TRUE(Constants.owns(True));
    EXPECT_TRUE(Constants.owns(False));
    EXPECT_FALSE(Constants.owns(nullptr));
    EXPECT_FALSE(Constants.owns(Other.getBoolConstant(true)));
    EXPECT_NE(True, Other.getBoolConstant(true));
  }

  // Numeric integer lookups normalize to raw bits while signedness and width remain part of identity.
  TEST(IRConstantPoolTest, IntegerLookupsReuseNormalizedBitsPerType)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const IntegerConstant *Signed32 = Constants.getSignedIntegerConstant(Types.typePool().getInt32Type(), -1);
    const IntegerConstant *Unsigned32 = Constants.getUnsignedIntegerConstant(Types.typePool().getUInt32Type(), std::numeric_limits<std::uint32_t>::max());
    const IntegerConstant *Signed64 = Constants.getSignedIntegerConstant(Types.typePool().getInt64Type(), -1);
    ASSERT_NE(Signed32, nullptr);
    ASSERT_NE(Unsigned32, nullptr);
    ASSERT_NE(Signed64, nullptr);
    EXPECT_EQ(Signed32, Constants.getIntegerConstantFromBits(Types.typePool().getInt32Type(), 0xffffffffULL));
    EXPECT_EQ(Unsigned32, Constants.getIntegerConstantFromBits(Types.typePool().getUInt32Type(), 0xffffffffULL));
    EXPECT_EQ(Signed64, Constants.getIntegerConstantFromBits(Types.typePool().getInt64Type(), 0xffffffffffffffffULL));
    EXPECT_NE(Signed32, Unsigned32);
    EXPECT_NE(Signed32, Signed64);
    EXPECT_NE(Constants.getSignedIntegerConstant(Types.typePool().getInt32Type(), 0), Constants.getSignedIntegerConstant(Types.typePool().getInt64Type(), 0));
    EXPECT_NE(Constants.getSignedIntegerConstant(Types.typePool().getInt32Type(), 0), Constants.getUnsignedIntegerConstant(Types.typePool().getUInt32Type(), 0));
    EXPECT_NE(Signed32, Constants.getSignedIntegerConstant(Types.typePool().getInt32Type(), 1));
    EXPECT_EQ(Signed32->bits(), 0xffffffffULL);
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
    const FloatConstant *Float32 = Constants.getFloatConstant(1.25f);
    const FloatConstant *Float64 = Constants.getFloatConstant(1.25);
    const FloatConstant *PositiveZero = Constants.getFloatConstant(0.0f);
    const FloatConstant *NegativeZero = Constants.getFloatConstant(-0.0f);
    const FloatConstant *FirstNaN = Constants.getFloatConstantFromBits(Types.typePool().getFloat32Type(), 0x7fc00001ULL);
    const FloatConstant *SecondNaN = Constants.getFloatConstantFromBits(Types.typePool().getFloat32Type(), 0x7fc00002ULL);
    ASSERT_NE(Float32, nullptr);
    ASSERT_NE(Float64, nullptr);
    ASSERT_NE(PositiveZero, nullptr);
    ASSERT_NE(NegativeZero, nullptr);
    ASSERT_NE(FirstNaN, nullptr);
    ASSERT_NE(SecondNaN, nullptr);
    EXPECT_EQ(Float32->type(), Types.typePool().getFloat32Type());
    EXPECT_EQ(Float64->type(), Types.typePool().getFloat64Type());
    EXPECT_NE(Float32, Float64);
    EXPECT_EQ(Float32, Constants.getFloatConstantFromBits(Types.typePool().getFloat32Type(), 0x3fa00000ULL));
    EXPECT_EQ(Float64, Constants.getFloatConstantFromBits(Types.typePool().getFloat64Type(), 0x3ff4000000000000ULL));
    EXPECT_EQ(PositiveZero, Constants.getFloatConstantFromBits(Types.typePool().getFloat32Type(), 0));
    EXPECT_EQ(NegativeZero, Constants.getFloatConstantFromBits(Types.typePool().getFloat32Type(), 0x80000000ULL));
    EXPECT_NE(PositiveZero, NegativeZero);
    EXPECT_NE(PositiveZero, Constants.getFloatConstant(0.0));
    EXPECT_EQ(FirstNaN, Constants.getFloatConstantFromBits(Types.typePool().getFloat32Type(), 0x7fc00001ULL));
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
    const PointerType *BoolPointer = Types.typePool().getPointerType(Types.typePool().getBoolType());
    const PointerType *IntegerPointer = Types.typePool().getPointerType(Types.typePool().getInt32Type());
    const NullPointerConstant *Null = Constants.getNullPointerConstant();
    ASSERT_NE(Null, nullptr);
    EXPECT_EQ(Null, Constants.getNullPointerConstant());
    EXPECT_EQ(Null->type(), Types.typePool().getNullPointerType());
    EXPECT_FALSE(PointerType::classof(Null->type()));
    EXPECT_TRUE(NullPointerType::classof(Null->type()));
    EXPECT_TRUE(Null->canInitialize(BoolPointer));
    EXPECT_TRUE(Null->canInitialize(IntegerPointer));
    EXPECT_TRUE(Constants.owns(Null));
    EXPECT_FALSE(Other.owns(Null));
    EXPECT_NE(Null, Other.getNullPointerConstant());
    EXPECT_NE(Null->type(), Other.getNullPointerConstant()->type());
    EXPECT_FALSE(Constants.owns(Other.getNullPointerConstant()));
  }

  // Strings infer local byte-array types, preserve exact UTF-8 identity and keep borrowed views valid across cache growth.
  TEST(IRConstantPoolTest, StringLookupsPreserveBytesAndStableViews)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
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
    EXPECT_EQ(BinarySafe->type(), Types.typePool().getArrayType(Types.typePool().getUInt8Type(), 3));
    EXPECT_EQ(Stable->type(), Types.typePool().getArrayType(Types.typePool().getUInt8Type(), 128));
    EXPECT_EQ(BinarySafe, Constants.getStringConstant(std::string("a\0b", 3)));
    EXPECT_EQ(BinarySafe->value(), std::string_view(EmbeddedNul));
    EXPECT_NE(BinarySafe, Constants.getStringConstant("a"));
    EXPECT_EQ(Composed, Constants.getStringConstant("\xC3\xA9"));
    EXPECT_EQ(Decomposed, Constants.getStringConstant("e\xCC\x81"));
    EXPECT_NE(Composed, Decomposed);
    for (std::uint32_t Index = 0; Index < 256; ++Index)
    {
      ASSERT_NE(Constants.getStringConstant(std::to_string(Index)), nullptr);
      ASSERT_NE(Constants.getUnsignedIntegerConstant(Types.typePool().getUInt32Type(), Index), nullptr);
    }
    EXPECT_EQ(Stable, Constants.getStringConstant(StableText));
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
    EXPECT_EQ(Constants.getStringConstant("same text"), SharedTypes.getStringConstant("same text"));
    EXPECT_EQ(Constants.getFloatConstant(1.25f), SharedTypes.getFloatConstant(1.25f));
    EXPECT_EQ(Constants.getFloatConstant(1.25), SharedTypes.getFloatConstant(1.25));
    for (const ModuleCase &Case : Cases)
    {
      const auto String = Case.Constants->getStringConstant("same text");
      const auto Float32 = Case.Constants->getFloatConstant(1.25f);
      const auto Float64 = Case.Constants->getFloatConstant(1.25);
      ASSERT_NE(String, nullptr);
      ASSERT_NE(Float32, nullptr);
      ASSERT_NE(Float64, nullptr);
      EXPECT_EQ(String->type(), Case.Types->typePool().getArrayType(Case.Types->typePool().getUInt8Type(), 9));
      EXPECT_EQ(Float32->type(), Case.Types->typePool().getFloat32Type());
      EXPECT_EQ(Float64->type(), Case.Types->typePool().getFloat64Type());
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
  TEST(IRConstantPoolTest, ArrayLookupsReuseCompleteOrderedKeys)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const BoolConstant *True = Constants.getBoolConstant(true);
    const BoolConstant *False = Constants.getBoolConstant(false);
    const ArrayType *PairType = Types.typePool().getArrayType(Types.typePool().getBoolType(), 2);
    const ArrayConstant *Forward = Constants.getArrayConstant(PairType, {True, False});
    const ArrayConstant *Reverse = Constants.getArrayConstant(PairType, {False, True});
    ASSERT_NE(Forward, nullptr);
    ASSERT_NE(Reverse, nullptr);
    EXPECT_EQ(Forward, Constants.getArrayConstant(PairType, {True, False}));
    EXPECT_NE(Forward, Reverse);
    EXPECT_NE(Forward, Constants.getArrayConstant(PairType, {True, True}));
    EXPECT_EQ(Forward->elements()[0], True);
    EXPECT_EQ(Forward->elements()[1], False);
    const ArrayType *EmptyBoolType = Types.typePool().getArrayType(Types.typePool().getBoolType(), 0);
    const ArrayType *EmptyIntegerType = Types.typePool().getArrayType(Types.typePool().getInt32Type(), 0);
    const ArrayConstant *EmptyBool = Constants.getArrayConstant(EmptyBoolType);
    const ArrayConstant *EmptyInteger = Constants.getArrayConstant(EmptyIntegerType);
    ASSERT_NE(EmptyBool, nullptr);
    ASSERT_NE(EmptyInteger, nullptr);
    EXPECT_EQ(EmptyBool, Constants.getArrayConstant(EmptyBoolType, {}));
    EXPECT_NE(EmptyBool, EmptyInteger);
    const ArrayType *NestedType = Types.typePool().getArrayType(PairType, 2);
    const ArrayConstant *Nested = Constants.getArrayConstant(NestedType, {Forward, Reverse});
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(Nested, Constants.getArrayConstant(NestedType, {Forward, Reverse}));
    EXPECT_NE(Nested, Constants.getArrayConstant(NestedType, {Reverse, Forward}));
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
    EXPECT_EQ(Constants.getIntegerConstantFromBits(Other.typePool().getInt32Type(), 0), nullptr);
    EXPECT_EQ(Constants.getSignedIntegerConstant(Other.typePool().getInt32Type(), 0), nullptr);
    EXPECT_EQ(Constants.getUnsignedIntegerConstant(Other.typePool().getUInt32Type(), 0), nullptr);
    EXPECT_EQ(Constants.getFloatConstantFromBits(Other.typePool().getFloat32Type(), 0), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(Other.typePool().getArrayType(Other.typePool().getBoolType(), 0)), nullptr);
    EXPECT_EQ(Constants.getIntegerConstantFromBits(nullptr, 0), nullptr);
    EXPECT_EQ(Constants.getSignedIntegerConstant(nullptr, 0), nullptr);
    EXPECT_EQ(Constants.getUnsignedIntegerConstant(nullptr, 0), nullptr);
    EXPECT_EQ(Constants.getFloatConstantFromBits(nullptr, 0), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(nullptr), nullptr);
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
    const BoolConstant *True = Constants.getBoolConstant(true);
    const BoolConstant *False = Constants.getBoolConstant(false);
    const BoolConstant *OtherTrue = Other.getBoolConstant(true);
    const BoolConstant *OtherFalse = Other.getBoolConstant(false);
    EXPECT_NE(True->type(), OtherTrue->type());
    EXPECT_FALSE(Constants.owns(OtherTrue));
    EXPECT_FALSE(Other.owns(True));
    const ArrayType *PairType = Types.typePool().getArrayType(Types.typePool().getBoolType(), 2);
    const ArrayType *OtherPairType = OtherTypes.typePool().getArrayType(OtherTypes.typePool().getBoolType(), 2);
    const ArrayConstant *Pair = Constants.getArrayConstant(PairType, {True, False});
    const ArrayConstant *OtherPair = Other.getArrayConstant(OtherPairType, {OtherTrue, OtherFalse});
    ASSERT_NE(Pair, nullptr);
    ASSERT_NE(OtherPair, nullptr);
    EXPECT_EQ(Constants.getArrayConstant(PairType, {OtherTrue, OtherFalse}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(PairType, {True, OtherFalse}), nullptr);
    EXPECT_EQ(Other.getArrayConstant(OtherPairType, {True, False}), nullptr);
    const ArrayType *NestedType = Types.typePool().getArrayType(PairType, 1);
    ASSERT_NE(Constants.getArrayConstant(NestedType, {Pair}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(NestedType, {OtherPair}), nullptr);
  }
} // namespace ink::ir::test
