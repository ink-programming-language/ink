#include "ink/core/name/name_pool.h"
#include "ink/core/support/hash.h"
#include "ink/ir/type/bool_type.h"
#include "ink/ir/type/function_type.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/void_type.h"

#include <gtest/gtest.h>

#include <functional>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir::test
{
  static_assert(std::is_copy_constructible_v<Parameter>);
  static_assert(std::is_copy_assignable_v<Parameter>);
  static_assert(std::is_same_v<decltype(std::declval<const Parameter &>().name()), core::Name>);
  static_assert(!std::is_constructible_v<Parameter, const IntegerType &, std::string>);
  static_assert(!std::is_constructible_v<Parameter, const IntegerType &, const char *>);
  static_assert(!std::is_constructible_v<Parameter, IntegerType &&>);
  static_assert(!std::is_constructible_v<Parameter, const IntegerType &&>);
  static_assert(!std::is_constructible_v<Parameter, IntegerType &&, core::Name>);
  static_assert(!std::is_constructible_v<Parameter, const IntegerType &&, core::Name>);
  static_assert(!std::is_constructible_v<Parameter, IntegerType &&, core::Name, Parameter::Kind>);
  static_assert(!std::is_constructible_v<Parameter, const IntegerType &&, core::Name, Parameter::Kind>);
  static_assert(!std::is_constructible_v<FunctionType, IntegerType &&>);
  static_assert(!std::is_constructible_v<FunctionType, const IntegerType &&>);
  static_assert(!std::is_constructible_v<FunctionType, IntegerType &&, std::span<const Parameter>>);
  static_assert(!std::is_constructible_v<FunctionType, const IntegerType &&, std::span<const Parameter>>);
  static_assert(!std::is_copy_constructible_v<FunctionType>);
  static_assert(!std::is_move_constructible_v<FunctionType>);

  // Parameter copies share interned names while retaining the exact borrowed type identity and parameter kind.
  TEST(IRFunctionTypeTest, CopiesInternedParameterNamesAndBorrowedTypeIdentity)
  {
    auto &Names = core::NamePool::instance();
    const IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Bool;
    std::string SourceName = "count";
    const Parameter Named(Integer, Names.intern(SourceName), Parameter::Kind::Named);
    SourceName.assign("changed");
    EXPECT_EQ(Named.name(), Names.intern("count"));
    EXPECT_EQ(Names.text(Named.name()), "count");
    EXPECT_EQ(&Named.type(), &Integer);
    EXPECT_EQ(Named.kind(), Parameter::Kind::Named);
    Parameter Copy = Named;
    EXPECT_EQ(Copy.name(), Named.name());
    EXPECT_EQ(&Copy.type(), &Integer);
    EXPECT_EQ(Copy.kind(), Parameter::Kind::Named);
    Copy = Parameter(Bool);
    EXPECT_EQ(Copy.name(), core::NAME_None);
    EXPECT_EQ(&Copy.type(), &Bool);
    EXPECT_EQ(Copy.kind(), Parameter::Kind::Positional);
    EXPECT_EQ(Named.name(), Names.intern("count"));
    EXPECT_EQ(&Named.type(), &Integer);
    EXPECT_EQ(Named.kind(), Parameter::Kind::Named);
  }

  // Names do not imply named parameters, and signatures retain positional, named, and variadic metadata independently.
  TEST(IRFunctionTypeTest, PreservesExplicitParameterKindsWithoutInferringFromNames)
  {
    auto &Names = core::NamePool::instance();
    const IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const Parameter Parameters[] = {
        Parameter(Integer, Names.intern("value")),
        Parameter(Integer, Names.intern("value"), Parameter::Kind::Named),
        Parameter(Integer, Names.intern("value"), Parameter::Kind::Variadic),
    };
    const FunctionType Function(Integer, Parameters);
    ASSERT_EQ(Function.parameters().size(), 3U);
    EXPECT_EQ(Function.parameters()[0].kind(), Parameter::Kind::Positional);
    EXPECT_EQ(Function.parameters()[1].kind(), Parameter::Kind::Named);
    EXPECT_EQ(Function.parameters()[2].kind(), Parameter::Kind::Variadic);
    for (const auto &ParameterValue : Function.parameters())
    {
      EXPECT_EQ(ParameterValue.name(), Names.intern("value"));
      EXPECT_EQ(&ParameterValue.type(), &Integer);
    }
  }

  // Function types preserve ordered parameter metadata and expose their distinct kind through Type.
  TEST(IRFunctionTypeTest, IdentifiesFunctionsAndPreservesOrderedSignatures)
  {
    auto &Names = core::NamePool::instance();
    const IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Bool;
    const VoidType Void;
    const Parameter Parameters[] = {
        Parameter(Integer, Names.intern("count")),
        Parameter(Bool, Names.intern("ready")),
    };
    const FunctionType Function(Void, Parameters);
    const Type *Base = &Function;
    EXPECT_EQ(Base->getKind(), TypeKind::Function);
    EXPECT_EQ(Base->getName(), "function");
    EXPECT_EQ(typeKindName(TypeKind::Function), "function");
    EXPECT_EQ(Base->getSemantics(), TypeSemantics::Value);
    EXPECT_TRUE(FunctionType::classof(Base));
    EXPECT_FALSE(FunctionType::classof(nullptr));
    EXPECT_FALSE(FunctionType::classof(&Integer));
    EXPECT_FALSE(IntegerType::classof(Base));
    EXPECT_EQ(&Function.returnType(), &Void);
    ASSERT_EQ(Function.parameters().size(), 2U);
    EXPECT_EQ(&Function.parameters()[0].type(), &Integer);
    EXPECT_EQ(Function.parameters()[0].name(), Names.intern("count"));
    EXPECT_EQ(&Function.parameters()[1].type(), &Bool);
    EXPECT_EQ(Function.parameters()[1].name(), Names.intern("ready"));
  }

  // Empty signatures and function-typed parameters retain their borrowed return and parameter types.
  TEST(IRFunctionTypeTest, SupportsEmptyAndNestedFunctionSignatures)
  {
    auto &Names = core::NamePool::instance();
    const VoidType Void;
    const FunctionType Empty(Void);
    EXPECT_TRUE(Empty.parameters().empty());
    EXPECT_EQ(&Empty.returnType(), &Void);
    const Parameter Parameters[] = {
        Parameter(Empty, Names.intern("callback")),
    };
    const FunctionType Nested(Empty, Parameters);
    EXPECT_EQ(&Nested.returnType(), &Empty);
    ASSERT_EQ(Nested.parameters().size(), 1U);
    EXPECT_EQ(&Nested.parameters()[0].type(), &Empty);
    EXPECT_EQ(Nested.parameters()[0].name(), Names.intern("callback"));
  }

  // A signature copies parameter metadata and retains interned names after input vectors and source strings are destroyed.
  TEST(IRFunctionTypeTest, CopiesParameterMetadataAndRetainsInternedNamesAfterInputsAreDestroyed)
  {
    auto &Names = core::NamePool::instance();
    const IntegerType Integer(IntegerType::Width::Bits64, IntegerType::Signedness::Unsigned);
    const auto MakeFunction = [&Integer, &Names]()
    {
      std::string Name = "a_parameter_name_long_enough_to_require_owned_string_storage";
      std::vector<Parameter> Parameters;
      Parameters.emplace_back(Integer, Names.intern(Name));
      return FunctionType(Integer, Parameters);
    };
    const FunctionType Function = MakeFunction();
    ASSERT_EQ(Function.parameters().size(), 1U);
    EXPECT_EQ(Function.parameters()[0].name(), Names.intern("a_parameter_name_long_enough_to_require_owned_string_storage"));
    EXPECT_EQ(Names.text(Function.parameters()[0].name()), "a_parameter_name_long_enough_to_require_owned_string_storage");
    EXPECT_EQ(&Function.parameters()[0].type(), &Integer);
    EXPECT_EQ(&Function.returnType(), &Integer);
  }

  // Function values and transparent keys agree on parameter names and kinds, ordering, and borrowed type identity.
  TEST(IRFunctionTypeTest, MatchesTransparentKeysByCompleteSignatureMetadata)
  {
    auto &Names = core::NamePool::instance();
    const IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType OtherInteger(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Bool;
    const Parameter Parameters[] = {
        Parameter(Integer, Names.intern("first")),
        Parameter(Bool, Names.intern("second")),
    };
    const Parameter MatchingParameters[] = {
        Parameter(Integer, Names.intern("first")),
        Parameter(Bool, Names.intern("second")),
    };
    const Parameter RenamedParameters[] = {
        Parameter(Integer, Names.intern("renamed")),
        Parameter(Bool, Names.intern("second")),
    };
    const Parameter ReorderedParameters[] = {
        Parameter(Bool, Names.intern("second")),
        Parameter(Integer, Names.intern("first")),
    };
    const Parameter OtherTypeParameters[] = {
        Parameter(OtherInteger, Names.intern("first")),
        Parameter(Bool, Names.intern("second")),
    };
    const Parameter NamedParameters[] = {
        Parameter(Integer, Names.intern("first"), Parameter::Kind::Named),
        Parameter(Bool, Names.intern("second")),
    };
    const Parameter VariadicParameters[] = {
        Parameter(Integer, Names.intern("first"), Parameter::Kind::Variadic),
        Parameter(Bool, Names.intern("second")),
    };
    const FunctionType Function(Integer, Parameters);
    const FunctionType Matching(Integer, MatchingParameters);
    const FunctionType Renamed(Integer, RenamedParameters);
    const FunctionType::Key MatchingKey{&Integer, MatchingParameters};
    const FunctionType::Key OtherKeys[] = {
        {&OtherInteger, Parameters},
        {&Integer, RenamedParameters},
        {&Integer, ReorderedParameters},
        {&Integer, OtherTypeParameters},
        {&Integer, NamedParameters},
        {&Integer, VariadicParameters},
        {&Integer, std::span<const Parameter>(Parameters, 1)},
        {&Integer, {}},
    };
    const core::Hash<FunctionType> Hash;
    const std::equal_to<> Equal;
    EXPECT_TRUE(Function == Matching);
    EXPECT_TRUE(Function == MatchingKey);
    EXPECT_TRUE(MatchingKey == Function);
    EXPECT_EQ(Hash(Function), Hash(Matching));
    EXPECT_EQ(Hash(Function), Hash(MatchingKey));
    EXPECT_TRUE(Equal(Function, MatchingKey));
    EXPECT_TRUE(Equal(MatchingKey, Function));
    EXPECT_FALSE(Function == Renamed);
    for (const auto &OtherKey : OtherKeys)
    {
      EXPECT_FALSE(Function == OtherKey);
      EXPECT_FALSE(OtherKey == Function);
      EXPECT_FALSE(MatchingKey == OtherKey);
      EXPECT_FALSE(OtherKey == MatchingKey);
    }
  }

  // A default empty key and an explicit empty span share the same signature identity and hash.
  TEST(IRFunctionTypeTest, MatchesEmptySignaturesWithTransparentKeys)
  {
    const VoidType Void;
    const FunctionType Empty(Void);
    const FunctionType Other(Void, std::span<const Parameter>{});
    const FunctionType::Key Key{&Void, {}};
    EXPECT_TRUE(Empty == Other);
    EXPECT_TRUE(Empty == Key);
    EXPECT_TRUE(Key == Empty);
    EXPECT_EQ(Empty.hash(), Key.hash());
    EXPECT_EQ(Empty.hash(), Other.hash());
  }
} // namespace ink::ir::test
