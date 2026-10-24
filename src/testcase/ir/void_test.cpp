#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/module/module.h"

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
    static_assert(std::is_same_v<decltype(std::declval<const Module &>().typePool().get<ink::ir::VoidType>()), const VoidType *>);
    static_assert(noexcept(std::declval<const Module &>().typePool().get<ink::ir::VoidType>()));
    static_assert(noexcept(std::declval<const PointerType &>().isOpaque()));
  } // namespace

  // Void has one canonical TypePool-owned identity and a valid zero-byte layout on either target width.
  TEST(IRVoidTypeTest, CanonicalUnitLayoutIsIndependentOfTargetWidth)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      IRContext TypesContext(Target);
      const Module Types(TypesContext);
      IRContext OtherContext(Target);
      const Module Other(OtherContext);
      const VoidType *Void = Types.typePool().get<ink::ir::VoidType>();
      ASSERT_NE(Void, nullptr);
      EXPECT_EQ(Void, Types.typePool().get<ink::ir::VoidType>());
      EXPECT_NE(Void, Other.typePool().get<ink::ir::VoidType>());
      EXPECT_TRUE(Types.ownsType(Void));
      EXPECT_FALSE(Other.ownsType(Void));
      const Type *TypeValue = Void;
      const Value *ValueValue = Void;
      EXPECT_TRUE(TypeValue->hasLayout());
      EXPECT_EQ(TypeValue->size(), 0U);
      EXPECT_EQ(TypeValue->alignment(), 1U);
      EXPECT_EQ(ValueValue->kind(), ValueKind::Void);
      EXPECT_TRUE(Type::classof(ValueValue));
      EXPECT_TRUE(VoidType::classof(ValueValue));
      EXPECT_FALSE(VoidType::classof(Types.typePool().get<ink::ir::BoolType>()));
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
      IRContext TypesContext(Target);
      Module Types(TypesContext);
      const PointerType *Opaque = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::VoidType>());
      const PointerType *Nested = Types.typePool().get<ink::ir::PointerType>(Opaque);
      const PointerType *Typed = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::BoolType>());
      ASSERT_NE(Opaque, nullptr);
      ASSERT_NE(Nested, nullptr);
      ASSERT_NE(Typed, nullptr);
      EXPECT_EQ(Opaque, Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::VoidType>()));
      EXPECT_EQ(Nested, Types.typePool().get<ink::ir::PointerType>(Opaque));
      EXPECT_EQ(Opaque->pointeeType(), Types.typePool().get<ink::ir::VoidType>());
      EXPECT_EQ(Nested->pointeeType(), Opaque);
      EXPECT_TRUE(Opaque->isOpaque());
      EXPECT_FALSE(Nested->isOpaque());
      EXPECT_FALSE(Typed->isOpaque());
      EXPECT_NE(Opaque, Typed);
      EXPECT_EQ(Opaque->size(), Target.pointerByteWidth());
      EXPECT_EQ(Opaque->alignment(), Target.pointerByteWidth());
      EXPECT_EQ(Nested->size(), Target.pointerByteWidth());
      EXPECT_TRUE(Types.ownsType(Opaque));
      EXPECT_TRUE(Types.ownsType(Nested));
    }
  }

  // Void can be a function result, a unit parameter, an array element or a referenced unit value.
  TEST(IRVoidTypeTest, UnitTypesComposeWithFunctionsArraysAndReferences)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const VoidType *Void = Types.typePool().get<ink::ir::VoidType>();
    const PointerType *Opaque = Types.typePool().get<ink::ir::PointerType>(Void);
    const FunctionType *Function = Types.typePool().get<ink::ir::FunctionType>(Void, {Opaque});
    const FunctionType *UnitParameter = Types.typePool().get<ink::ir::FunctionType>(Void, {Void});
    const FunctionType *Empty = Types.typePool().get<ink::ir::FunctionType>(Void);
    const ArrayType *Units = Types.typePool().get<ink::ir::ArrayType>(Void, std::numeric_limits<std::uint64_t>::max());
    const ReferenceType *Reference = Types.typePool().get<ink::ir::ReferenceType>(Void);
    ASSERT_NE(Function, nullptr);
    ASSERT_NE(UnitParameter, nullptr);
    ASSERT_NE(Empty, nullptr);
    ASSERT_NE(Units, nullptr);
    ASSERT_NE(Reference, nullptr);
    EXPECT_EQ(Function, Types.typePool().get<ink::ir::FunctionType>(Void, {Opaque}));
    EXPECT_EQ(Function->returnType(), Void);
    ASSERT_EQ(Function->parameterCount(), 1U);
    EXPECT_EQ(Function->parameters()[0].ParameterType, Opaque);
    EXPECT_EQ(UnitParameter->parameters()[0].ParameterType, Void);
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
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    IRContext OtherContext(Target64);
    const Module Other(OtherContext);
    const VoidType *Void = Types.typePool().get<ink::ir::VoidType>();
    const VoidType *Foreign = Other.typePool().get<ink::ir::VoidType>();
    EXPECT_EQ(Types.typePool().get<ink::ir::PointerType>(Foreign), nullptr);
    EXPECT_EQ(Types.typePool().get<ink::ir::ReferenceType>(Foreign), nullptr);
    EXPECT_EQ(Types.typePool().get<ink::ir::ArrayType>(Foreign, 0), nullptr);
    EXPECT_EQ(Types.typePool().get<ink::ir::FunctionType>(Foreign), nullptr);
    EXPECT_EQ(Types.typePool().get<ink::ir::FunctionType>(Void, {Foreign}), nullptr);
    EXPECT_EQ(Types.typePool().get<ink::ir::PointerType>(nullptr), nullptr);
  }

  // One null literal initializes opaque, typed and nested pointers while preserving its TypePool-owned literal type.
  TEST(IRVoidTypeTest, OpaquePointersAcceptCanonicalNullLiteral)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      IRContext TypesContext(Target);
      Module Types(TypesContext);
      IRContext OtherContext(Target);
      Module Other(OtherContext);
      auto &Constants = Types.constantPool();
      auto &SharedTypes = Types.constantPool();
      auto &OtherConstants = Other.constantPool();
      const PointerType *Opaque = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::VoidType>());
      const PointerType *Typed = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::BoolType>());
      const PointerType *Nested = Types.typePool().get<ink::ir::PointerType>(Opaque);
      const NullPointerConstant *Null = Constants.get<ink::ir::NullPointerConstant>();
      ASSERT_NE(Null, nullptr);
      EXPECT_EQ(Null, Constants.get<ink::ir::NullPointerConstant>());
      EXPECT_EQ(Null->type(), Types.typePool().get<ink::ir::NullPointerType>());
      EXPECT_EQ(Null->type()->size(), Target.pointerByteWidth());
      EXPECT_TRUE(Null->canInitialize(Opaque));
      EXPECT_TRUE(Null->canInitialize(Typed));
      EXPECT_TRUE(Null->canInitialize(Nested));
      EXPECT_EQ(&Constants, &SharedTypes);
      EXPECT_EQ(Null, SharedTypes.get<ink::ir::NullPointerConstant>());
      EXPECT_NE(Null, OtherConstants.get<ink::ir::NullPointerConstant>());
      EXPECT_TRUE(Constants.owns(Null));
      EXPECT_TRUE(SharedTypes.owns(Null));
      EXPECT_FALSE(OtherConstants.owns(Null));
      EXPECT_FALSE(Null->canInitialize(Other.typePool().get<ink::ir::PointerType>(Other.typePool().get<ink::ir::VoidType>())));
    }
  }
} // namespace ink::ir::test
