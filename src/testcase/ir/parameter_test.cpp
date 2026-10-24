#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/param/parameter.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <type_traits>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  static_assert(std::is_base_of_v<Value, Parameter>);
  static_assert(!std::is_base_of_v<Type, Parameter>);
  static_assert(!std::is_constructible_v<Parameter, const Type *, bool>);
  static_assert(!std::is_constructible_v<Parameter, Function &, std::size_t>);
  static_assert(!std::is_copy_constructible_v<Parameter> && !std::is_move_constructible_v<Parameter>);

  // Function construction inherits descriptor types and variadic flags while creating owned unnamed formal values.
  TEST(IRParameterTest, DerivesOrderedTypesAndOwnershipFromSignature)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Bool = Types.typePool().get<ink::ir::BoolType>();
    const auto Integer = Types.typePool().get<ink::ir::IntegerType>(32, true);
    const FunctionTypeParameter First(Integer);
    const FunctionTypeParameter Second(Bool, true);
    const std::vector<FunctionTypeParameter> Parameters = {First, Second, First};
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Bool, Parameters);
    ASSERT_NE(Signature, nullptr);
    Function Callable(*Signature);
    const Function &ReadOnly = Callable;
    ASSERT_EQ(Callable.parameterCount(), Parameters.size());
    ASSERT_EQ(Callable.parameters().size(), Parameters.size());
    for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
    {
      SCOPED_TRACE(Index);
      auto *ParameterValue = Callable.parameter(Index);
      const auto *ReadOnlyParameter = ReadOnly.parameter(Index);
      ASSERT_NE(ParameterValue, nullptr);
      ASSERT_NE(ReadOnlyParameter, nullptr);
      EXPECT_EQ(Callable.parameters()[Index].get(), ParameterValue);
      EXPECT_EQ(ReadOnlyParameter, ParameterValue);
      EXPECT_EQ(ParameterValue->function(), &Callable);
      EXPECT_EQ(ReadOnlyParameter->function(), &ReadOnly);
      EXPECT_EQ(ParameterValue->index(), Index);
      EXPECT_EQ(ParameterValue->type(), Parameters[Index].ParameterType);
      EXPECT_EQ(ParameterValue->isVariadic(), Parameters[Index].Variadic);
      EXPECT_EQ(ParameterValue->name(), core::NAME_None);
      EXPECT_FALSE(ParameterValue->hasName());
    }
    EXPECT_EQ(Callable.parameter(Parameters.size()), nullptr);
    EXPECT_EQ(ReadOnly.parameter(Parameters.size()), nullptr);
  }

  // Functions sharing a signature own distinct formal values whose local names can change independently.
  TEST(IRParameterTest, SharedSignatureCreatesIndependentParameterValues)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {{Types.typePool().get<ink::ir::IntegerType>(32, true), true}, Types.typePool().get<ink::ir::BoolType>()});
    ASSERT_NE(Signature, nullptr);
    Function First(*Signature);
    Function Second(*Signature);
    EXPECT_EQ(First.functionType(), Second.functionType());
    ASSERT_EQ(First.parameterCount(), 2U);
    ASSERT_EQ(Second.parameterCount(), 2U);
    for (std::size_t Index = 0; Index < First.parameterCount(); ++Index)
    {
      SCOPED_TRACE(Index);
      const auto *FirstParameter = First.parameter(Index);
      const auto *SecondParameter = Second.parameter(Index);
      ASSERT_NE(FirstParameter, nullptr);
      ASSERT_NE(SecondParameter, nullptr);
      EXPECT_NE(FirstParameter, SecondParameter);
      EXPECT_EQ(FirstParameter->function(), &First);
      EXPECT_EQ(SecondParameter->function(), &Second);
      EXPECT_EQ(FirstParameter->type(), SecondParameter->type());
      EXPECT_EQ(FirstParameter->isVariadic(), SecondParameter->isVariadic());
      EXPECT_EQ(FirstParameter->isVariadic(), Index == 0);
      EXPECT_EQ(FirstParameter->index(), SecondParameter->index());
    }
    const core::Name FirstName = core::NamePool::instance().intern("IRParameterTest.First");
    const core::Name SecondName = core::NamePool::instance().intern("IRParameterTest.Second");
    ASSERT_TRUE(FirstName.valid());
    ASSERT_TRUE(SecondName.valid());
    First.parameter(0)->setName(FirstName);
    EXPECT_TRUE(First.parameter(0)->hasName());
    EXPECT_FALSE(Second.parameter(0)->hasName());
    Second.parameter(0)->setName(SecondName);
    EXPECT_EQ(First.parameter(0)->name(), FirstName);
    EXPECT_EQ(Second.parameter(0)->name(), SecondName);
    First.parameter(0)->setName(core::NAME_None);
    EXPECT_FALSE(First.parameter(0)->hasName());
    EXPECT_EQ(Second.parameter(0)->name(), SecondName);
    EXPECT_EQ(Signature->parameters()[0].ParameterType, Types.typePool().get<ink::ir::IntegerType>(32, true));
    EXPECT_TRUE(Signature->parameters()[0].Variadic);
    EXPECT_EQ(First.functionType(), Second.functionType());
    EXPECT_EQ(Signature, Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {{Types.typePool().get<ink::ir::IntegerType>(32, true), true}, Types.typePool().get<ink::ir::BoolType>()}));
  }

  // A signature built from a formal value's type metadata survives renaming and destruction of the source function.
  TEST(IRParameterTest, SignatureFromParameterMetadataOutlivesSourceFunction)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Integer = Types.typePool().get<ink::ir::IntegerType>(32, true);
    const auto Void = Types.typePool().get<ink::ir::VoidType>();
    const auto SourceSignature = Types.typePool().get<ink::ir::FunctionType>(Void, {Integer, {Integer, true}});
    ASSERT_NE(SourceSignature, nullptr);
    const FunctionType *Signature = nullptr;
    {
      Function Source(*SourceSignature);
      auto *Formal = Source.parameter(1);
      ASSERT_NE(Formal, nullptr);
      Formal->setName(core::NamePool::instance().intern("IRParameterTest.Source"));
      ASSERT_TRUE(Formal->hasName());
      const std::vector<FunctionTypeParameter> Parameters = {{Formal->type(), Formal->isVariadic()}};
      Signature = Types.typePool().get<ink::ir::FunctionType>(Integer, Parameters);
      ASSERT_NE(Signature, nullptr);
      ASSERT_EQ(Signature->parameterCount(), 1U);
      Formal->setName(core::NamePool::instance().intern("IRParameterTest.RenamedSource"));
      EXPECT_EQ(Signature, Types.typePool().get<ink::ir::FunctionType>(Integer, Parameters));
    }
    EXPECT_EQ(Signature->parameters()[0].ParameterType, Integer);
    EXPECT_TRUE(Signature->parameters()[0].Variadic);
    EXPECT_EQ(Signature, Types.typePool().get<ink::ir::FunctionType>(Integer, {{Integer, true}}));
  }

  // Parameters classify as runtime values without being confused with functions, types or constants.
  TEST(IRParameterTest, ParameterClassificationExcludesOtherValueKinds)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Types.typePool().get<ink::ir::IntegerType>(32, true)});
    ASSERT_NE(Signature, nullptr);
    Function Callable(*Signature);
    const Value *ParameterValue = Callable.parameter(0);
    ASSERT_NE(ParameterValue, nullptr);
    EXPECT_EQ(ParameterValue->kind(), ValueKind::Parameter);
    EXPECT_TRUE(Parameter::classof(ParameterValue));
    EXPECT_FALSE(Type::classof(ParameterValue));
    EXPECT_FALSE(Constant::classof(ParameterValue));
    EXPECT_FALSE(Function::classof(ParameterValue));
    EXPECT_FALSE(FunctionType::classof(ParameterValue));
    EXPECT_FALSE(Parameter::classof(&Callable));
    EXPECT_FALSE(Parameter::classof(Signature));
    EXPECT_FALSE(Parameter::classof(Types.typePool().get<ink::ir::IntegerType>(32, true)));
    EXPECT_FALSE(Parameter::classof(nullptr));
  }
} // namespace ink::ir::test
