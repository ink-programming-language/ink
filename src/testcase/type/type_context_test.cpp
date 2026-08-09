#include "ink/type/type_context.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace ink::type
{
  namespace
  {
    static_assert(!std::is_convertible<TypeId, TypeId::ValueType>::value, "TypeId must not implicitly convert to its storage type");
    static_assert(!std::is_convertible<TypeId::ValueType, TypeId>::value, "TypeId must not implicitly construct from its storage type");

    // Verifies TypeId implements the full strongly typed value, ordering, Boolean, and hashing contract at valid and sentinel values.
    TEST(TypeIdTest, PreservesCompleteValueSemantics)
    {
      constexpr TypeId Invalid;
      constexpr TypeId First = TypeId::fromValue(3);
      constexpr TypeId Equal = TypeId::fromValue(3);
      constexpr TypeId Later = TypeId::fromValue(4);
      constexpr TypeId Sentinel = TypeId::fromValue(TypeId::InvalidValue);

      EXPECT_FALSE(Invalid.isValid());
      EXPECT_FALSE(static_cast<bool>(Invalid));
      EXPECT_EQ(Invalid, Sentinel);
      EXPECT_TRUE(First.isValid());
      EXPECT_TRUE(static_cast<bool>(First));
      EXPECT_EQ(First, Equal);
      EXPECT_NE(First, Later);
      EXPECT_LT(First, Later);
      EXPECT_LE(First, Equal);
      EXPECT_LE(First, Later);
      EXPECT_GT(Later, First);
      EXPECT_GE(Later, First);
      EXPECT_GE(Equal, First);
      EXPECT_EQ(std::hash<TypeId>{}(First), std::hash<TypeId>{}(Equal));
    }

    // Verifies every public type kind has a stable name and out-of-domain values use the documented fallback.
    TEST(TypeKindTest, NamesEveryPublicKind)
    {
      struct Expectation
      {
        TypeKind Kind;
        std::string_view Name;
      };
      constexpr Expectation Expectations[] = {
          {TypeKind::Error, "Error"},
          {TypeKind::Unit, "Unit"},
          {TypeKind::Bool, "Bool"},
          {TypeKind::I32, "I32"},
          {TypeKind::I64, "I64"},
          {TypeKind::U32, "U32"},
          {TypeKind::U64, "U64"},
          {TypeKind::Void, "Void"},
          {TypeKind::Never, "Never"},
          {TypeKind::Function, "Function"},
      };

      for (const Expectation &Value : Expectations)
      {
        EXPECT_EQ(typeKindName(Value.Kind), Value.Name);
      }
      EXPECT_STREQ(typeKindName(static_cast<TypeKind>(255)), "Unknown");
    }

    // Verifies structural equality observes function parameter order, result types, type kinds, and payload alternatives.
    TEST(TypeValueTest, ComparesCompleteStructuralPayloads)
    {
      const TypeId I32 = TypeId::fromValue(3);
      const TypeId I64 = TypeId::fromValue(4);
      const FunctionType First{{I32, I64}, I32};
      const FunctionType Equal{{I32, I64}, I32};
      const FunctionType DifferentOrder{{I64, I32}, I32};
      const FunctionType DifferentResult{{I32, I64}, I64};
      const Type Function{TypeKind::Function, First};
      const Type EqualFunction{TypeKind::Function, Equal};
      const Type DifferentKind{TypeKind::Error, First};
      const Type DifferentPayload{TypeKind::Function, DifferentResult};
      const Type Scalar{TypeKind::I32, std::monostate{}};

      EXPECT_EQ(First, Equal);
      EXPECT_NE(First, DifferentOrder);
      EXPECT_NE(First, DifferentResult);
      EXPECT_EQ(Function, EqualFunction);
      EXPECT_NE(Function, DifferentKind);
      EXPECT_NE(Function, DifferentPayload);
      EXPECT_NE(Function, Scalar);
    }

    // Verifies the foundational scalar and control types have stable distinct IDs and the expected kinds.
    TEST(TypeContextTest, ProvidesCanonicalBuiltinTypes)
    {
      TypeContext Types;

      EXPECT_EQ(Types.size(), 9U);
      EXPECT_NE(Types.errorType(), Types.unitType());
      EXPECT_NE(Types.unitType(), Types.boolType());
      EXPECT_NE(Types.boolType(), Types.i32Type());
      EXPECT_NE(Types.unitType(), Types.voidType());
      EXPECT_NE(Types.voidType(), Types.neverType());
      EXPECT_EQ(Types.type(Types.errorType()).Kind, TypeKind::Error);
      EXPECT_EQ(Types.type(Types.unitType()).Kind, TypeKind::Unit);
      EXPECT_EQ(Types.type(Types.boolType()).Kind, TypeKind::Bool);
      EXPECT_EQ(Types.type(Types.i32Type()).Kind, TypeKind::I32);
      EXPECT_EQ(Types.type(Types.i64Type()).Kind, TypeKind::I64);
      EXPECT_EQ(Types.type(Types.u32Type()).Kind, TypeKind::U32);
      EXPECT_EQ(Types.type(Types.u64Type()).Kind, TypeKind::U64);
      EXPECT_EQ(Types.type(Types.voidType()).Kind, TypeKind::Void);
      EXPECT_EQ(Types.type(Types.neverType()).Kind, TypeKind::Never);
    }

    // Verifies structurally equal function types reuse one TypeId while distinct signatures remain distinct.
    TEST(TypeContextTest, CanonicalizesFunctionTypesStructurally)
    {
      TypeContext Types;
      const TypeId First = Types.functionType({Types.i32Type(), Types.boolType()}, Types.i32Type());
      const TypeId Equal = Types.functionType({Types.i32Type(), Types.boolType()}, Types.i32Type());
      const TypeId DifferentResult = Types.functionType({Types.i32Type(), Types.boolType()}, Types.boolType());
      const TypeId DifferentParameters = Types.functionType({Types.boolType(), Types.i32Type()}, Types.i32Type());

      EXPECT_EQ(First, Equal);
      EXPECT_NE(First, DifferentResult);
      EXPECT_NE(First, DifferentParameters);
      EXPECT_EQ(Types.type(First).Kind, TypeKind::Function);
      EXPECT_EQ(Types.function(First).Parameters, (std::vector<TypeId>{Types.i32Type(), Types.boolType()}));
      EXPECT_EQ(Types.function(First).Result, Types.i32Type());
    }

    // Verifies zero-parameter and nested function signatures are canonicalized without flattening their structural boundaries.
    TEST(TypeContextTest, CanonicalizesEmptyAndNestedFunctionTypes)
    {
      TypeContext Types;
      const TypeId Leaf = Types.functionType({}, Types.i32Type());
      const TypeId EqualLeaf = Types.functionType({}, Types.i32Type());
      const TypeId Nested = Types.functionType({Leaf}, Leaf);
      const TypeId DifferentNested = Types.functionType({Types.i32Type()}, Leaf);

      EXPECT_EQ(Leaf, EqualLeaf);
      EXPECT_TRUE(Types.function(Leaf).Parameters.empty());
      EXPECT_EQ(Types.function(Leaf).Result, Types.i32Type());
      EXPECT_EQ(Types.function(Nested).Parameters, (std::vector<TypeId>{Leaf}));
      EXPECT_EQ(Types.function(Nested).Result, Leaf);
      EXPECT_NE(Nested, DifferentNested);
      EXPECT_EQ(Types.size(), 12U);
    }

    // Verifies invalid IDs and non-function queries are rejected at the owning context boundary.
    TEST(TypeContextTest, RejectsInvalidAndMismatchedQueries)
    {
      TypeContext Types;

      EXPECT_FALSE(Types.contains({}));
      EXPECT_THROW(Types.type({}), std::out_of_range);
      EXPECT_THROW(Types.function(Types.i32Type()), std::invalid_argument);
      EXPECT_THROW(Types.functionType({TypeId::fromValue(100)}, Types.i32Type()), std::invalid_argument);
      EXPECT_THROW(Types.functionType({Types.i32Type()}, TypeId::fromValue(100)), std::invalid_argument);
    }

    // Verifies independent contexts do not share their canonical function-type storage or identity growth.
    TEST(TypeContextTest, KeepsCanonicalTablesContextLocal)
    {
      TypeContext First;
      TypeContext Second;
      First.functionType({First.i32Type()}, First.i32Type());

      EXPECT_EQ(First.size(), 10U);
      EXPECT_EQ(Second.size(), 9U);
      EXPECT_EQ(Second.functionType({Second.i32Type()}, Second.i32Type()).value(), 9U);
    }
  } // namespace
} // namespace ink::type
