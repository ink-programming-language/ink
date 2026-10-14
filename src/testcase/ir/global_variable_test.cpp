#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  static_assert(std::is_base_of_v<Value, GlobalVariable>);
  static_assert(!std::is_base_of_v<Type, GlobalVariable>);
  static_assert(!std::is_base_of_v<Constant, GlobalVariable>);
  static_assert(!std::is_copy_constructible_v<GlobalVariable>);
  static_assert(!std::is_copy_assignable_v<GlobalVariable>);
  static_assert(!std::is_move_constructible_v<GlobalVariable>);
  static_assert(!std::is_move_assignable_v<GlobalVariable>);

  // A new global variable borrows its type and starts unnamed and uninitialized.
  TEST(IRGlobalVariableTest, DefaultsToUnnamedUninitializedVariable)
  {
    Module Types(Target);
    GlobalVariable Variable(*Types.typePool().getInt32Type());
    EXPECT_EQ(Variable.type(), Types.typePool().getInt32Type());
    EXPECT_EQ(Variable.name(), core::NAME_None);
    EXPECT_FALSE(Variable.hasName());
    EXPECT_EQ(Variable.initializer(), nullptr);
    EXPECT_FALSE(Variable.hasInitializer());
  }

  // Global variables classify distinctly from their types, initializer constants and function values.
  TEST(IRGlobalVariableTest, ClassifiesAsGlobalVariableOnly)
  {
    Module Types(Target);
    auto &Constants = Types.constantPool();
    GlobalVariable Variable(*Types.typePool().getBoolType());
    const auto Signature = Types.typePool().getFunctionType(Types.typePool().getVoidType());
    ASSERT_NE(Signature, nullptr);
    Function Callable(*Signature);
    const Value *Entity = &Variable;
    EXPECT_EQ(Entity->kind(), ValueKind::GlobalVariable);
    EXPECT_EQ(valueKindName(ValueKind::GlobalVariable), "global_variable");
    EXPECT_TRUE(GlobalVariable::classof(Entity));
    EXPECT_FALSE(Type::classof(Entity));
    EXPECT_FALSE(Constant::classof(Entity));
    EXPECT_FALSE(Function::classof(Entity));
    EXPECT_FALSE(GlobalVariable::classof(Types.typePool().getBoolType()));
    EXPECT_FALSE(GlobalVariable::classof(Constants.getBoolConstant(true)));
    EXPECT_FALSE(GlobalVariable::classof(&Callable));
    EXPECT_FALSE(GlobalVariable::classof(nullptr));
  }

  // Renaming or removing a global's local name preserves its type and initializer.
  TEST(IRGlobalVariableTest, ChangesNameIndependentlyOfTypeAndInitializer)
  {
    Module Types(Target);
    auto &Constants = Types.constantPool();
    const core::Name OriginalName = core::NamePool::instance().intern("IRGlobalVariableTest.Original");
    const core::Name NewName = core::NamePool::instance().intern("IRGlobalVariableTest.Renamed");
    ASSERT_TRUE(OriginalName.valid());
    ASSERT_TRUE(NewName.valid());
    GlobalVariable Variable(*Types.typePool().getBoolType(), OriginalName);
    const Constant *Initializer = Constants.getBoolConstant(true);
    ASSERT_TRUE(Variable.setInitializer(Initializer));
    EXPECT_TRUE(Variable.hasName());
    EXPECT_EQ(Variable.name(), OriginalName);
    Variable.setName(NewName);
    EXPECT_EQ(Variable.name(), NewName);
    EXPECT_EQ(Variable.initializer(), Initializer);
    Variable.setName(core::NAME_None);
    EXPECT_FALSE(Variable.hasName());
    EXPECT_EQ(Variable.name(), core::NAME_None);
    EXPECT_EQ(Variable.type(), Types.typePool().getBoolType());
    EXPECT_EQ(Variable.initializer(), Initializer);
  }

  // Initializers are borrowed, replaceable and clearable without transferring ownership from their constant pool.
  TEST(IRGlobalVariableTest, BorrowsReplacesAndClearsInitializer)
  {
    Module Types(Target);
    auto &Constants = Types.constantPool();
    const Constant *First = Constants.getBoolConstant(false);
    const Constant *Second = Constants.getBoolConstant(true);
    {
      GlobalVariable Variable(*Types.typePool().getBoolType());
      ASSERT_TRUE(Variable.setInitializer(First));
      EXPECT_TRUE(Variable.hasInitializer());
      EXPECT_EQ(Variable.initializer(), First);
      ASSERT_TRUE(Variable.setInitializer(Second));
      EXPECT_EQ(Variable.initializer(), Second);
      ASSERT_TRUE(Variable.setInitializer(nullptr));
      EXPECT_FALSE(Variable.hasInitializer());
      EXPECT_EQ(Variable.initializer(), nullptr);
      ASSERT_TRUE(Variable.setInitializer(First));
    }
    EXPECT_TRUE(Constants.owns(First));
    EXPECT_TRUE(Constants.owns(Second));
    EXPECT_EQ(First, Constants.getBoolConstant(false));
    EXPECT_EQ(Second, Constants.getBoolConstant(true));
  }

  // A different type or an equivalent type from another module's type pool is rejected while preserving the initializer.
  TEST(IRGlobalVariableTest, RejectsInitializerWithDifferentTypeIdentity)
  {
    Module Types(Target);
    Module ForeignTypes(Target);
    auto &Constants = Types.constantPool();
    auto &ForeignConstants = ForeignTypes.constantPool();
    GlobalVariable Variable(*Types.typePool().getBoolType());
    const Constant *Initializer = Constants.getBoolConstant(true);
    const Constant *WrongType = Constants.getSignedIntegerConstant(Types.typePool().getInt32Type(), 1);
    const Constant *ForeignInitializer = ForeignConstants.getBoolConstant(true);
    ASSERT_NE(WrongType, nullptr);
    ASSERT_NE(ForeignInitializer, nullptr);
    EXPECT_FALSE(Variable.setInitializer(WrongType));
    EXPECT_FALSE(Variable.hasInitializer());
    ASSERT_TRUE(Variable.setInitializer(Initializer));
    EXPECT_FALSE(Variable.setInitializer(WrongType));
    EXPECT_EQ(Variable.initializer(), Initializer);
    EXPECT_FALSE(Variable.setInitializer(ForeignInitializer));
    EXPECT_EQ(Variable.initializer(), Initializer);
    EXPECT_EQ(Variable.type(), Types.typePool().getBoolType());
  }
} // namespace ink::ir::test
