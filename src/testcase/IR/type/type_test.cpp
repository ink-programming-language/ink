#include "ink/ir/type/array_type.h"
#include "ink/ir/type/bool_type.h"
#include "ink/ir/type/float_type.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/reference_type.h"
#include "ink/ir/type/type.h"
#include "ink/ir/type/type_kind.h"
#include "ink/ir/type/void_type.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>

namespace ink::ir::test
{
  static_assert(!std::is_constructible_v<ArrayType, IntegerType &&, std::uint64_t>);
  static_assert(!std::is_constructible_v<ArrayType, const IntegerType &&, std::uint64_t>);
  static_assert(!std::is_constructible_v<ReferenceType, IntegerType &&>);
  static_assert(!std::is_constructible_v<ReferenceType, const IntegerType &&>);

  // Scalar categories keep distinct spellings while different widths and signs share their category kind.
  TEST(IRTypeTest, IdentifiesDistinctScalarTypesThroughBasePointers)
  {
    const IntegerType SignedInteger(IntegerType::Width::Bits8, IntegerType::Signedness::Signed);
    const IntegerType UnsignedInteger(IntegerType::Width::Bits64, IntegerType::Signedness::Unsigned);
    const FloatType Float32(FloatType::Width::Bits32);
    const FloatType Float64(FloatType::Width::Bits64);
    const BoolType Bool;
    const VoidType Void;
    struct Case
    {
        const Type *Value;
        TypeKind Kind;
        std::string_view Name;
        bool (*Classof)(const ink::ir::Value *) noexcept;
    };
    const Case Cases[] = {
        {&SignedInteger, TypeKind::Integer, "integer", IntegerType::classof},
        {&UnsignedInteger, TypeKind::Integer, "integer", IntegerType::classof},
        {&Float32, TypeKind::Float, "float", FloatType::classof},
        {&Float64, TypeKind::Float, "float", FloatType::classof},
        {&Bool, TypeKind::Bool, "bool", BoolType::classof},
        {&Void, TypeKind::Void, "void", VoidType::classof},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Name);
      EXPECT_EQ(Entry.Value->getKind(), Entry.Kind);
      EXPECT_EQ(Entry.Value->getName(), Entry.Name);
      EXPECT_EQ(Entry.Value->getSemantics(), TypeSemantics::Value);
      EXPECT_EQ(typeKindName(Entry.Kind), Entry.Name);
      EXPECT_FALSE(Entry.Classof(nullptr));
      EXPECT_FALSE(ArrayType::classof(Entry.Value));
      for (const auto &Other : Cases)
      {
        SCOPED_TRACE(Other.Name);
        EXPECT_EQ(Entry.Classof(Other.Value), Entry.Kind == Other.Kind);
      }
    }
  }

  // Every integer width supports both signs without changing its width or the shared integer category.
  TEST(IRTypeTest, PreservesIntegerWidthsAndSignedness)
  {
    struct Case
    {
        IntegerType::Width Width;
        IntegerType::Signedness Sign;
        std::uint32_t ExpectedWidth;
        bool ExpectedSigned;
    };
    const Case Cases[] = {
        {IntegerType::Width::Bits8, IntegerType::Signedness::Signed, 8, true},
        {IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned, 8, false},
        {IntegerType::Width::Bits16, IntegerType::Signedness::Signed, 16, true},
        {IntegerType::Width::Bits16, IntegerType::Signedness::Unsigned, 16, false},
        {IntegerType::Width::Bits32, IntegerType::Signedness::Signed, 32, true},
        {IntegerType::Width::Bits32, IntegerType::Signedness::Unsigned, 32, false},
        {IntegerType::Width::Bits64, IntegerType::Signedness::Signed, 64, true},
        {IntegerType::Width::Bits64, IntegerType::Signedness::Unsigned, 64, false},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.ExpectedWidth);
      SCOPED_TRACE(Entry.ExpectedSigned);
      const IntegerType Integer(Entry.Width, Entry.Sign);
      const Type *Base = &Integer;
      EXPECT_EQ(Integer.bitWidth(), Entry.ExpectedWidth);
      EXPECT_EQ(Integer.isSigned(), Entry.ExpectedSigned);
      EXPECT_EQ(Base->getKind(), TypeKind::Integer);
      EXPECT_EQ(Base->getSemantics(), TypeSemantics::Value);
      EXPECT_TRUE(IntegerType::classof(Base));
    }
  }

  // Floating point widths remain distinct within one category and bool retains its independent one-bit width.
  TEST(IRTypeTest, PreservesFloatingPointWidthsAndBooleanWidth)
  {
    EXPECT_EQ(FloatType(FloatType::Width::Bits32).bitWidth(), 32U);
    EXPECT_EQ(FloatType(FloatType::Width::Bits64).bitWidth(), 64U);
    EXPECT_EQ(BoolType().bitWidth(), 1U);
  }

  // Nested arrays preserve the borrowed element identity and the independent length of each dimension.
  TEST(IRTypeTest, PreservesNestedArrayElementTypesAndLengths)
  {
    const IntegerType Element(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const ArrayType Row(Element, 4);
    const ArrayType Matrix(Row, 3);
    const Type *Base = &Matrix;
    EXPECT_EQ(Base->getKind(), TypeKind::Array);
    EXPECT_EQ(Base->getName(), "array");
    EXPECT_EQ(Base->getSemantics(), TypeSemantics::Value);
    EXPECT_EQ(typeKindName(TypeKind::Array), "array");
    EXPECT_TRUE(ArrayType::classof(Base));
    EXPECT_FALSE(ArrayType::classof(nullptr));
    EXPECT_FALSE(IntegerType::classof(Base));
    EXPECT_EQ(&Matrix.elementType(), &Row);
    EXPECT_EQ(Matrix.elementCount(), 3U);
    EXPECT_TRUE(ArrayType::classof(&Matrix.elementType()));
    EXPECT_EQ(&Row.elementType(), &Element);
    EXPECT_EQ(Row.elementCount(), 4U);
    EXPECT_TRUE(IntegerType::classof(&Row.elementType()));
  }

  // Zero-length arrays remain representable and large lengths are stored without narrowing to a host-sized integer.
  TEST(IRTypeTest, PreservesEmptyAndMaximumArrayLengths)
  {
    const IntegerType Element(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned);
    const ArrayType Empty(Element, 0);
    const ArrayType Maximum(Element, std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(Empty.elementCount(), 0U);
    EXPECT_EQ(&Empty.elementType(), &Element);
    EXPECT_EQ(Maximum.elementCount(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(&Maximum.elementType(), &Element);
  }

  // References preserve target identity and their distinct kind while exposing reference semantics through Type.
  TEST(IRTypeTest, IdentifiesReferencesAndPreservesNestedTargets)
  {
    const IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const ReferenceType Reference(Integer);
    const ReferenceType Nested(static_cast<const Type &>(Reference));
    const Type *Base = &Reference;
    EXPECT_EQ(Base->getKind(), TypeKind::Reference);
    EXPECT_EQ(Base->getName(), "reference");
    EXPECT_EQ(typeKindName(TypeKind::Reference), "reference");
    EXPECT_EQ(Base->getSemantics(), TypeSemantics::Reference);
    EXPECT_TRUE(ReferenceType::classof(Base));
    EXPECT_FALSE(ReferenceType::classof(nullptr));
    EXPECT_FALSE(ReferenceType::classof(&Integer));
    EXPECT_FALSE(IntegerType::classof(Base));
    EXPECT_EQ(&Reference.referencedType(), &Integer);
    EXPECT_EQ(&Nested.referencedType(), &Reference);
  }

  // An unrecognized kind is named explicitly instead of being confused with a valid scalar or array.
  TEST(IRTypeTest, NamesUnknownKindsWithoutMisclassification)
  {
    EXPECT_EQ(typeKindName(static_cast<TypeKind>(255)), "InvalidTypeKind");
  }
} // namespace ink::ir::test
