#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"

#include <gtest/gtest.h>

#include <functional>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir::test
{
  static_assert(!std::is_constructible_v<InstructionBuilder, TypePool &&>);
  static_assert(std::is_same_v<decltype(std::declval<Instruction &>().parent()), BasicBlock *>);
  static_assert(std::is_same_v<decltype(std::declval<const Instruction &>().parent()), const BasicBlock *>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<BasicBlock &>().instructions())>, Instruction &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const BasicBlock &>().instructions())>, const Instruction &>);
  static_assert(std::bidirectional_iterator<BasicBlock::Iterator> && std::bidirectional_iterator<BasicBlock::ConstIterator>);
  static_assert(std::is_convertible_v<BasicBlock::Iterator, BasicBlock::ConstIterator> && !std::is_convertible_v<BasicBlock::ConstIterator, BasicBlock::Iterator>);

  template <typename OperandType>
  constexpr bool AcceptsBuilderOperands = requires(InstructionBuilder &Builder, OperandType &Operand, ValueRef Other)
  {
    Builder.createAdd(Operand, Other);
    Builder.createAdd(Other, Operand);
    Builder.createReturn(Operand);
  };

  static_assert(AcceptsBuilderOperands<IntegerConstant> && AcceptsBuilderOperands<const IntegerConstant>);
  static_assert(AcceptsBuilderOperands<AddInstruction> && AcceptsBuilderOperands<const AddInstruction> && AcceptsBuilderOperands<ValueRef>);

  template <typename OperandType>
  constexpr bool RejectsTemporaryBuilderOperands = !requires(InstructionBuilder &Builder, OperandType &&Operand, ValueRef Other)
  {
    Builder.createAdd(std::forward<OperandType>(Operand), Other);
  } && !requires(InstructionBuilder &Builder, OperandType &&Operand, ValueRef Other)
  {
    Builder.createAdd(Other, std::forward<OperandType>(Operand));
  } && !requires(InstructionBuilder &Builder, OperandType &&Operand)
  {
    Builder.createAdd(std::forward<OperandType>(Operand), std::forward<OperandType>(Operand));
  } && !requires(InstructionBuilder &Builder, OperandType &&Operand)
  {
    Builder.createReturn(std::forward<OperandType>(Operand));
  };

  static_assert(RejectsTemporaryBuilderOperands<IntegerConstant> && RejectsTemporaryBuilderOperands<const IntegerConstant>);
  static_assert(RejectsTemporaryBuilderOperands<AddInstruction> && RejectsTemporaryBuilderOperands<const AddInstruction>);

  // An unset or cleared insertion point rejects every creation overload without changing the selected block.
  TEST(IRInstructionBuilderTest, RejectsCreationWithoutAnInsertionPoint)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const FloatConstant &LHS = Constants.getFloat(1.0f);
    const FloatConstant &RHS = Constants.getFloat(2.0f);
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    EXPECT_EQ(Builder.getInsertBlock(), nullptr);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
    EXPECT_EQ(Builder.createAlloca(Types.getFloatType(FloatType::Width::Bits32)), nullptr);
    EXPECT_EQ(Builder.createAdd(LHS, RHS), nullptr);
    EXPECT_EQ(Builder.createReturn(), nullptr);
    EXPECT_EQ(Builder.createReturn(LHS), nullptr);
    EXPECT_EQ(Block.instructionCount(), 0u);

    Builder.setInsertPoint(Block);
    ASSERT_NE(Builder.createAdd(LHS, RHS), nullptr);
    Builder.clearInsertPoint();
    EXPECT_EQ(Builder.getInsertBlock(), nullptr);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
    EXPECT_EQ(Builder.createAlloca(Types.getFloatType(FloatType::Width::Bits32)), nullptr);
    EXPECT_EQ(Builder.createAdd(LHS, RHS), nullptr);
    EXPECT_EQ(Builder.createReturn(), nullptr);
    EXPECT_EQ(Builder.createReturn(LHS), nullptr);
    EXPECT_EQ(Block.instructionCount(), 1u);
  }

  // Function blocks own ordered Add and Return nodes after their builder is destroyed, preserving borrowed operand identities.
  TEST(IRInstructionBuilderTest, KeepsCreatedInstructionsInTheirOwningBlockAfterBuilderDestruction)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant *Twenty = Constants.getSignedInteger(Int32, 20);
    const IntegerConstant *TwentyTwo = Constants.getSignedInteger(Int32, 22);
    ASSERT_NE(Twenty, nullptr);
    ASSERT_NE(TwentyTwo, nullptr);
    Function FunctionValue(Types.getFunctionType(Int32));
    BasicBlock &Block = FunctionValue.createBasicBlock();
    AddInstruction *Sum = nullptr;
    ReturnInstruction *Return = nullptr;
    {
      InstructionBuilder Builder(Types);
      Builder.setInsertPoint(Block);
      Sum = Builder.createAdd(ValueRef(*Twenty), std::cref(*TwentyTwo));
      ASSERT_NE(Sum, nullptr);
      Return = Builder.createReturn(std::cref(*Sum));
      ASSERT_NE(Return, nullptr);
    }

    ASSERT_EQ(Block.instructionCount(), 2u);
    auto Iterator = Block.instructions().begin();
    EXPECT_EQ(&*Iterator, Sum);
    ++Iterator;
    EXPECT_EQ(&*Iterator, Return);
    ++Iterator;
    EXPECT_EQ(Iterator, Block.instructions().end());
    EXPECT_EQ(Sum->parent(), &Block);
    EXPECT_EQ(Return->parent(), &Block);
    EXPECT_EQ(&Sum->getLHS(), Twenty);
    EXPECT_EQ(&Sum->getRHS(), TwentyTwo);
    EXPECT_EQ(&Sum->getType(), &Int32);
    EXPECT_EQ(Return->getReturnValue(), Sum);
    EXPECT_EQ(&Return->getType(), &Types.getVoidType());
    const BasicBlock &ConstBlock = Block;
    const Instruction &ConstSum = *ConstBlock.instructions().begin();
    EXPECT_EQ(&ConstSum, Sum);
    EXPECT_EQ(ConstSum.parent(), &ConstBlock);
  }

  // Reusing a stable anchor preserves creation order across block switches, and a detached instruction cannot replace that anchor.
  TEST(IRInstructionBuilderTest, PreservesAnchoredInsertionOrderAndExistingIteratorsAcrossBlockSwitches)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const FloatConstant &LHS = Constants.getFloat(1.0f);
    const FloatConstant &RHS = Constants.getFloat(2.0f);
    BasicBlock FirstBlock;
    BasicBlock SecondBlock;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(FirstBlock);
    ReturnInstruction *Anchor = Builder.createReturn();
    ASSERT_NE(Anchor, nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Anchor));
    AddInstruction *First = Builder.createAdd(LHS, RHS);
    ASSERT_NE(First, nullptr);
    auto SavedIterator = FirstBlock.instructions().begin();
    EXPECT_EQ(&*SavedIterator, First);
    std::vector<const Instruction *> Expected = {First};

    Builder.setInsertPoint(SecondBlock);
    ReturnInstruction *SecondReturn = Builder.createReturn();
    ASSERT_NE(SecondReturn, nullptr);
    EXPECT_EQ(SecondReturn->parent(), &SecondBlock);
    EXPECT_EQ(SecondReturn->getReturnValue(), nullptr);
    EXPECT_EQ(SecondBlock.instructionCount(), 1u);
    ASSERT_TRUE(Builder.setInsertPoint(*Anchor));
    AddInstruction DetachedAnchor(LHS, RHS);
    EXPECT_EQ(DetachedAnchor.parent(), nullptr);
    EXPECT_FALSE(Builder.setInsertPoint(DetachedAnchor));
    EXPECT_EQ(Builder.getInsertBlock(), &FirstBlock);
    EXPECT_EQ(Builder.getInsertBefore(), Anchor);

    for (unsigned Index = 0; Index < 32; ++Index)
    {
      AddInstruction *Inserted = Builder.createAdd(*First, RHS);
      ASSERT_NE(Inserted, nullptr);
      EXPECT_EQ(Inserted->parent(), &FirstBlock);
      Expected.push_back(Inserted);
    }
    Expected.push_back(Anchor);
    EXPECT_EQ(FirstBlock.instructionCount(), Expected.size());
    EXPECT_EQ(&*SavedIterator, First);
    EXPECT_EQ(Builder.getInsertBefore(), Anchor);
    auto Iterator = FirstBlock.instructions().begin();
    for (const Instruction *ExpectedInstruction : Expected)
    {
      ASSERT_NE(Iterator, FirstBlock.instructions().end());
      EXPECT_EQ(&*Iterator, ExpectedInstruction);
      ++Iterator;
    }
    EXPECT_EQ(Iterator, FirstBlock.instructions().end());
    auto ReverseIterator = FirstBlock.instructions().end();
    for (auto ExpectedIterator = Expected.rbegin(); ExpectedIterator != Expected.rend(); ++ExpectedIterator)
    {
      --ReverseIterator;
      EXPECT_EQ(&*ReverseIterator, *ExpectedIterator);
    }
    EXPECT_EQ(ReverseIterator, FirstBlock.instructions().begin());
  }
} // namespace ink::ir::test
