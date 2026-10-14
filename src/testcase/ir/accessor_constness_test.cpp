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
  static_assert(std::same_as<decltype(std::declval<IRBuilder &>().typeContext()), TypeContext &>);
  static_assert(std::same_as<decltype(std::declval<const IRBuilder &>().typeContext()), const TypeContext &>);
  static_assert(std::same_as<decltype(std::declval<IRBuilder &&>().typeContext()), TypeContext &>);
  static_assert(std::same_as<decltype(std::declval<const IRBuilder &&>().typeContext()), const TypeContext &>);
  static_assert(noexcept(std::declval<IRBuilder &>().typeContext()) && noexcept(std::declval<const IRBuilder &>().typeContext()));

  static_assert(std::same_as<decltype(std::declval<IRBuilder &>().constantPool()), ConstantPool &>);
  static_assert(std::same_as<decltype(std::declval<const IRBuilder &>().constantPool()), const ConstantPool &>);
  static_assert(std::same_as<decltype(std::declval<IRBuilder &&>().constantPool()), ConstantPool &>);
  static_assert(std::same_as<decltype(std::declval<const IRBuilder &&>().constantPool()), const ConstantPool &>);
  static_assert(noexcept(std::declval<IRBuilder &>().constantPool()) && noexcept(std::declval<const IRBuilder &>().constantPool()));

  static_assert(std::same_as<decltype(std::declval<TypeContext &>().constantPool()), ConstantPool &>);
  static_assert(std::same_as<decltype(std::declval<const TypeContext &>().constantPool()), const ConstantPool &>);
  static_assert(std::same_as<decltype(std::declval<TypeContext &&>().constantPool()), ConstantPool &>);
  static_assert(std::same_as<decltype(std::declval<const TypeContext &&>().constantPool()), const ConstantPool &>);
  static_assert(noexcept(std::declval<TypeContext &>().constantPool()) && noexcept(std::declval<const TypeContext &>().constantPool()));

  static_assert(std::same_as<decltype(std::declval<Parameter &>().function()), Function &>);
  static_assert(std::same_as<decltype(std::declval<const Parameter &>().function()), const Function &>);
  static_assert(std::same_as<decltype(std::declval<Parameter &&>().function()), Function &>);
  static_assert(std::same_as<decltype(std::declval<const Parameter &&>().function()), const Function &>);
  static_assert(noexcept(std::declval<Parameter &>().function()) && noexcept(std::declval<const Parameter &>().function()));

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

  // Accessing an rvalue owner borrows the same IR objects without moving its owned storage or contexts.
  TEST(IRAccessorConstnessTest, RvalueAccessPreservesOwnershipAndIdentity)
  {
    TypeContext Types(Target);
    IRBuilder Builder(Types);
    Module Owner;
    auto *Callable = Owner.createGlobalFunction(*Types.getFunctionType(Types.getVoidType(), {Types.getInt32Type()}));
    auto *Entry = Callable->createBlock();
    auto *ParameterValue = Callable->parameter(0);
    auto *Variable = Owner.createGlobalVariable(*Types.getInt32Type());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Return = Builder.createReturn();
    ASSERT_NE(Return, nullptr);

    EXPECT_EQ(&std::move(Builder).typeContext(), &Types);
    EXPECT_EQ(&std::move(std::as_const(Builder)).typeContext(), &Types);
    EXPECT_EQ(&std::move(Builder).constantPool(), &Types.constantPool());
    EXPECT_EQ(&std::move(std::as_const(Types)).constantPool(), &Types.constantPool());
    EXPECT_EQ(std::move(Owner).globalFunction(0), Callable);
    EXPECT_EQ(std::move(std::as_const(Owner)).globalVariable(0), Variable);
    EXPECT_EQ(std::move(*Callable).entryBlock(), Entry);
    EXPECT_EQ(std::move(std::as_const(*Callable)).block(0), Entry);
    EXPECT_EQ(std::move(std::as_const(*Callable)).parameter(0), ParameterValue);
    EXPECT_EQ(&std::move(*ParameterValue).function(), Callable);
    EXPECT_EQ(&std::move(std::as_const(*ParameterValue)).function(), Callable);
    EXPECT_EQ(std::move(std::as_const(*Entry)).function(), Callable);
    EXPECT_EQ(std::move(std::as_const(*Entry)).instruction(0), Return);
    EXPECT_EQ(Owner.globalFunctionCount(), 1U);
    EXPECT_EQ(Callable->blockCount(), 1U);
    EXPECT_EQ(Entry->instructionCount(), 1U);

    auto *Lifecycle = Owner.createGlobalFunction(*Types.getFunctionType(Types.getVoidType()));
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
    TypeContext Types(Target);
    Module Owner;
    Function Callable(*Types.getFunctionType(Types.getVoidType()));
    BasicBlock Detached;
    Method Empty(nullptr);
    Method Member(std::make_unique<Function>(*Callable.functionType()));
    Method Moved(std::move(Member));
    constexpr std::size_t OutOfRange = std::numeric_limits<std::size_t>::max();

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
