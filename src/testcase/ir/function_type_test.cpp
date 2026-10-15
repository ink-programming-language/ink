#include "ink/target/target_context.h"
#include "ink/ir/type/structure_type/array_type.h"
#include "ink/ir/type/builtin_type/bool_type.h"
#include "ink/ir/type/builtin_type/float_type.h"
#include "ink/ir/type/structure_type/function_type.h"
#include "ink/ir/type/builtin_type/integer_type.h"
#include "ink/ir/type/structure_type/pointer_type.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    void expectParametersEqual(const FunctionType &Function, const std::vector<std::pair<const Type *, bool>> &Expected)
    {
      const auto Actual = Function.parameters();
      ASSERT_EQ(Actual.size(), Expected.size());
      for (std::size_t Index = 0; Index < Expected.size(); ++Index)
      {
        SCOPED_TRACE(Index);
        EXPECT_EQ(Actual[Index].ParameterType, Expected[Index].first);
        EXPECT_EQ(Actual[Index].Variadic, Expected[Index].second);
      }
    }
  } // namespace

  static_assert(!std::is_base_of_v<Value, FunctionTypeParameter>);
  static_assert(std::is_copy_constructible_v<FunctionTypeParameter> && std::is_copy_assignable_v<FunctionTypeParameter>);
  static_assert(std::is_same_v<decltype(std::declval<const FunctionType &>().parameters()), std::span<const FunctionTypeParameter>>);

  // Omitting parameters creates a valid zero-argument signature with the supplied return type.
  TEST(IRFunctionTypeTest, EmptySignaturePreservesReturnType)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Bool = Types.typePool().getBoolType();
    const auto Function = Types.typePool().getFunctionType(Bool);
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function->kind(), ValueKind::FunctionType);
    EXPECT_EQ(Function->returnType(), Bool);
    EXPECT_TRUE(Function->parameters().empty());
    EXPECT_EQ(Function->parameterCount(), 0U);
  }

  // Mixed parameter descriptors retain ordered types and variadic flags, including repeated inputs.
  TEST(IRFunctionTypeTest, SignaturePreservesParameterOrderAndDuplicates)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Bool = Types.typePool().getBoolType();
    const auto Integer = Types.typePool().getIntegerType(32, true);
    const auto Float = Types.typePool().getFloatType(64);
    const auto Result = Types.typePool().getArrayType(Types.typePool().getInt64Type(), 3);
    ASSERT_NE(Integer, nullptr);
    ASSERT_NE(Float, nullptr);
    ASSERT_NE(Result, nullptr);
    const FunctionTypeParameter First(Integer);
    const FunctionTypeParameter Second(Bool, true);
    const FunctionTypeParameter Third(Float);
    const std::vector<FunctionTypeParameter> Parameters = {First, Second, Third, First};
    const auto Function = Types.typePool().getFunctionType(Result, Parameters);
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function->returnType(), Result);
    EXPECT_EQ(Function->parameterCount(), Parameters.size());
    expectParametersEqual(*Function, {{Integer, false}, {Bool, true}, {Float, false}, {Integer, false}});
  }

  // Source descriptor mutation and destruction leave the canonical signature and its cache identity unchanged.
  TEST(IRFunctionTypeTest, SignatureOwnsCopiedDescriptors)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Bool = Types.typePool().getBoolType();
    const auto Integer = Types.typePool().getIntegerType(32, false);
    ASSERT_NE(Integer, nullptr);
    const FunctionType *Function = nullptr;
    {
      std::vector<FunctionTypeParameter> Parameters = {Bool, {Integer, true}, Bool};
      Function = Types.typePool().getFunctionType(Integer, Parameters);
      ASSERT_NE(Function, nullptr);
      Parameters[0].ParameterType = Integer;
      Parameters[1].Variadic = false;
      expectParametersEqual(*Function, {{Bool, false}, {Integer, true}, {Bool, false}});
      const auto ModifiedSignature = Types.typePool().getFunctionType(Integer, Parameters);
      ASSERT_NE(ModifiedSignature, nullptr);
      EXPECT_NE(Function, ModifiedSignature);
      Parameters.clear();
      expectParametersEqual(*Function, {{Bool, false}, {Integer, true}, {Bool, false}});
    }
    expectParametersEqual(*Function, {{Bool, false}, {Integer, true}, {Bool, false}});
    EXPECT_EQ(Function->parameterCount(), 3U);
    EXPECT_EQ(Function->returnType(), Integer);
    EXPECT_EQ(Function, Types.typePool().getFunctionType(Integer, {Bool, {Integer, true}, Bool}));
  }

  // Temporary initializer-list descriptors can expire while the signature retains their types and variadic flags.
  TEST(IRFunctionTypeTest, SignatureOwnsInitializerListDescriptors)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Bool = Types.typePool().getBoolType();
    const auto Pointer = Types.typePool().getPointerType(Bool);
    ASSERT_NE(Pointer, nullptr);
    const FunctionType *Function = Types.typePool().getFunctionType(Bool, {Pointer, {Bool, true}});
    ASSERT_NE(Function, nullptr);
    expectParametersEqual(*Function, {{Pointer, false}, {Bool, true}});
    EXPECT_EQ(Function->parameterCount(), 2U);
    EXPECT_EQ(Function->returnType(), Bool);
  }

  // Canonical signature identity distinguishes the variadic flag at every ordered parameter position.
  TEST(IRFunctionTypeTest, CanonicalizationIncludesOrderedVariadicFlags)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Integer = Types.typePool().getInt32Type();
    const auto Fixed = Types.typePool().getFunctionType(Integer, {Integer, Integer});
    const auto FirstVariadic = Types.typePool().getFunctionType(Integer, {{Integer, true}, Integer});
    const auto LastVariadic = Types.typePool().getFunctionType(Integer, {Integer, {Integer, true}});
    const auto BothVariadic = Types.typePool().getFunctionType(Integer, {{Integer, true}, {Integer, true}});
    ASSERT_NE(Fixed, nullptr);
    ASSERT_NE(FirstVariadic, nullptr);
    ASSERT_NE(LastVariadic, nullptr);
    ASSERT_NE(BothVariadic, nullptr);
    EXPECT_NE(Fixed, FirstVariadic);
    EXPECT_NE(Fixed, LastVariadic);
    EXPECT_NE(Fixed, BothVariadic);
    EXPECT_NE(FirstVariadic, LastVariadic);
    EXPECT_NE(FirstVariadic, BothVariadic);
    EXPECT_NE(LastVariadic, BothVariadic);
    const FunctionTypeParameter Repeated(Integer, true);
    const std::vector<FunctionTypeParameter> Parameters = {Repeated, Repeated};
    EXPECT_EQ(BothVariadic, Types.typePool().getFunctionType(Integer, Parameters));
    EXPECT_EQ(FirstVariadic, Types.typePool().getFunctionType(Integer, {{Integer, true}, Integer}));
  }

  // Missing return types and null fixed or variadic parameter types fail at every parameter position.
  TEST(IRFunctionTypeTest, RejectsNullSignatureTypes)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Bool = Types.typePool().getBoolType();
    EXPECT_EQ(Types.typePool().getFunctionType(nullptr), nullptr);
    EXPECT_EQ(Types.typePool().getFunctionType(nullptr, {Bool}), nullptr);
    EXPECT_EQ(Types.typePool().getFunctionType(Bool, {nullptr}), nullptr);
    EXPECT_EQ(Types.typePool().getFunctionType(Bool, {{nullptr, true}}), nullptr);
    const FunctionTypeParameter Valid(Bool);
    const FunctionTypeParameter MissingType(nullptr);
    const FunctionTypeParameter MissingVariadicType(nullptr, true);
    for (std::size_t Index = 0; Index < 3; ++Index)
    {
      SCOPED_TRACE(Index);
      std::vector<FunctionTypeParameter> Parameters(3, Valid);
      for (const FunctionTypeParameter &Invalid : {MissingType, MissingVariadicType})
      {
        Parameters[Index] = Invalid;
        EXPECT_EQ(Types.typePool().getFunctionType(Bool, Parameters), nullptr);
      }
    }
  }

  // Function types expose a target-sized address through Type and retain callable identity through Value.
  TEST(IRFunctionTypeTest, CallableLayoutUsesTargetPointerWidth)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      IRContext TypesContext(Target);
      Module Types(TypesContext);
      const auto Aggregate = Types.typePool().getArrayType(Types.typePool().getInt64Type(), 3);
      ASSERT_NE(Aggregate, nullptr);
      const auto Function = Types.typePool().getFunctionType(Aggregate, {Aggregate, Aggregate});
      ASSERT_NE(Function, nullptr);
      const Type *TypeValue = Function;
      const Value *ValueValue = Function;
      EXPECT_EQ(TypeValue->kind(), ValueKind::FunctionType);
      EXPECT_EQ(TypeValue->size(), Target.pointerByteWidth());
      EXPECT_EQ(TypeValue->alignment(), Target.pointerByteWidth());
      EXPECT_EQ(ValueValue->kind(), ValueKind::FunctionType);
      EXPECT_TRUE(Type::classof(ValueValue));
      EXPECT_TRUE(FunctionType::classof(ValueValue));
    }
  }

  // Higher-order signatures retain callable return and parameter types as well as pointers to callable types.
  TEST(IRFunctionTypeTest, SupportsNestedCallableSignatures)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Bool = Types.typePool().getBoolType();
    const auto Callback = Types.typePool().getFunctionType(Bool, {Bool});
    ASSERT_NE(Callback, nullptr);
    const auto CallbackPointer = Types.typePool().getPointerType(Callback);
    const auto Factory = Types.typePool().getFunctionType(Callback, {Callback, CallbackPointer});
    ASSERT_NE(Factory, nullptr);
    EXPECT_EQ(Factory->returnType(), Callback);
    ASSERT_EQ(Factory->parameterCount(), 2U);
    EXPECT_EQ(Factory->parameters()[0].ParameterType, Callback);
    EXPECT_EQ(Factory->parameters()[1].ParameterType, CallbackPointer);
    EXPECT_EQ(CallbackPointer->pointeeType(), Callback);
  }

  // Function signatures reject foreign return types and foreign fixed or variadic parameter types at any position.
  TEST(IRFunctionTypeTest, RejectsForeignSignatureTypes)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    IRContext OtherTypesContext(Target64);
    const Module OtherTypes(OtherTypesContext);
    const auto Bool = Types.typePool().getBoolType();
    const auto Foreign = OtherTypes.typePool().getBoolType();
    EXPECT_EQ(Types.typePool().getFunctionType(Foreign), nullptr);
    EXPECT_EQ(Types.typePool().getFunctionType(Foreign, {Bool}), nullptr);
    const FunctionTypeParameter LocalParameter(Bool);
    const FunctionTypeParameter ForeignParameter(Foreign);
    const FunctionTypeParameter ForeignVariadicParameter(Foreign, true);
    for (std::size_t Index = 0; Index < 3; ++Index)
    {
      SCOPED_TRACE(Index);
      std::vector<FunctionTypeParameter> Parameters(3, LocalParameter);
      for (const FunctionTypeParameter &Invalid : {ForeignParameter, ForeignVariadicParameter})
      {
        Parameters[Index] = Invalid;
        EXPECT_EQ(Types.typePool().getFunctionType(Bool, Parameters), nullptr);
      }
    }
  }
} // namespace ink::ir::test
