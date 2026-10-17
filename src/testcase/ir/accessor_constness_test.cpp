#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/nominal_type/method.h"

#include <gtest/gtest.h>

#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  // Explicit object accessors retain const propagation and borrowed lvalue/pointer results on every object category.
  static_assert(std::same_as<decltype(std::declval<IRBuilder &>().irContext()), IRContext &>);
  static_assert(std::same_as<decltype(std::declval<const IRBuilder &>().irContext()), const IRContext &>);
  static_assert(std::same_as<decltype(std::declval<IRBuilder &&>().irContext()), IRContext &>);
  static_assert(std::same_as<decltype(std::declval<const IRBuilder &&>().irContext()), const IRContext &>);
  static_assert(noexcept(std::declval<IRBuilder &>().irContext()) && noexcept(std::declval<const IRBuilder &>().irContext()));

  static_assert(std::same_as<decltype(std::declval<IRBuilder &>().typePool()), TypePool &>);
  static_assert(std::same_as<decltype(std::declval<const IRBuilder &>().typePool()), const TypePool &>);
  static_assert(std::same_as<decltype(std::declval<IRBuilder &&>().typePool()), TypePool &>);
  static_assert(std::same_as<decltype(std::declval<const IRBuilder &&>().typePool()), const TypePool &>);
  static_assert(noexcept(std::declval<IRBuilder &>().typePool()) && noexcept(std::declval<const IRBuilder &>().typePool()));

  static_assert(std::same_as<decltype(std::declval<IRBuilder &>().constantPool()), ConstantPool &>);
  static_assert(std::same_as<decltype(std::declval<const IRBuilder &>().constantPool()), const ConstantPool &>);
  static_assert(std::same_as<decltype(std::declval<IRBuilder &&>().constantPool()), ConstantPool &>);
  static_assert(std::same_as<decltype(std::declval<const IRBuilder &&>().constantPool()), const ConstantPool &>);
  static_assert(noexcept(std::declval<IRBuilder &>().constantPool()) && noexcept(std::declval<const IRBuilder &>().constantPool()));

  static_assert(std::same_as<decltype(std::declval<Module &>().constantPool()), ConstantPool &>);
  static_assert(std::same_as<decltype(std::declval<const Module &>().constantPool()), const ConstantPool &>);
  static_assert(std::same_as<decltype(std::declval<Module &&>().constantPool()), ConstantPool &>);
  static_assert(std::same_as<decltype(std::declval<const Module &&>().constantPool()), const ConstantPool &>);
  static_assert(noexcept(std::declval<Module &>().constantPool()) && noexcept(std::declval<const Module &>().constantPool()));

  static_assert(std::same_as<decltype(std::declval<Module &>().typePool()), TypePool &>);
  static_assert(std::same_as<decltype(std::declval<const Module &>().typePool()), const TypePool &>);
  static_assert(std::same_as<decltype(std::declval<Module &&>().typePool()), TypePool &>);
  static_assert(std::same_as<decltype(std::declval<const Module &&>().typePool()), const TypePool &>);
  static_assert(noexcept(std::declval<Module &>().typePool()) && noexcept(std::declval<const Module &>().typePool()));

  static_assert(std::same_as<decltype(std::declval<Parameter &>().function()), Function *>);
  static_assert(std::same_as<decltype(std::declval<const Parameter &>().function()), const Function *>);
  static_assert(std::same_as<decltype(std::declval<Parameter &&>().function()), Function *>);
  static_assert(std::same_as<decltype(std::declval<const Parameter &&>().function()), const Function *>);
  static_assert(noexcept(std::declval<Parameter &>().function()) && noexcept(std::declval<const Parameter &>().function()));

  static_assert(std::same_as<decltype(std::declval<Instruction &>().parentBlock()), BasicBlock *>);
  static_assert(std::same_as<decltype(std::declval<const Instruction &>().parentBlock()), const BasicBlock *>);
  static_assert(std::same_as<decltype(std::declval<Instruction &&>().parentBlock()), BasicBlock *>);
  static_assert(std::same_as<decltype(std::declval<const Instruction &&>().parentBlock()), const BasicBlock *>);
  static_assert(noexcept(std::declval<Instruction &>().parentBlock()) && noexcept(std::declval<const Instruction &>().parentBlock()));

  static_assert(std::same_as<decltype(std::declval<BasicBlock &>().function()), Function *>);
  static_assert(std::same_as<decltype(std::declval<const BasicBlock &>().function()), const Function *>);
  static_assert(std::same_as<decltype(std::declval<BasicBlock &&>().function()), Function *>);
  static_assert(std::same_as<decltype(std::declval<const BasicBlock &&>().function()), const Function *>);
  static_assert(noexcept(std::declval<BasicBlock &>().function()) && noexcept(std::declval<const BasicBlock &>().function()));

  static_assert(std::same_as<decltype(std::declval<BasicBlock &>().instruction(0)), Instruction *>);
  static_assert(std::same_as<decltype(std::declval<const BasicBlock &>().instruction(0)), const Instruction *>);
  static_assert(std::same_as<decltype(std::declval<BasicBlock &&>().instruction(0)), Instruction *>);
  static_assert(std::same_as<decltype(std::declval<const BasicBlock &&>().instruction(0)), const Instruction *>);
  static_assert(noexcept(std::declval<BasicBlock &>().instruction(0)) && noexcept(std::declval<const BasicBlock &>().instruction(0)));

  static_assert(std::same_as<decltype(std::declval<Function &>().parameter(0)), Parameter *>);
  static_assert(std::same_as<decltype(std::declval<const Function &>().parameter(0)), const Parameter *>);
  static_assert(std::same_as<decltype(std::declval<Function &&>().parameter(0)), Parameter *>);
  static_assert(std::same_as<decltype(std::declval<const Function &&>().parameter(0)), const Parameter *>);
  static_assert(noexcept(std::declval<Function &>().parameter(0)) && noexcept(std::declval<const Function &>().parameter(0)));

  static_assert(std::same_as<decltype(std::declval<Function &>().block(0)), BasicBlock *>);
  static_assert(std::same_as<decltype(std::declval<const Function &>().block(0)), const BasicBlock *>);
  static_assert(std::same_as<decltype(std::declval<Function &&>().block(0)), BasicBlock *>);
  static_assert(std::same_as<decltype(std::declval<const Function &&>().block(0)), const BasicBlock *>);
  static_assert(noexcept(std::declval<Function &>().block(0)) && noexcept(std::declval<const Function &>().block(0)));

  static_assert(std::same_as<decltype(std::declval<Function &>().entryBlock()), BasicBlock *>);
  static_assert(std::same_as<decltype(std::declval<const Function &>().entryBlock()), const BasicBlock *>);
  static_assert(std::same_as<decltype(std::declval<Function &&>().entryBlock()), BasicBlock *>);
  static_assert(std::same_as<decltype(std::declval<const Function &&>().entryBlock()), const BasicBlock *>);
  static_assert(noexcept(std::declval<Function &>().entryBlock()) && noexcept(std::declval<const Function &>().entryBlock()));

  static_assert(std::same_as<decltype(std::declval<Module &>().globalVariable(0)), GlobalVariable *>);
  static_assert(std::same_as<decltype(std::declval<const Module &>().globalVariable(0)), const GlobalVariable *>);
  static_assert(std::same_as<decltype(std::declval<Module &&>().globalVariable(0)), GlobalVariable *>);
  static_assert(std::same_as<decltype(std::declval<const Module &&>().globalVariable(0)), const GlobalVariable *>);
  static_assert(noexcept(std::declval<Module &>().globalVariable(0)) && noexcept(std::declval<const Module &>().globalVariable(0)));

  static_assert(std::same_as<decltype(std::declval<Module &>().globalFunction(0)), Function *>);
  static_assert(std::same_as<decltype(std::declval<const Module &>().globalFunction(0)), const Function *>);
  static_assert(std::same_as<decltype(std::declval<Module &&>().globalFunction(0)), Function *>);
  static_assert(std::same_as<decltype(std::declval<const Module &&>().globalFunction(0)), const Function *>);
  static_assert(noexcept(std::declval<Module &>().globalFunction(0)) && noexcept(std::declval<const Module &>().globalFunction(0)));

  static_assert(std::same_as<decltype(std::declval<Module &>().classType(0)), ClassType *>);
  static_assert(std::same_as<decltype(std::declval<const Module &>().classType(0)), const ClassType *>);
  static_assert(std::same_as<decltype(std::declval<Module &&>().classType(0)), ClassType *>);
  static_assert(std::same_as<decltype(std::declval<const Module &&>().classType(0)), const ClassType *>);
  static_assert(noexcept(std::declval<Module &>().classType(0)) && noexcept(std::declval<const Module &>().classType(0)));

  static_assert(std::same_as<decltype(std::declval<Module &>().interfaceType(0)), InterfaceType *>);
  static_assert(std::same_as<decltype(std::declval<const Module &>().interfaceType(0)), const InterfaceType *>);
  static_assert(std::same_as<decltype(std::declval<Module &&>().interfaceType(0)), InterfaceType *>);
  static_assert(std::same_as<decltype(std::declval<const Module &&>().interfaceType(0)), const InterfaceType *>);
  static_assert(noexcept(std::declval<Module &>().interfaceType(0)) && noexcept(std::declval<const Module &>().interfaceType(0)));

  static_assert(std::same_as<decltype(std::declval<Module &>().structType(0)), StructType *>);
  static_assert(std::same_as<decltype(std::declval<const Module &>().structType(0)), const StructType *>);
  static_assert(std::same_as<decltype(std::declval<Module &&>().structType(0)), StructType *>);
  static_assert(std::same_as<decltype(std::declval<const Module &&>().structType(0)), const StructType *>);
  static_assert(noexcept(std::declval<Module &>().structType(0)) && noexcept(std::declval<const Module &>().structType(0)));

  static_assert(std::same_as<decltype(std::declval<Module &>().initializationFunction()), Function *>);
  static_assert(std::same_as<decltype(std::declval<const Module &>().initializationFunction()), const Function *>);
  static_assert(std::same_as<decltype(std::declval<Module &&>().initializationFunction()), Function *>);
  static_assert(std::same_as<decltype(std::declval<const Module &&>().initializationFunction()), const Function *>);
  static_assert(noexcept(std::declval<Module &>().initializationFunction()) && noexcept(std::declval<const Module &>().initializationFunction()));

  static_assert(std::same_as<decltype(std::declval<Module &>().deinitializationFunction()), Function *>);
  static_assert(std::same_as<decltype(std::declval<const Module &>().deinitializationFunction()), const Function *>);
  static_assert(std::same_as<decltype(std::declval<Module &&>().deinitializationFunction()), Function *>);
  static_assert(std::same_as<decltype(std::declval<const Module &&>().deinitializationFunction()), const Function *>);
  static_assert(noexcept(std::declval<Module &>().deinitializationFunction()) && noexcept(std::declval<const Module &>().deinitializationFunction()));

  static_assert(std::same_as<decltype(std::declval<Method &>().function()), Function *>);
  static_assert(std::same_as<decltype(std::declval<const Method &>().function()), const Function *>);
  static_assert(std::same_as<decltype(std::declval<Method &&>().function()), Function *>);
  static_assert(std::same_as<decltype(std::declval<const Method &&>().function()), const Function *>);
  static_assert(noexcept(std::declval<Method &>().function()) && noexcept(std::declval<const Method &>().function()));

  // Accessing an rvalue owner borrows the same IR objects without moving its owned storage or pools.
  TEST(IRAccessorConstnessTest, RvalueAccessPreservesOwnershipAndIdentity)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    Module &Types = Owner;
    IRBuilder Builder(Owner.irContext());
    auto *Callable = Owner.createGlobalFunction(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Types.typePool().getInt32Type()}));
    auto *Entry = Callable->createBlock();
    auto *ParameterValue = Callable->parameter(0);
    auto *Variable = Owner.createGlobalVariable(*Types.typePool().getInt32Type());
    auto *Class = Owner.createClassType(8, 8);
    auto *Interface = Owner.createInterfaceType(8, 8);
    auto *Struct = Owner.createStructType(8, 8);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Return = Builder.createReturn();
    ASSERT_NE(Return, nullptr);

    EXPECT_EQ(&std::move(Builder).irContext(), &Types.irContext());
    EXPECT_EQ(&std::move(std::as_const(Builder)).irContext(), &Types.irContext());
    EXPECT_EQ(&std::move(Builder).typePool(), &Types.typePool());
    EXPECT_EQ(&std::move(std::as_const(Builder)).typePool(), &Types.typePool());
    EXPECT_EQ(&std::move(Builder).constantPool(), &Types.constantPool());
    EXPECT_EQ(&std::move(std::as_const(Types)).constantPool(), &Types.constantPool());
    EXPECT_EQ(&std::move(Owner).typePool(), &Owner.typePool());
    EXPECT_EQ(&std::move(std::as_const(Owner)).typePool(), &Owner.typePool());
    EXPECT_EQ(std::move(Owner).classType(0), Class);
    EXPECT_EQ(std::move(std::as_const(Owner)).classType(0), Class);
    EXPECT_EQ(std::move(Owner).interfaceType(0), Interface);
    EXPECT_EQ(std::move(std::as_const(Owner)).interfaceType(0), Interface);
    EXPECT_EQ(std::move(Owner).structType(0), Struct);
    EXPECT_EQ(std::move(std::as_const(Owner)).structType(0), Struct);
    EXPECT_EQ(std::move(Owner).globalFunction(0), Callable);
    EXPECT_EQ(std::move(std::as_const(Owner)).globalVariable(0), Variable);
    EXPECT_EQ(std::move(*Callable).entryBlock(), Entry);
    EXPECT_EQ(std::move(std::as_const(*Callable)).block(0), Entry);
    EXPECT_EQ(std::move(std::as_const(*Callable)).parameter(0), ParameterValue);
    EXPECT_EQ(std::move(*ParameterValue).function(), Callable);
    EXPECT_EQ(std::move(std::as_const(*ParameterValue)).function(), Callable);
    EXPECT_EQ(std::move(std::as_const(*Entry)).function(), Callable);
    EXPECT_EQ(std::move(std::as_const(*Entry)).instruction(0), Return);
    EXPECT_EQ(std::move(*Return).parentBlock(), Entry);
    EXPECT_EQ(std::move(std::as_const(*Return)).parentBlock(), Entry);
    EXPECT_EQ(Owner.classCount(), 1U);
    EXPECT_EQ(Owner.interfaceCount(), 1U);
    EXPECT_EQ(Owner.structCount(), 1U);
    EXPECT_EQ(Owner.globalFunctionCount(), 1U);
    EXPECT_EQ(Callable->blockCount(), 1U);
    EXPECT_EQ(Entry->instructionCount(), 1U);

    auto *Lifecycle = Owner.createGlobalFunction(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    ASSERT_TRUE(Owner.setInitializationFunction(Lifecycle));
    ASSERT_TRUE(Owner.setDeinitializationFunction(Lifecycle));
    EXPECT_EQ(std::move(Owner).initializationFunction(), Lifecycle);
    EXPECT_EQ(std::move(std::as_const(Owner)).deinitializationFunction(), Lifecycle);

    Method Member(std::make_unique<Function>(*Lifecycle->functionType()));
    auto *MemberFunction = Member.function();
    EXPECT_EQ(std::move(Member).function(), MemberFunction);
    EXPECT_EQ(std::move(std::as_const(Member)).function(), MemberFunction);
    EXPECT_EQ(Member.function(), MemberFunction);
  }

  // Const-propagating pointer access never dereferences absent parents, moved-from methods or out-of-range entries.
  TEST(IRAccessorConstnessTest, NullableAndIndexedAccessPreservesNull)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    Module &Types = Owner;
    Function Callable(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    BasicBlock Detached;
    Method Empty(nullptr);
    Method Member(std::make_unique<Function>(*Callable.functionType()));
    Method Moved(std::move(Member));
    constexpr std::size_t OutOfRange = std::numeric_limits<std::size_t>::max();

    EXPECT_EQ(std::move(Owner).classType(OutOfRange), nullptr);
    EXPECT_EQ(std::move(std::as_const(Owner)).classType(OutOfRange), nullptr);
    EXPECT_EQ(std::move(Owner).interfaceType(OutOfRange), nullptr);
    EXPECT_EQ(std::move(std::as_const(Owner)).interfaceType(OutOfRange), nullptr);
    EXPECT_EQ(std::move(Owner).structType(OutOfRange), nullptr);
    EXPECT_EQ(std::move(std::as_const(Owner)).structType(OutOfRange), nullptr);
    EXPECT_EQ(std::move(std::as_const(Owner)).globalVariable(OutOfRange), nullptr);
    EXPECT_EQ(std::move(std::as_const(Owner)).globalFunction(OutOfRange), nullptr);
    EXPECT_EQ(std::move(std::as_const(Owner)).initializationFunction(), nullptr);
    EXPECT_EQ(std::move(std::as_const(Owner)).deinitializationFunction(), nullptr);
    EXPECT_EQ(std::move(std::as_const(Callable)).parameter(OutOfRange), nullptr);
    EXPECT_EQ(std::move(std::as_const(Callable)).block(OutOfRange), nullptr);
    EXPECT_EQ(std::move(std::as_const(Callable)).entryBlock(), nullptr);
    EXPECT_EQ(std::move(std::as_const(Detached)).function(), nullptr);
    EXPECT_EQ(std::move(std::as_const(Detached)).instruction(OutOfRange), nullptr);
    EXPECT_EQ(std::move(std::as_const(Empty)).function(), nullptr);
    EXPECT_EQ(std::move(std::as_const(Member)).function(), nullptr);
    EXPECT_NE(Moved.function(), nullptr);
  }
} // namespace ink::ir::test
