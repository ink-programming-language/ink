#include "ink/core/support/hash.h"
#include "ink/ir/type/bool_type.h"
#include "ink/ir/type/function_type.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/void_type.h"

#include <gtest/gtest.h>

#include <functional>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir::test
{
  static_assert(!std::is_base_of_v<Value, Parameter>);
  static_assert(std::is_trivially_copyable_v<Parameter>);
  static_assert(std::is_copy_constructible_v<Parameter>);
  static_assert(std::is_copy_assignable_v<Parameter>);
  static_assert(std::is_same_v<decltype(std::declval<const Parameter &>().type()), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<const Parameter &>().kind()), Parameter::Kind>);
  static_assert(!std::is_constructible_v<Parameter, IntegerType &&>);
  static_assert(!std::is_constructible_v<Parameter, const IntegerType &&>);
  static_assert(!std::is_constructible_v<Parameter, IntegerType &&, Parameter::Kind>);
  static_assert(!std::is_constructible_v<Parameter, const IntegerType &&, Parameter::Kind>);
  static_assert(!std::is_constructible_v<FunctionType, IntegerType &&>);
  static_assert(!std::is_constructible_v<FunctionType, const IntegerType &&>);
  static_assert(!std::is_constructible_v<FunctionType, IntegerType &&, std::span<const Parameter>>);
  static_assert(!std::is_constructible_v<FunctionType, const IntegerType &&, std::span<const Parameter>>);
  static_assert(!std::is_copy_constructible_v<FunctionType>);
  static_assert(!std::is_move_constructible_v<FunctionType>);

  // Copying parameter metadata preserves its borrowed type and kind, while assigning a copy leaves the original unchanged.
  TEST(IRFunctionTypeTest, CopiesParameterMetadataWithoutCreatingValueIdentity)
  {
    const IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Bool;
    const Parameter Named(Integer, Parameter::Kind::Named);
    EXPECT_EQ(&Named.type(), &Integer);
    EXPECT_EQ(Named.kind(), Parameter::Kind::Named);
    Parameter Copy = Named;
    EXPECT_EQ(&Copy.type(), &Integer);
    EXPECT_EQ(Copy.kind(), Parameter::Kind::Named);
    Copy = Parameter(Bool);
    EXPECT_EQ(&Copy.type(), &Bool);
    EXPECT_EQ(Copy.kind(), Parameter::Kind::Positional);
    EXPECT_EQ(&Named.type(), &Integer);
    EXPECT_EQ(Named.kind(), Parameter::Kind::Named);
  }

  // Signatures retain positional, named, and variadic kinds independently of their shared parameter type.
  TEST(IRFunctionTypeTest, PreservesExplicitParameterKinds)
  {
    const IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const Parameter Parameters[] = {
        Parameter(Integer),
        Parameter(Integer, Parameter::Kind::Named),
        Parameter(Integer, Parameter::Kind::Variadic),
    };
    const FunctionType Function(Integer, Parameters);
    ASSERT_EQ(Function.parameters().size(), 3U);
    EXPECT_EQ(Function.parameters()[0].kind(), Parameter::Kind::Positional);
    EXPECT_EQ(Function.parameters()[1].kind(), Parameter::Kind::Named);
    EXPECT_EQ(Function.parameters()[2].kind(), Parameter::Kind::Variadic);
    for (const auto &ParameterValue : Function.parameters())
    {
      EXPECT_EQ(&ParameterValue.type(), &Integer);
    }
  }

  // Function types preserve ordered parameter metadata and expose their distinct kind through Type.
  TEST(IRFunctionTypeTest, IdentifiesFunctionsAndPreservesOrderedSignatures)
  {
    const IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Bool;
    const VoidType Void;
    const Parameter Parameters[] = {
        Parameter(Integer),
        Parameter(Bool, Parameter::Kind::Named),
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
    EXPECT_EQ(Function.parameters()[0].kind(), Parameter::Kind::Positional);
    EXPECT_EQ(&Function.parameters()[1].type(), &Bool);
    EXPECT_EQ(Function.parameters()[1].kind(), Parameter::Kind::Named);
  }

  // Empty signatures and function-typed parameters retain their borrowed return and parameter types.
  TEST(IRFunctionTypeTest, SupportsEmptyAndNestedFunctionSignatures)
  {
    const VoidType Void;
    const FunctionType Empty(Void);
    EXPECT_TRUE(Empty.parameters().empty());
    EXPECT_EQ(&Empty.returnType(), &Void);
    const Parameter Parameters[] = {
        Parameter(Empty),
    };
    const FunctionType Nested(Empty, Parameters);
    EXPECT_EQ(&Nested.returnType(), &Empty);
    ASSERT_EQ(Nested.parameters().size(), 1U);
    EXPECT_EQ(&Nested.parameters()[0].type(), &Empty);
    EXPECT_EQ(Nested.parameters()[0].kind(), Parameter::Kind::Positional);
  }

  // Signatures own parameter metadata snapshots after the source vector is edited and destroyed.
  TEST(IRFunctionTypeTest, CopiesParameterMetadataBeforeInputsAreChangedAndDestroyed)
  {
    const IntegerType Integer(IntegerType::Width::Bits64, IntegerType::Signedness::Unsigned);
    const BoolType Bool;
    const auto MakeFunction = [&Integer, &Bool]()
    {
      std::vector<Parameter> Parameters;
      Parameters.emplace_back(Integer, Parameter::Kind::Variadic);
      FunctionType Function(Integer, Parameters);
      Parameters[0] = Parameter(Bool, Parameter::Kind::Named);
      EXPECT_EQ(&Function.parameters()[0].type(), &Integer);
      EXPECT_EQ(Function.parameters()[0].kind(), Parameter::Kind::Variadic);
      return FunctionType(Integer, Function.parameters());
    };
    const FunctionType Function = MakeFunction();
    ASSERT_EQ(Function.parameters().size(), 1U);
    EXPECT_EQ(&Function.parameters()[0].type(), &Integer);
    EXPECT_EQ(Function.parameters()[0].kind(), Parameter::Kind::Variadic);
    EXPECT_EQ(&Function.returnType(), &Integer);
  }

  // Function values and transparent keys agree on parameter kinds, ordering, and borrowed type identity.
  TEST(IRFunctionTypeTest, MatchesTransparentKeysByCompleteSignatureMetadata)
  {
    const IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType OtherInteger(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Bool;
    const Parameter Parameters[] = {
        Parameter(Integer),
        Parameter(Bool),
    };
    const Parameter MatchingParameters[] = {
        Parameter(Integer),
        Parameter(Bool),
    };
    const Parameter ReorderedParameters[] = {
        Parameter(Bool),
        Parameter(Integer),
    };
    const Parameter OtherTypeParameters[] = {
        Parameter(OtherInteger),
        Parameter(Bool),
    };
    const Parameter NamedParameters[] = {
        Parameter(Integer, Parameter::Kind::Named),
        Parameter(Bool),
    };
    const Parameter VariadicParameters[] = {
        Parameter(Integer, Parameter::Kind::Variadic),
        Parameter(Bool),
    };
    const FunctionType Function(Integer, Parameters);
    const FunctionType Matching(Integer, MatchingParameters);
    const FunctionType::Key MatchingKey{&Integer, MatchingParameters};
    const FunctionType::Key OtherKeys[] = {
        {&OtherInteger, Parameters},
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

  // Call ABI is copied into signatures and participates in both value and transparent-key identity.
  TEST(IRFunctionTypeTest, DistinguishesABIAndCallingConventionInSignaturesAndKeys)
  {
    const VoidType Void;
    const FunctionType Ink(Void);
    const FunctionType C(Void, {}, {ABIKind::C, CallingConvention::Default});
    CallABI Contract{ABIKind::C, CallingConvention::StdCall};
    const FunctionType StdCall(Void, {}, Contract);
    Contract = {};
    const FunctionType Matching(Void, {}, {ABIKind::C, CallingConvention::StdCall});
    const FunctionType::Key InkKey{&Void, {}};
    const FunctionType::Key CKey{&Void, {}, {ABIKind::C, CallingConvention::Default}};
    const FunctionType::Key StdCallKey{&Void, {}, {ABIKind::C, CallingConvention::StdCall}};
    EXPECT_EQ(Ink.callABI(), CallABI{});
    EXPECT_EQ(Ink.abi(), ABIKind::Ink);
    EXPECT_EQ(Ink.callingConvention(), CallingConvention::Default);
    EXPECT_EQ(C.abi(), ABIKind::C);
    EXPECT_EQ(C.callingConvention(), CallingConvention::Default);
    EXPECT_EQ(StdCall.abi(), ABIKind::C);
    EXPECT_EQ(StdCall.callingConvention(), CallingConvention::StdCall);
    EXPECT_TRUE(Ink == InkKey);
    EXPECT_TRUE(C == CKey);
    EXPECT_TRUE(StdCall == Matching);
    EXPECT_TRUE(StdCall == StdCallKey);
    EXPECT_TRUE(StdCallKey == StdCall);
    EXPECT_EQ(StdCall.hash(), Matching.hash());
    EXPECT_EQ(StdCall.hash(), StdCallKey.hash());
    EXPECT_FALSE(Ink == C);
    EXPECT_FALSE(C == StdCall);
    EXPECT_FALSE(Ink == CKey);
    EXPECT_FALSE(CKey == InkKey);
    EXPECT_FALSE(C == StdCallKey);
    EXPECT_FALSE(StdCallKey == CKey);
  }
} // namespace ink::ir::test
