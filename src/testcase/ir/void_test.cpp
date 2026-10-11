#include "ink/ir/constant/constant_context.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::BigEndian);

    constexpr std::optional<std::uint64_t> fixedValueSize(ValueKind Kind) noexcept
    {
      switch (Kind)
      {
#define INK_VALUE(Kind, Name, FixedSize) \
  case ValueKind::Kind:                  \
    return FixedSize;
#include "ink/ir/type/Value.def"
#undef INK_VALUE
      }
      return std::nullopt;
    }

    static_assert(!std::is_default_constructible_v<VoidType>);
    static_assert(!std::is_copy_constructible_v<VoidType> && !std::is_move_constructible_v<VoidType>);
    static_assert(std::is_same_v<decltype(std::declval<const TypeContext &>().getVoidType()), const VoidType *>);
    static_assert(noexcept(std::declval<const TypeContext &>().getVoidType()));
    static_assert(noexcept(std::declval<const PointerType &>().isOpaque()));
  } // namespace

  // Void has one canonical context-owned identity and a valid zero-byte layout on either target width.
  TEST(IRVoidTypeTest, CanonicalUnitLayoutIsIndependentOfTargetWidth)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      const TypeContext Types(Target);
      const TypeContext Other(Target);
      const VoidType *Void = Types.getVoidType();
      ASSERT_NE(Void, nullptr);
      EXPECT_EQ(Void, Types.getVoidType());
      EXPECT_NE(Void, Other.getVoidType());
      EXPECT_TRUE(Types.owns(Void));
      EXPECT_FALSE(Other.owns(Void));
      const Type *TypeValue = Void;
      const Value *ValueValue = Void;
      EXPECT_TRUE(TypeValue->hasLayout());
      EXPECT_EQ(TypeValue->size(), 0U);
      EXPECT_EQ(TypeValue->alignment(), 1U);
      EXPECT_EQ(ValueValue->kind(), ValueKind::Void);
      EXPECT_TRUE(Type::classof(ValueValue));
      EXPECT_TRUE(VoidType::classof(ValueValue));
      EXPECT_FALSE(VoidType::classof(Types.getBoolType()));
      EXPECT_FALSE(VoidType::classof(nullptr));
      EXPECT_FALSE(Constant::classof(ValueValue));
    }
    EXPECT_EQ(valueKindName(ValueKind::Void), "void");
    ASSERT_TRUE(fixedValueSize(ValueKind::Void).has_value());
    EXPECT_EQ(*fixedValueSize(ValueKind::Void), 0U);
    EXPECT_FALSE(fixedValueSize(ValueKind::Pointer).has_value());
  }

  // Only the immediate *void pointer is opaque; an outer pointer to *void keeps a known pointer-sized pointee.
  TEST(IRVoidTypeTest, OpaquePointersPreserveNestedPointeeIdentityAndTargetLayout)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      TypeContext Types(Target);
      const PointerType *Opaque = Types.getPointerType(Types.getVoidType());
      const PointerType *Nested = Types.getPointerType(Opaque);
      const PointerType *Typed = Types.getPointerType(Types.getBoolType());
      ASSERT_NE(Opaque, nullptr);
      ASSERT_NE(Nested, nullptr);
      ASSERT_NE(Typed, nullptr);
      EXPECT_EQ(Opaque, Types.getPointerType(Types.getVoidType()));
      EXPECT_EQ(Nested, Types.getPointerType(Opaque));
      EXPECT_EQ(Opaque->pointeeType(), Types.getVoidType());
      EXPECT_EQ(Nested->pointeeType(), Opaque);
      EXPECT_TRUE(Opaque->isOpaque());
      EXPECT_FALSE(Nested->isOpaque());
      EXPECT_FALSE(Typed->isOpaque());
      EXPECT_NE(Opaque, Typed);
      EXPECT_EQ(Opaque->size(), Target.pointerByteWidth());
      EXPECT_EQ(Opaque->alignment(), Target.pointerByteWidth());
      EXPECT_EQ(Nested->size(), Target.pointerByteWidth());
      EXPECT_TRUE(Types.owns(Opaque));
      EXPECT_TRUE(Types.owns(Nested));
    }
  }

  // Void can be a function result, a unit parameter, an array element or a referenced unit value.
  TEST(IRVoidTypeTest, UnitTypesComposeWithFunctionsArraysAndReferences)
  {
    TypeContext Types(Target64);
    const VoidType *Void = Types.getVoidType();
    const PointerType *Opaque = Types.getPointerType(Void);
    const FunctionType *Function = Types.getFunctionType(Void, {Parameter(Opaque)});
    const FunctionType *UnitParameter = Types.getFunctionType(Void, {Parameter(Void)});
    const FunctionType *Empty = Types.getFunctionType(Void);
    const ArrayType *Units = Types.getArrayType(Void, std::numeric_limits<std::uint64_t>::max());
    const ReferenceType *Reference = Types.getReferenceType(Void);
    ASSERT_NE(Function, nullptr);
    ASSERT_NE(UnitParameter, nullptr);
    ASSERT_NE(Empty, nullptr);
    ASSERT_NE(Units, nullptr);
    ASSERT_NE(Reference, nullptr);
    EXPECT_EQ(Function, Types.getFunctionType(Void, {Parameter(Opaque)}));
    EXPECT_EQ(Function->returnType(), Void);
    ASSERT_EQ(Function->parameterCount(), 1U);
    EXPECT_EQ(Function->parameters()[0].type(), Opaque);
    EXPECT_EQ(UnitParameter->parameters()[0].type(), Void);
    EXPECT_TRUE(Empty->parameters().empty());
    EXPECT_EQ(Empty->returnType(), Void);
    EXPECT_EQ(Units->elementType(), Void);
    EXPECT_EQ(Units->elementCount(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(Units->size(), 0U);
    EXPECT_EQ(Units->alignment(), 1U);
    EXPECT_EQ(Reference->referencedType(), Void);
    EXPECT_EQ(Reference->size(), Target64.pointerByteWidth());
  }

  // Structural lookups reject a foreign void identity in every operand position while accepting the local unit.
  TEST(IRVoidTypeTest, StructuralTypesRejectForeignUnitIdentities)
  {
    TypeContext Types(Target64);
    const TypeContext Other(Target64);
    const VoidType *Void = Types.getVoidType();
    const VoidType *Foreign = Other.getVoidType();
    EXPECT_EQ(Types.getPointerType(Foreign), nullptr);
    EXPECT_EQ(Types.getReferenceType(Foreign), nullptr);
    EXPECT_EQ(Types.getArrayType(Foreign, 0), nullptr);
    EXPECT_EQ(Types.getFunctionType(Foreign), nullptr);
    EXPECT_EQ(Types.getFunctionType(Void, {Parameter(Foreign)}), nullptr);
    EXPECT_EQ(Types.getPointerType(nullptr), nullptr);
  }

  // Null opaque pointers retain a non-null void pointee and remain distinct from typed and nested null pointers.
  TEST(IRVoidTypeTest, OpaqueNullConstantsUseCanonicalPointerTypes)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      TypeContext Types(Target);
      TypeContext Other(Target);
      ConstantContext Constants(Types);
      ConstantContext SharedTypes(Types);
      const PointerType *Opaque = Types.getPointerType(Types.getVoidType());
      const NullPointerConstant *Null = Constants.getNullPointerConstant(Opaque);
      const NullPointerConstant *TypedNull = Constants.getNullPointerConstant(Types.getPointerType(Types.getBoolType()));
      const NullPointerConstant *NestedNull = Constants.getNullPointerConstant(Types.getPointerType(Opaque));
      ASSERT_NE(Null, nullptr);
      ASSERT_NE(TypedNull, nullptr);
      ASSERT_NE(NestedNull, nullptr);
      EXPECT_EQ(Null, Constants.getNullPointerConstant(Opaque));
      EXPECT_EQ(Null->type(), Opaque);
      EXPECT_EQ(Null->type()->size(), Target.pointerByteWidth());
      EXPECT_NE(Null, TypedNull);
      EXPECT_NE(Null, NestedNull);
      EXPECT_NE(Null, SharedTypes.getNullPointerConstant(Opaque));
      EXPECT_TRUE(Constants.owns(Null));
      EXPECT_FALSE(SharedTypes.owns(Null));
      EXPECT_EQ(Constants.getNullPointerConstant(Other.getPointerType(Other.getVoidType())), nullptr);
    }
  }
} // namespace ink::ir::test
