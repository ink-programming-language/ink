#include "ink/ir/context.h"
#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  static_assert(!std::is_default_constructible_v<IRContext>);
  static_assert(!std::is_copy_constructible_v<IRContext>);
  static_assert(!std::is_move_constructible_v<IRContext>);
  static_assert(!std::is_copy_assignable_v<IRContext>);
  static_assert(!std::is_move_assignable_v<IRContext>);
  static_assert(!std::is_constructible_v<Module, target::TargetContext>);
  static_assert(!std::is_constructible_v<Module, IRContext &&>);
  static_assert(!std::is_constructible_v<IRBuilder, IRContext &&>);

  // Modules sharing a context reuse target-specific types, constants and null compatibility without sharing declarations.
  TEST(IRContextTest, SharesPoolsAcrossCallerOwnedModules)
  {
    IRContext Context(Target);
    Module First(Context);
    Module Second(Context);
    const Module &ReadOnly = Second;
    EXPECT_EQ(&First.irContext(), &Context);
    EXPECT_EQ(&ReadOnly.irContext(), &Context);
    EXPECT_EQ(&First.targetContext(), &Context.targetContext());
    EXPECT_EQ(&First.typePool(), &Second.typePool());
    EXPECT_EQ(&First.constantPool(), &Second.constantPool());
    auto *Integer = First.typePool().getInt32Type();
    auto *Signature = First.typePool().getFunctionType(Integer, {Integer});
    ASSERT_NE(Signature, nullptr);
    EXPECT_EQ(Signature, Second.typePool().getFunctionType(Integer, {Integer}));
    auto *Constant = First.constantPool().getSignedIntegerConstant(Integer, 42);
    EXPECT_EQ(Constant, Second.constantPool().getSignedIntegerConstant(Integer, 42));
    auto *FirstFunction = First.createGlobalFunction(*Signature);
    auto *SecondFunction = Second.createGlobalFunction(*Signature);
    ASSERT_NE(FirstFunction, nullptr);
    ASSERT_NE(SecondFunction, nullptr);
    EXPECT_NE(FirstFunction, SecondFunction);
    EXPECT_FALSE(First.owns(SecondFunction));
    EXPECT_FALSE(Second.owns(FirstFunction));
    auto *Pointer = Second.typePool().getPointerType(Integer);
    ASSERT_NE(Pointer, nullptr);
    auto *Null = First.constantPool().getNullPointerConstant();
    EXPECT_EQ(Null, Second.constantPool().getNullPointerConstant());
    EXPECT_TRUE(Null->canInitialize(Pointer));
    auto *Variable = Second.createGlobalVariable(*Pointer);
    ASSERT_NE(Variable, nullptr);
    EXPECT_TRUE(Variable->setInitializer(Null));
  }

  // Equal nominal names and layouts remain distinct module-owned identities inside one shared type pool.
  TEST(IRContextTest, PreservesModuleOwnedNominalIdentities)
  {
    IRContext Context(Target);
    Module First(Context);
    Module Second(Context);
    const auto Name = core::NamePool::instance().intern("SharedName");
    auto *FirstClass = First.createClassType(8, 8, Name);
    auto *SecondClass = Second.createClassType(8, 8, Name);
    auto *Interface = First.createInterfaceType(8, 8, Name);
    auto *Struct = Second.createStructType(8, 8, Name);
    ASSERT_NE(FirstClass, nullptr);
    ASSERT_NE(SecondClass, nullptr);
    ASSERT_NE(Interface, nullptr);
    ASSERT_NE(Struct, nullptr);
    EXPECT_NE(FirstClass, SecondClass);
    EXPECT_EQ(First.classes().front().get(), FirstClass);
    EXPECT_EQ(Second.classes().front().get(), SecondClass);
    EXPECT_TRUE(Context.containsType(FirstClass));
    EXPECT_TRUE(Context.containsType(SecondClass));
    EXPECT_TRUE(Context.containsType(Interface));
    EXPECT_TRUE(Context.containsType(Struct));
    EXPECT_FALSE(Context.typePool().owns(FirstClass));
    EXPECT_FALSE(First.ownsType(SecondClass));
    EXPECT_FALSE(Second.ownsType(FirstClass));
    auto &Types = Context.typePool();
    auto *FirstPointer = Types.getPointerType(FirstClass);
    auto *SecondPointer = Types.getPointerType(SecondClass);
    ASSERT_NE(FirstPointer, nullptr);
    ASSERT_NE(SecondPointer, nullptr);
    EXPECT_NE(FirstPointer, SecondPointer);
    EXPECT_EQ(FirstPointer, Types.getPointerType(FirstClass));
    EXPECT_NE(Types.getFunctionType(FirstClass), Types.getFunctionType(SecondClass));
    EXPECT_NE(Types.getReferenceType(Interface), nullptr);
    EXPECT_NE(Types.getArrayType(Struct, 2), nullptr);
    IRContext Foreign(Target);
    EXPECT_FALSE(Foreign.containsType(FirstClass));
    EXPECT_EQ(Foreign.typePool().getPointerType(FirstClass), nullptr);
    EXPECT_FALSE(Context.containsType(nullptr));
  }

  // Builtins, structural types without nominal dependencies and constants survive the modules that use them.
  TEST(IRContextTest, RetainsPooledValuesAfterModuleDestruction)
  {
    IRContext Context(Target);
    const PointerType *Pointer = nullptr;
    const StringConstant *Text = nullptr;
    const NullPointerConstant *Null = nullptr;
    {
      Module Owner(Context);
      Pointer = Owner.typePool().getPointerType(Owner.typePool().getInt32Type());
      Text = Owner.constantPool().getStringConstant("persistent");
      Null = Owner.constantPool().getNullPointerConstant();
      ASSERT_NE(Pointer, nullptr);
      ASSERT_NE(Text, nullptr);
      auto *Variable = Owner.createGlobalVariable(*Pointer);
      ASSERT_NE(Variable, nullptr);
      ASSERT_TRUE(Variable->setInitializer(Null));
    }
    EXPECT_TRUE(Context.containsType(Pointer));
    EXPECT_EQ(Pointer->size(), Target.pointerByteWidth());
    EXPECT_EQ(Pointer, Context.typePool().getPointerType(Context.typePool().getInt32Type()));
    EXPECT_TRUE(Context.constantPool().owns(Text));
    EXPECT_EQ(Text, Context.constantPool().getStringConstant("persistent"));
    EXPECT_EQ(Null, Context.constantPool().getNullPointerConstant());
    EXPECT_TRUE(Null->canInitialize(Pointer));
    Module Next(Context);
    EXPECT_NE(Next.createGlobalVariable(*Text->type()), nullptr);
  }

  // Independent builders preserve insertion points and reject foreign operands across modules sharing a context.
  TEST(IRContextTest, BuildersKeepIndependentStateAcrossModules)
  {
    IRContext Context(Target);
    Module First(Context);
    Module Second(Context);
    auto *Integer = Context.typePool().getInt32Type();
    auto *Signature = Context.typePool().getFunctionType(Integer, {Integer});
    ASSERT_NE(Signature, nullptr);
    auto *FirstFunction = First.createGlobalFunction(*Signature);
    auto *SecondFunction = Second.createGlobalFunction(*Signature);
    ASSERT_NE(FirstFunction, nullptr);
    ASSERT_NE(SecondFunction, nullptr);
    auto *FirstBlock = FirstFunction->createBlock();
    auto *SecondBlock = SecondFunction->createBlock();
    IRBuilder FirstBuilder(Context);
    IRBuilder SecondBuilder(Context);
    ASSERT_TRUE(FirstBuilder.setInsertPoint(*FirstBlock));
    ASSERT_TRUE(SecondBuilder.setInsertPoint(*SecondBlock));
    EXPECT_EQ(FirstBuilder.createReturn(*SecondFunction->parameter(0)), nullptr);
    EXPECT_EQ(FirstBuilder.insertBlock(), FirstBlock);
    EXPECT_EQ(FirstBlock->instructionCount(), 0U);
    IRContext Foreign(Target);
    Module ForeignModule(Foreign);
    auto *ForeignFunction = ForeignModule.createGlobalFunction(*Foreign.typePool().getFunctionType(Foreign.typePool().getVoidType()));
    ASSERT_NE(ForeignFunction, nullptr);
    EXPECT_FALSE(FirstBuilder.setInsertPoint(*ForeignFunction->createBlock()));
    EXPECT_EQ(FirstBuilder.insertBlock(), FirstBlock);
    ASSERT_NE(FirstBuilder.createReturn(*FirstFunction->parameter(0)), nullptr);
    FirstBuilder.clearInsertPoint();
    EXPECT_EQ(SecondBuilder.insertBlock(), SecondBlock);
    ASSERT_TRUE(FirstBuilder.setInsertPoint(*SecondBlock));
    ASSERT_NE(FirstBuilder.createReturn(*SecondFunction->parameter(0)), nullptr);
    EXPECT_EQ(FirstBlock->instructionCount(), 1U);
    EXPECT_EQ(SecondBlock->instructionCount(), 1U);
  }
} // namespace ink::ir::test
