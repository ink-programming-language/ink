#include "ink/ir/type/value.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    struct TypeQueryCase
    {
        const Value *ValueObject;
        const Type *ExpectedType;
    };
  } // namespace

  static_assert(std::is_same_v<decltype(std::declval<const Value &>().type()), const Type *>);
  static_assert(noexcept(std::declval<const Value &>().type()));
  static_assert(std::is_same_v<decltype(std::declval<const Function &>().type()), const FunctionType *>);
  static_assert(noexcept(std::declval<const Function &>().type()));

  // Base Value pointers dynamically return each constant family's canonical type, including null literals and string arrays.
  TEST(IRValueTest, QueriesAllConstantFamilies)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    const auto *Array = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::BoolType>(), 1);
    ASSERT_NE(Array, nullptr);
    const auto *StringType = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(8, false), 5);
    ASSERT_NE(StringType, nullptr);
    const TypeQueryCase Cases[] = {
        {Constants.get<ink::ir::BoolConstant>(true), Types.typePool().get<ink::ir::BoolType>()},
        {Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(42)), Types.typePool().get<ink::ir::IntegerType>(32, true)},
        {Constants.get<ink::ir::FloatConstant>(3.5F), Types.typePool().get<ink::ir::FloatType>(32)},
        {Constants.get<ink::ir::ArrayConstant>(Array, {Constants.get<ink::ir::BoolConstant>(false)}), Array},
        {Constants.get<ink::ir::NullPointerConstant>(), Types.typePool().get<ink::ir::NullPointerType>()},
        {Constants.get<ink::ir::StringConstant>("value"), StringType},
    };
    for (const auto &Case : Cases)
    {
      ASSERT_NE(Case.ValueObject, nullptr);
      EXPECT_EQ(Case.ValueObject->type(), Case.ExpectedType);
      EXPECT_TRUE(Constants.owns(static_cast<const Constant *>(Case.ValueObject)));
    }
  }

  // Base Value references return function signatures, parameter types and global storage types without changing object state.
  TEST(IRValueTest, QueriesFunctionParameterAndGlobalWithoutMutation)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto *Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::IntegerType>(32, true), {Types.typePool().get<ink::ir::BoolType>()});
    ASSERT_NE(Signature, nullptr);
    const auto Name = core::NamePool::instance().intern("IRValueTest.Named");
    ASSERT_TRUE(Name.valid());
    Function Callable(*Signature, Name);
    auto *ParameterValue = Callable.parameter(0);
    ASSERT_NE(ParameterValue, nullptr);
    ParameterValue->setName(Name);
    GlobalVariable Global(*Types.typePool().get<ink::ir::BoolType>(), Name);
    const auto *Initializer = Types.constantPool().get<ink::ir::BoolConstant>(true);
    ASSERT_TRUE(Global.setInitializer(Initializer));
    const Value &CallableValue = Callable;
    const Value &FormalValue = *ParameterValue;
    const Value &GlobalValue = Global;
    EXPECT_EQ(CallableValue.type(), Signature);
    EXPECT_EQ(Callable.type(), Signature);
    EXPECT_EQ(Callable.type(), Callable.functionType());
    EXPECT_EQ(FormalValue.type(), Types.typePool().get<ink::ir::BoolType>());
    EXPECT_EQ(GlobalValue.type(), Types.typePool().get<ink::ir::BoolType>());
    EXPECT_EQ(Callable.name(), Name);
    EXPECT_EQ(Callable.parameterCount(), 1U);
    EXPECT_EQ(Callable.blockCount(), 0U);
    EXPECT_EQ(ParameterValue->function(), &Callable);
    EXPECT_EQ(ParameterValue->index(), 0U);
    EXPECT_EQ(ParameterValue->name(), Name);
    EXPECT_EQ(Global.name(), Name);
    EXPECT_EQ(Global.initializer(), Initializer);
  }
} // namespace ink::ir::test
