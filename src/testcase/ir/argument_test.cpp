#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/param/argument.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <type_traits>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  static_assert(!std::is_base_of_v<Value, Argument>);
  static_assert(std::is_copy_constructible_v<Argument>);
  static_assert(std::is_copy_assignable_v<Argument>);
  static_assert(!std::is_default_constructible_v<Argument>);
  static_assert(!std::is_convertible_v<const Value &, Argument>);

  // Separate calls supply different actual values while the callee retains the same formal parameter identity.
  TEST(IRArgumentTest, DifferentActualListsShareCalleeParameters)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Types.typePool().get<ink::ir::IntegerType>(32, true)});
    const auto FirstLiteral = Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(1));
    const auto SecondLiteral = Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(2));
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(FirstLiteral, nullptr);
    ASSERT_NE(SecondLiteral, nullptr);
    Function Callee(*Signature);
    const auto *Formal = Callee.parameter(0);
    ASSERT_NE(Formal, nullptr);
    const std::vector<Argument> FirstCall = {Argument(*FirstLiteral)};
    const std::vector<Argument> SecondCall = {Argument(*SecondLiteral)};
    EXPECT_NE(FirstCall[0].value(), SecondCall[0].value());
    EXPECT_EQ(FirstCall[0].value(), FirstLiteral);
    EXPECT_EQ(SecondCall[0].value(), SecondLiteral);
    EXPECT_EQ(Callee.parameter(0), Formal);
    EXPECT_EQ(Formal->function(), &Callee);
    EXPECT_EQ(Callee.parameterCount(), 1U);
    EXPECT_EQ(Callee.functionType(), Signature);
  }

  // Renaming actual arguments is independent of the names on their referenced formal parameter and function values.
  TEST(IRArgumentTest, LocalNamesDoNotFollowReferencedValueNames)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Types.typePool().get<ink::ir::IntegerType>(32, true)});
    ASSERT_NE(Signature, nullptr);
    const core::Name FunctionName = core::NamePool::instance().intern("IRArgumentTest.Callee");
    const core::Name FormalName = core::NamePool::instance().intern("IRArgumentTest.Formal");
    const core::Name ActualName = core::NamePool::instance().intern("IRArgumentTest.Actual");
    ASSERT_TRUE(FunctionName.valid());
    ASSERT_TRUE(FormalName.valid());
    ASSERT_TRUE(ActualName.valid());
    Function Callable(*Signature, FunctionName);
    auto *Formal = Callable.parameter(0);
    ASSERT_NE(Formal, nullptr);
    Formal->setName(FormalName);
    Argument ParameterArgument(*Formal, ActualName);
    Argument FunctionArgument(Callable);
    EXPECT_TRUE(ParameterArgument.hasName());
    EXPECT_EQ(ParameterArgument.name(), ActualName);
    EXPECT_FALSE(FunctionArgument.hasName());
    FunctionArgument.setName(ActualName);
    EXPECT_EQ(Formal->name(), FormalName);
    EXPECT_EQ(Callable.name(), FunctionName);
    Formal->setName(core::NAME_None);
    Callable.setName(core::NAME_None);
    EXPECT_EQ(ParameterArgument.name(), ActualName);
    EXPECT_EQ(FunctionArgument.name(), ActualName);
    ParameterArgument.setName(core::NAME_None);
    EXPECT_FALSE(ParameterArgument.hasName());
    EXPECT_EQ(FunctionArgument.name(), ActualName);
    EXPECT_EQ(ParameterArgument.value(), Formal);
    EXPECT_EQ(FunctionArgument.value(), &Callable);
  }
} // namespace ink::ir::test
