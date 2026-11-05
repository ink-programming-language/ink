#include "ink/ir/constant/constant.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/constant/float_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/string_constant.h"
#include "ink/ir/type/array_type.h"
#include "ink/ir/type/float_type.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/type_pool.h"
#include "ink/ir/value/value.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string_view>
#include <type_traits>

namespace ink::ir::test
{
  static_assert(std::is_base_of_v<Value, Constant>);
  static_assert(std::is_base_of_v<Constant, IntegerConstant>);
  static_assert(std::is_base_of_v<Constant, FloatConstant>);
  static_assert(std::is_base_of_v<Constant, StringConstant>);
  static_assert(std::is_convertible_v<const IntegerConstant *, const Constant *>);
  static_assert(std::is_convertible_v<const FloatConstant *, const Constant *>);
  static_assert(std::is_convertible_v<const StringConstant *, const Constant *>);
  static_assert(std::is_convertible_v<const Constant *, const Value *>);
  static_assert(!std::is_constructible_v<Constant, const Type &>);
  static_assert(!std::is_destructible_v<Constant>);
  static_assert(!std::has_virtual_destructor_v<Constant>);
  static_assert(std::is_destructible_v<IntegerConstant>);
  static_assert(std::is_destructible_v<FloatConstant>);
  static_assert(std::is_destructible_v<StringConstant>);
  static_assert(!std::is_copy_constructible_v<Constant>);
  static_assert(!std::is_copy_assignable_v<Constant>);
  static_assert(!std::is_move_constructible_v<Constant>);
  static_assert(!std::is_move_assignable_v<Constant>);
  static_assert(!std::is_copy_constructible_v<IntegerConstant>);
  static_assert(!std::is_copy_assignable_v<IntegerConstant>);
  static_assert(!std::is_move_constructible_v<IntegerConstant>);
  static_assert(!std::is_move_assignable_v<IntegerConstant>);
  static_assert(!std::is_copy_constructible_v<FloatConstant>);
  static_assert(!std::is_copy_assignable_v<FloatConstant>);
  static_assert(!std::is_move_constructible_v<FloatConstant>);
  static_assert(!std::is_move_assignable_v<FloatConstant>);
  static_assert(!std::is_copy_constructible_v<StringConstant>);
  static_assert(!std::is_copy_assignable_v<StringConstant>);
  static_assert(!std::is_move_constructible_v<StringConstant>);
  static_assert(!std::is_move_assignable_v<StringConstant>);
  static_assert(!std::is_constructible_v<IntegerConstant, IntegerType &&, std::uint64_t>);
  static_assert(!std::is_constructible_v<IntegerConstant, const IntegerType &&, std::uint64_t>);
  static_assert(!std::is_constructible_v<FloatConstant, FloatType &&, std::uint64_t>);
  static_assert(!std::is_constructible_v<FloatConstant, const FloatType &&, std::uint64_t>);
  static_assert(!std::is_constructible_v<StringConstant, ArrayType &&, std::string_view>);
  static_assert(!std::is_constructible_v<StringConstant, const ArrayType &&, std::string_view>);

  namespace
  {
    constexpr IntegerType CompileTimeIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Signed);
    constexpr FloatType CompileTimeFloatType(FloatType::Width::Bits64);
    constexpr IntegerConstant CompileTimeInteger(CompileTimeIntegerType, 255);
    constexpr FloatConstant CompileTimeFloat(CompileTimeFloatType, std::uint64_t{0x3ff0000000000000});
    constexpr const Constant *CompileTimeIntegerBase = &CompileTimeInteger;
    constexpr const Constant *CompileTimeFloatBase = &CompileTimeFloat;

    static_assert(CompileTimeInteger.bits() == 255);
    static_assert(CompileTimeInteger.signedValue() == -1);
    static_assert(CompileTimeFloat.bits() == std::uint64_t{0x3ff0000000000000});
    static_assert(&CompileTimeIntegerBase->getType() == &CompileTimeIntegerType);
    static_assert(&CompileTimeFloatBase->getType() == &CompileTimeFloatType);
  } // namespace

  // Each concrete constant exposes its exact borrowed type through the common Constant and Value bases.
  TEST(IRConstantTest, PreservesConcreteTypeIdentityThroughCommonBases)
  {
    const IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const FloatType Float(FloatType::Width::Bits32);
    const IntegerType Byte(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned);
    const ArrayType Bytes(Byte, 3);
    const IntegerConstant IntegerValue(Integer, 42);
    const FloatConstant FloatValue(Float, std::uint64_t{0x3f800000});
    const StringConstant StringValue(Bytes, "ink");
    struct Case
    {
        const Constant *ConstantValue;
        const Type *ExpectedType;
    };
    const Case Cases[] = {
        {&IntegerValue, &Integer},
        {&FloatValue, &Float},
        {&StringValue, &Bytes},
    };
    for (const auto &Entry : Cases)
    {
      const Value *ValueBase = Entry.ConstantValue;
      EXPECT_EQ(&Entry.ConstantValue->getType(), Entry.ExpectedType);
      EXPECT_EQ(&ValueBase->getType(), Entry.ExpectedType);
    }
    EXPECT_EQ(IntegerValue.signedValue(), 42);
    EXPECT_EQ(FloatValue.value(), 1.0);
    EXPECT_EQ(StringValue.value(), "ink");
  }

  // Interned concrete constants retain canonical type and object identities when viewed as common constants.
  TEST(IRConstantTest, PreservesPoolReuseThroughCommonConstantPointers)
  {
    const IntegerType ExternalInteger(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const FloatType ExternalFloat(FloatType::Width::Bits64);
    TypePool Types;
    ConstantPool Constants(Types);
    const auto &Integer = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const auto &Float = Types.getFloatType(FloatType::Width::Bits64);
    const auto *IntegerValue = Constants.getInteger(ExternalInteger, 42);
    const auto *FloatValue = Constants.getFloatBits(ExternalFloat, std::uint64_t{0x3ff0000000000000});
    const auto &StringValue = Constants.getString("ink");
    ASSERT_NE(IntegerValue, nullptr);
    ASSERT_NE(FloatValue, nullptr);
    const Constant *IntegerBase = IntegerValue;
    const Constant *FloatBase = FloatValue;
    const Constant *StringBase = &StringValue;
    EXPECT_EQ(IntegerBase, Constants.getInteger(Integer, 42));
    EXPECT_EQ(FloatBase, &Constants.getFloat(1.0));
    EXPECT_EQ(StringBase, &Constants.getString("ink"));
    EXPECT_EQ(&IntegerBase->getType(), &Integer);
    EXPECT_EQ(&FloatBase->getType(), &Float);
    const auto &Byte = Types.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned);
    EXPECT_EQ(&StringBase->getType(), &Types.getArrayType(Byte, 3));
    const Value *Values[] = {
        IntegerBase,
        FloatBase,
        StringBase,
    };
    EXPECT_EQ(&Values[0]->getType(), &Integer);
    EXPECT_EQ(&Values[1]->getType(), &Float);
    EXPECT_EQ(&Values[2]->getType(), &StringValue.getType());
  }
} // namespace ink::ir::test
