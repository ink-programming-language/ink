#include "ink/ir/type/value.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

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
    const auto *Array = Types.typePool().getArrayType(Types.typePool().getBoolType(), 1);
    ASSERT_NE(Array, nullptr);
    const auto *StringType = Types.typePool().getArrayType(Types.typePool().getUInt8Type(), 5);
    ASSERT_NE(StringType, nullptr);
    const TypeQueryCase Cases[] = {
        {Constants.getBoolConstant(true), Types.typePool().getBoolType()},
        {Constants.getSignedIntegerConstant(Types.typePool().getInt32Type(), 42), Types.typePool().getInt32Type()},
        {Constants.getFloatConstant(3.5F), Types.typePool().getFloat32Type()},
        {Constants.getArrayConstant(Array, {Constants.getBoolConstant(false)}), Array},
        {Constants.getNullPointerConstant(), Types.typePool().getNullPointerType()},
        {Constants.getStringConstant("value"), StringType},
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
    const auto *Signature = Types.typePool().getFunctionType(Types.typePool().getInt32Type(), {Types.typePool().getBoolType()});
    ASSERT_NE(Signature, nullptr);
    const auto Name = core::NamePool::instance().intern("IRValueTest.Named");
    ASSERT_TRUE(Name.valid());
    Function Callable(*Signature, Name);
    auto *ParameterValue = Callable.parameter(0);
    ASSERT_NE(ParameterValue, nullptr);
    ParameterValue->setName(Name);
    GlobalVariable Global(*Types.typePool().getBoolType(), Name);
    const auto *Initializer = Types.constantPool().getBoolConstant(true);
    ASSERT_TRUE(Global.setInitializer(Initializer));
    const Value &CallableValue = Callable;
    const Value &FormalValue = *ParameterValue;
    const Value &GlobalValue = Global;
    EXPECT_EQ(CallableValue.type(), Signature);
    EXPECT_EQ(Callable.type(), Signature);
    EXPECT_EQ(Callable.type(), Callable.functionType());
    EXPECT_EQ(FormalValue.type(), Types.typePool().getBoolType());
    EXPECT_EQ(GlobalValue.type(), Types.typePool().getBoolType());
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
