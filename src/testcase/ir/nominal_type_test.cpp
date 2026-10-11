#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    constexpr std::uint64_t MaximumSize = std::numeric_limits<std::uint64_t>::max();

    std::array<const Type *, 3> createNominalTypes(TypeContext &Types, std::uint64_t Size, std::uint64_t Alignment)
    {
      return {Types.createClassType(Size, Alignment), Types.createInterfaceType(Size, Alignment), Types.createStructType(Size, Alignment)};
    }
  } // namespace

  // Nominal types preserve explicit byte layouts, including empty and maximum-sized storage.
  TEST(IRNominalTypeTest, PreservesLayoutAndConcreteKinds)
  {
    TypeContext Types(Target64);
    constexpr ValueKind Kinds[] = {
        ValueKind::Class,
        ValueKind::Interface,
        ValueKind::Struct,
    };
    for (const std::uint64_t Size : {std::uint64_t{0}, std::uint64_t{5}, MaximumSize})
    {
      for (const std::uint64_t Alignment : {std::uint64_t{1}, std::uint64_t{8}, std::uint64_t{1} << 63})
      {
        const auto Values = createNominalTypes(Types, Size, Alignment);
        for (std::size_t Index = 0; Index < Values.size(); ++Index)
        {
          const Type *Value = Values[Index];
          ASSERT_NE(Value, nullptr);
          EXPECT_EQ(Value->kind(), Kinds[Index]);
          EXPECT_TRUE(Value->hasLayout());
          EXPECT_EQ(Value->size(), Size);
          EXPECT_EQ(Value->alignment(), Alignment);
          EXPECT_TRUE(Type::classof(Value));
          EXPECT_EQ(ClassType::classof(Value), Index == 0);
          EXPECT_EQ(InterfaceType::classof(Value), Index == 1);
          EXPECT_EQ(StructType::classof(Value), Index == 2);
        }
      }
    }
  }

  // Equal layouts create distinct nominal identities owned exclusively by their creating context.
  TEST(IRNominalTypeTest, EqualLayoutsHaveDistinctContextOwnedIdentities)
  {
    TypeContext Types(Target64);
    TypeContext OtherTypes(Target64);
    const auto First = createNominalTypes(Types, 8, 8);
    const auto Second = createNominalTypes(Types, 8, 8);
    const auto Foreign = createNominalTypes(OtherTypes, 8, 8);
    for (std::size_t Index = 0; Index < First.size(); ++Index)
    {
      ASSERT_NE(First[Index], nullptr);
      ASSERT_NE(Second[Index], nullptr);
      ASSERT_NE(Foreign[Index], nullptr);
      EXPECT_NE(First[Index], Second[Index]);
      EXPECT_NE(First[Index], Foreign[Index]);
      EXPECT_TRUE(Types.owns(First[Index]));
      EXPECT_TRUE(Types.owns(Second[Index]));
      EXPECT_FALSE(OtherTypes.owns(First[Index]));
      EXPECT_FALSE(Types.owns(Foreign[Index]));
      EXPECT_TRUE(OtherTypes.owns(Foreign[Index]));
      EXPECT_NE(Types.getPointerType(First[Index]), Types.getPointerType(Second[Index]));
      EXPECT_NE(Types.getReferenceType(First[Index]), Types.getReferenceType(Second[Index]));
      EXPECT_NE(Types.getArrayType(First[Index], 2), Types.getArrayType(Second[Index], 2));
      EXPECT_NE(Types.getFunctionType(First[Index], {First[Index]}), Types.getFunctionType(Second[Index], {Second[Index]}));
    }
  }

  // Every nominal type factory rejects zero and non-power-of-two alignments without creating a type.
  TEST(IRNominalTypeTest, RejectsInvalidAlignment)
  {
    TypeContext Types(Target64);
    for (const std::uint64_t Alignment : {std::uint64_t{0}, std::uint64_t{3}, std::uint64_t{6}, MaximumSize})
    {
      for (const Type *Value : createNominalTypes(Types, 5, Alignment))
      {
        EXPECT_EQ(Value, nullptr);
        EXPECT_FALSE(Types.owns(Value));
      }
    }
  }

  // Class, interface and struct identities remain intact in pointer, reference, array and function types.
  TEST(IRNominalTypeTest, ComposesWithStructuralTypes)
  {
    TypeContext Types(Target64);
    for (const Type *Value : createNominalTypes(Types, 5, 8))
    {
      ASSERT_NE(Value, nullptr);
      const auto Pointer = Types.getPointerType(Value);
      const auto Reference = Types.getReferenceType(Value);
      const auto Array = Types.getArrayType(Value, 3);
      const auto Function = Types.getFunctionType(Value, {Value});
      ASSERT_NE(Pointer, nullptr);
      ASSERT_NE(Reference, nullptr);
      ASSERT_NE(Array, nullptr);
      ASSERT_NE(Function, nullptr);
      EXPECT_EQ(Pointer->pointeeType(), Value);
      EXPECT_EQ(Reference->referencedType(), Value);
      EXPECT_EQ(Array->elementType(), Value);
      EXPECT_EQ(Array->elementCount(), 3U);
      EXPECT_EQ(Array->size(), 24U);
      EXPECT_EQ(Array->alignment(), 8U);
      EXPECT_EQ(Function->returnType(), Value);
      ASSERT_EQ(Function->parameterCount(), 1U);
      EXPECT_EQ(Function->parameters()[0], Value);
      EXPECT_EQ(Types.getPointerType(Value), Pointer);
      EXPECT_EQ(Types.getReferenceType(Value), Reference);
      EXPECT_EQ(Types.getArrayType(Value, 3), Array);
      EXPECT_EQ(Types.getFunctionType(Value, {Value}), Function);
    }
  }

  // Structural type factories reject nominal types belonging to another context in every operand position.
  TEST(IRNominalTypeTest, RejectsForeignContextOperands)
  {
    TypeContext Types(Target64);
    TypeContext OtherTypes(Target64);
    for (const Type *Value : createNominalTypes(OtherTypes, 8, 8))
    {
      ASSERT_NE(Value, nullptr);
      EXPECT_EQ(Types.getPointerType(Value), nullptr);
      EXPECT_EQ(Types.getReferenceType(Value), nullptr);
      EXPECT_EQ(Types.getArrayType(Value, 0), nullptr);
      EXPECT_EQ(Types.getArrayType(Value, 3), nullptr);
      EXPECT_EQ(Types.getFunctionType(Value, {}), nullptr);
      EXPECT_EQ(Types.getFunctionType(Types.getBoolType(), {Value}), nullptr);
    }
  }

  // Maximum unaligned nominal sizes reject overflowing array strides while allowing empty arrays.
  TEST(IRNominalTypeTest, ArrayChecksNominalStrideOverflow)
  {
    TypeContext Types(Target64);
    for (const Type *Value : createNominalTypes(Types, MaximumSize, 8))
    {
      ASSERT_NE(Value, nullptr);
      EXPECT_EQ(Value->size(), MaximumSize);
      EXPECT_EQ(Types.getArrayType(Value, 1), nullptr);
      const auto Empty = Types.getArrayType(Value, 0);
      ASSERT_NE(Empty, nullptr);
      EXPECT_EQ(Empty->elementType(), Value);
      EXPECT_EQ(Empty->size(), 0U);
      EXPECT_EQ(Empty->alignment(), 8U);
    }
  }
} // namespace ink::ir::test
