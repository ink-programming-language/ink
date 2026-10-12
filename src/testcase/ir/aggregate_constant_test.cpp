#include "ink/target/target_context.h"
#include "ink/ir/constant/array_constant.h"
#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/constant/null_pointer_constant.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <vector>

namespace ink::ir::test
{
  // An array owns its pointer list while preserving element order and borrowed type/constant identity.
  TEST(IRArrayConstantTest, OwnsElementListAndBorrowsValues)
  {
    const target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    TypeContext Types(Target);
    auto &Constants = Types.constantPool();
    const auto ElementType = Types.getBoolType();
    const auto First = Constants.getBoolConstant(true);
    const auto Second = Constants.getBoolConstant(false);
    const auto Array = Types.getArrayType(ElementType, 2);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Array, nullptr);
    std::vector<const Constant *> Elements = {First, Second};
    const auto Value = Constants.getArrayConstant(Array, Elements);
    ASSERT_NE(Value, nullptr);
    Elements.clear();
    EXPECT_EQ(Value->type(), Array);
    ASSERT_EQ(Value->elementCount(), 2U);
    EXPECT_EQ(Value->elements()[0], First);
    EXPECT_EQ(Value->elements()[1], Second);
    EXPECT_TRUE(ArrayConstant::classof(Value));
    EXPECT_TRUE(Constant::classof(Value));
    EXPECT_FALSE(Type::classof(Value));
    EXPECT_FALSE(ArrayConstant::classof(nullptr));
    EXPECT_FALSE(ArrayConstant::classof(First));
  }

  // Array creation rejects missing types, wrong lengths, null elements and distinct but structurally equal element types.
  TEST(IRArrayConstantTest, RejectsInvalidElementLists)
  {
    const target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    TypeContext Types(Target);
    auto &Constants = Types.constantPool();
    TypeContext OtherTypes(Target);
    const auto ElementType = Types.getBoolType();
    const auto OtherType = OtherTypes.getBoolType();
    const auto Element = Constants.getBoolConstant(true);
    const auto Array = Types.getArrayType(ElementType, 1);
    const auto OtherArray = OtherTypes.getArrayType(OtherType, 1);
    ASSERT_NE(Element, nullptr);
    ASSERT_NE(Array, nullptr);
    ASSERT_NE(OtherArray, nullptr);
    EXPECT_EQ(Constants.getArrayConstant(nullptr), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(Array), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(Array, {Element, Element}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(Array, {nullptr}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(OtherArray, {Element}), nullptr);
  }

  // Empty arrays and nested array constants preserve each level's declared type and element count.
  TEST(IRArrayConstantTest, SupportsEmptyAndNestedArrays)
  {
    const target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    TypeContext Types(Target);
    auto &Constants = Types.constantPool();
    const auto EmptyType = Types.getArrayType(Types.getBoolType(), 0);
    ASSERT_NE(EmptyType, nullptr);
    const auto Empty = Constants.getArrayConstant(EmptyType);
    ASSERT_NE(Empty, nullptr);
    EXPECT_TRUE(Empty->elements().empty());
    EXPECT_EQ(Empty->elementCount(), 0U);
    EXPECT_EQ(Empty->type(), EmptyType);
    const auto OuterType = Types.getArrayType(EmptyType, 2);
    ASSERT_NE(OuterType, nullptr);
    const auto Outer = Constants.getArrayConstant(OuterType, {Empty, Empty});
    ASSERT_NE(Outer, nullptr);
    EXPECT_EQ(Outer->type(), OuterType);
    ASSERT_EQ(Outer->elementCount(), 2U);
    EXPECT_EQ(Outer->elements()[0], Empty);
    EXPECT_EQ(Outer->elements()[1], Empty);
  }

  // Null constants retain their own target-sized literal type on both 32-bit and 64-bit targets without a host address.
  TEST(IRNullPointerConstantTest, PreservesTargetNullPointerTypes)
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::BigEndian);
    TypeContext Types32(Target32);
    TypeContext Types64(Target64);
    auto &Constants32 = Types32.constantPool();
    auto &Constants64 = Types64.constantPool();
    const auto Pointer32 = Types32.getPointerType(Types32.getBoolType());
    const auto Null32 = Constants32.getNullPointerConstant();
    const auto Null64 = Constants64.getNullPointerConstant();
    ASSERT_NE(Null32, nullptr);
    ASSERT_NE(Null64, nullptr);
    EXPECT_EQ(Null32->type(), Types32.getNullPointerType());
    EXPECT_EQ(Null64->type(), Types64.getNullPointerType());
    EXPECT_EQ(Null32->type()->size(), 4U);
    EXPECT_EQ(Null64->type()->size(), 8U);
    EXPECT_TRUE(NullPointerConstant::classof(Null32));
    EXPECT_TRUE(NullPointerConstant::classof(Null64));
    EXPECT_TRUE(Constant::classof(Null32));
    EXPECT_FALSE(Type::classof(Null32));
    EXPECT_FALSE(NullPointerConstant::classof(Pointer32));
    EXPECT_FALSE(NullPointerConstant::classof(nullptr));
  }

  // The canonical null constant exists independently of a destination and rejects a missing initialization type.
  TEST(IRNullPointerConstantTest, RequiresValidInitializationDestination)
  {
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    TypeContext Types(Target64);
    auto &Constants = Types.constantPool();
    const auto Null = Constants.getNullPointerConstant();
    ASSERT_NE(Null, nullptr);
    EXPECT_FALSE(Null->canInitialize(nullptr));
    EXPECT_TRUE(Null->canInitialize(Types.getNullPointerType()));
    EXPECT_EQ(Types.getPointerType(nullptr), nullptr);
  }
} // namespace ink::ir::test
