#include "IR/Type/type.h"
#include "IR/Type/type_kind.h"
#include "IR/Type/int8_type.h"
#include "IR/Type/int16_type.h"
#include "IR/Type/int32_type.h"
#include "IR/Type/int64_type.h"
#include "IR/Type/uint8_type.h"
#include "IR/Type/uint16_type.h"
#include "IR/Type/uint32_type.h"
#include "IR/Type/uint64_type.h"
#include "IR/Type/float32_type.h"
#include "IR/Type/float64_type.h"
#include "IR/Type/bool_type.h"
#include "IR/Type/void_type.h"
#include "IR/Type/array_type.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>

namespace ink::ir::test
{
  static_assert(!std::is_constructible_v<ArrayType, Int32Type &&, std::uint64_t>);
  static_assert(!std::is_constructible_v<ArrayType, const Int32Type &&, std::uint64_t>);

  // Every scalar kind retains its spelling and is distinguished from every other scalar through Type pointers.
  TEST(IRTypeTest, IdentifiesDistinctScalarTypesThroughBasePointers)
  {
    const Int8Type Int8;
    const Int16Type Int16;
    const Int32Type Int32;
    const Int64Type Int64;
    const UInt8Type UInt8;
    const UInt16Type UInt16;
    const UInt32Type UInt32;
    const UInt64Type UInt64;
    const Float32Type Float32;
    const Float64Type Float64;
    const BoolType Bool;
    const VoidType Void;
    struct Case
    {
        const Type *Value;
        TypeKind Kind;
        std::string_view Name;
        bool (*Classof)(const Type *) noexcept;
    };
    const Case Cases[] = {
        {&Int8, TypeKind::Int8, "int8", Int8Type::classof},
        {&Int16, TypeKind::Int16, "int16", Int16Type::classof},
        {&Int32, TypeKind::Int32, "int32", Int32Type::classof},
        {&Int64, TypeKind::Int64, "int64", Int64Type::classof},
        {&UInt8, TypeKind::UInt8, "uint8", UInt8Type::classof},
        {&UInt16, TypeKind::UInt16, "uint16", UInt16Type::classof},
        {&UInt32, TypeKind::UInt32, "uint32", UInt32Type::classof},
        {&UInt64, TypeKind::UInt64, "uint64", UInt64Type::classof},
        {&Float32, TypeKind::Float32, "float32", Float32Type::classof},
        {&Float64, TypeKind::Float64, "float64", Float64Type::classof},
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

  // Signed and unsigned integers expose their declared widths, while floating point and bool retain their own widths.
  TEST(IRTypeTest, ReportsScalarBitWidths)
  {
    EXPECT_EQ(Int8Type().bitWidth(), 8U);
    EXPECT_EQ(Int16Type().bitWidth(), 16U);
    EXPECT_EQ(Int32Type().bitWidth(), 32U);
    EXPECT_EQ(Int64Type().bitWidth(), 64U);
    EXPECT_EQ(UInt8Type().bitWidth(), 8U);
    EXPECT_EQ(UInt16Type().bitWidth(), 16U);
    EXPECT_EQ(UInt32Type().bitWidth(), 32U);
    EXPECT_EQ(UInt64Type().bitWidth(), 64U);
    EXPECT_EQ(Float32Type().bitWidth(), 32U);
    EXPECT_EQ(Float64Type().bitWidth(), 64U);
    EXPECT_EQ(BoolType().bitWidth(), 1U);
  }

  // Nested arrays preserve the borrowed element identity and the independent length of each dimension.
  TEST(IRTypeTest, PreservesNestedArrayElementTypesAndLengths)
  {
    const Int32Type Element;
    const ArrayType Row(Element, 4);
    const ArrayType Matrix(Row, 3);
    const Type *Base = &Matrix;
    EXPECT_EQ(Base->getKind(), TypeKind::Array);
    EXPECT_EQ(Base->getName(), "array");
    EXPECT_EQ(Base->getSemantics(), TypeSemantics::Value);
    EXPECT_EQ(typeKindName(TypeKind::Array), "array");
    EXPECT_TRUE(ArrayType::classof(Base));
    EXPECT_FALSE(ArrayType::classof(nullptr));
    EXPECT_FALSE(Int32Type::classof(Base));
    EXPECT_EQ(&Matrix.elementType(), &Row);
    EXPECT_EQ(Matrix.elementCount(), 3U);
    EXPECT_TRUE(ArrayType::classof(&Matrix.elementType()));
    EXPECT_EQ(&Row.elementType(), &Element);
    EXPECT_EQ(Row.elementCount(), 4U);
    EXPECT_TRUE(Int32Type::classof(&Row.elementType()));
  }

  // Zero-length arrays remain representable and large lengths are stored without narrowing to a host-sized integer.
  TEST(IRTypeTest, PreservesEmptyAndMaximumArrayLengths)
  {
    const UInt8Type Element;
    const ArrayType Empty(Element, 0);
    const ArrayType Maximum(Element, std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(Empty.elementCount(), 0U);
    EXPECT_EQ(&Empty.elementType(), &Element);
    EXPECT_EQ(Maximum.elementCount(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(&Maximum.elementType(), &Element);
  }

  // A derived reference type overrides the default value semantics even when queried through the base interface.
  TEST(IRTypeTest, QueriesOverriddenSemanticsThroughBasePointer)
  {
    class ReferenceType final : public Type
    {
      public:
        constexpr ReferenceType() noexcept
            : Type(TypeKind::Array)
        {
        }

        constexpr TypeSemantics getSemantics() const noexcept override
        {
          return TypeSemantics::Reference;
        }
    };

    const ReferenceType Reference;
    const Type *Base = &Reference;
    EXPECT_EQ(Base->getSemantics(), TypeSemantics::Reference);
  }

  // An unrecognized kind is named explicitly instead of being confused with a valid scalar or array.
  TEST(IRTypeTest, NamesUnknownKindsWithoutMisclassification)
  {
    EXPECT_EQ(typeKindName(static_cast<TypeKind>(255)), "InvalidTypeKind");
  }
} // namespace ink::ir::test
