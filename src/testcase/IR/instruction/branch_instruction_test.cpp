#include "ink/core/support/core_define.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/cond_branch_instruction.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_nothrow_constructible_v<BranchInstruction, ValueRef, BasicBlock &>);
  static_assert(std::is_nothrow_constructible_v<CondBranchInstruction, ValueRef, ValueRef, BasicBlock &, BasicBlock &>);
  static_assert(!std::is_constructible_v<BranchInstruction, VoidType &&, BasicBlock &>);
  static_assert(!std::is_constructible_v<CondBranchInstruction, const VoidType &&, ValueRef, BasicBlock &, BasicBlock &>);
  static_assert(!std::is_constructible_v<CondBranchInstruction, ValueRef, IntegerConstant &&, BasicBlock &, BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<BranchInstruction &>().getTarget()), BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<const BranchInstruction &>().getTarget()), const BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<const CondBranchInstruction &>().getCondition()), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<CondBranchInstruction &>().getTrueTarget()), BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<const CondBranchInstruction &>().getTrueTarget()), const BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<CondBranchInstruction &>().getFalseTarget()), BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<const CondBranchInstruction &>().getFalseTarget()), const BasicBlock &>);

  template <typename ConditionType>
  constexpr bool RejectsTemporaryBranchConditions = !requires(InstructionBuilder &Builder, ConditionType &&Condition, BasicBlock &Target)
  {
    Builder.createCondBranch(std::forward<ConditionType>(Condition), Target, Target);
  };

  static_assert(RejectsTemporaryBranchConditions<IntegerConstant> && RejectsTemporaryBranchConditions<const IntegerConstant>);
  static_assert(RejectsTemporaryBranchConditions<Argument> && RejectsTemporaryBranchConditions<const Argument>);

  // Direct branch nodes borrow their condition and targets, preserve const access, and identify themselves as terminators.
  TEST(IRBranchInstructionTest, PreservesBorrowedOperandsAndTargetConstness)
  {
    TypePool Types;
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    const Argument &Condition = FunctionValue.argument(0);
    BasicBlock &TrueTarget = FunctionValue.createBasicBlock();
    BasicBlock &FalseTarget = FunctionValue.createBasicBlock();
    BranchInstruction Branch(Types.getVoidType(), TrueTarget);
    CondBranchInstruction Conditional(Types.getVoidType(), Condition, TrueTarget, FalseTarget);
    const BranchInstruction &ConstBranch = Branch;
    const CondBranchInstruction &ConstConditional = Conditional;
    EXPECT_EQ(&Branch.getType(), &Types.getVoidType());
    EXPECT_EQ(&Conditional.getType(), &Types.getVoidType());
    EXPECT_EQ(&Branch.getTarget(), &TrueTarget);
    EXPECT_EQ(&ConstBranch.getTarget(), &TrueTarget);
    EXPECT_EQ(&Conditional.getCondition(), &Condition);
    EXPECT_EQ(&ConstConditional.getCondition(), &Condition);
    EXPECT_EQ(&Conditional.getTrueTarget(), &TrueTarget);
    EXPECT_EQ(&Conditional.getFalseTarget(), &FalseTarget);
    EXPECT_EQ(&ConstConditional.getTrueTarget(), &TrueTarget);
    EXPECT_EQ(&ConstConditional.getFalseTarget(), &FalseTarget);
    EXPECT_EQ(Branch.getKind(), InstructionKind::Branch);
    EXPECT_EQ(Conditional.getKind(), InstructionKind::CondBranch);
    EXPECT_EQ(Branch.getName(), "branch");
    EXPECT_EQ(Conditional.getName(), "cond_branch");
    EXPECT_TRUE(Branch.isTerminator());
    EXPECT_TRUE(Conditional.isTerminator());
    EXPECT_TRUE(BranchInstruction::classof(&Branch));
    EXPECT_FALSE(BranchInstruction::classof(&Conditional));
    EXPECT_FALSE(BranchInstruction::classof(nullptr));
    EXPECT_TRUE(CondBranchInstruction::classof(&Conditional));
    EXPECT_FALSE(CondBranchInstruction::classof(&Branch));
    EXPECT_FALSE(CondBranchInstruction::classof(nullptr));
    EXPECT_EQ(Branch.parent(), nullptr);
    EXPECT_EQ(Conditional.parent(), nullptr);
  }

#if INK_ENABLE_ASSERTIONS
  // Invalid insertion owners, foreign or detached targets, and non-boolean conditions each violate branch construction contracts.
  TEST(IRBranchInstructionDeathTest, ChecksInvalidOwnersTargetsAndConditions)
  {
    TypePool Types;
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    Function OtherFunction(Types.getFunctionType(Types.getVoidType()));
    BasicBlock &Source = FunctionValue.createBasicBlock();
    BasicBlock &Target = FunctionValue.createBasicBlock();
    BasicBlock &Foreign = OtherFunction.createBasicBlock();
    BasicBlock Detached;
    const Argument &Condition = FunctionValue.argument(0);
    const IntegerConstant Integer(Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed), 1);
    InstructionBuilder Builder(Types);
    EXPECT_DEATH(static_cast<void>(Builder.createBranch(Target)), "insertion point");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Condition, Target, Target)), "insertion point");
    EXPECT_EQ(Builder.getInsertBlock(), nullptr);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
    Builder.setInsertPoint(Source);
    EXPECT_DEATH(static_cast<void>(Builder.createBranch(Foreign)), "Branch target must belong");
    EXPECT_DEATH(static_cast<void>(Builder.createBranch(Detached)), "Branch target must belong");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Integer, Target, Target)), "boolean condition");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Types.getBoolType(), Target, Target)), "boolean condition");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Condition, Foreign, Target)), "Conditional branch targets must belong");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Condition, Target, Foreign)), "Conditional branch targets must belong");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Condition, Detached, Target)), "Conditional branch targets must belong");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Condition, Target, Detached)), "Conditional branch targets must belong");
    EXPECT_EQ(Source.instructionCount(), 0U);
    EXPECT_EQ(Builder.getInsertBlock(), &Source);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
    Builder.setInsertPoint(Detached);
    EXPECT_DEATH(static_cast<void>(Builder.createBranch(Target)), "Branch target must belong");
    EXPECT_DEATH(static_cast<void>(Builder.createBranch(Detached)), "Branch target must belong");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Condition, Target, Target)), "Conditional branch targets must belong");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Condition, Detached, Detached)), "Conditional branch targets must belong");
    EXPECT_EQ(Detached.instructionCount(), 0U);
    EXPECT_EQ(Builder.getInsertBlock(), &Detached);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
    Builder.setInsertPoint(Source);
    BranchInstruction *Branch = &Builder.createBranch(Target);
    EXPECT_EQ(Branch->parent(), &Source);
    EXPECT_EQ(&Branch->getTarget(), &Target);
    EXPECT_EQ(Source.instructionCount(), 1U);
    ASSERT_TRUE(Builder.setInsertPoint(*Branch));
    EXPECT_DEATH(static_cast<void>(Builder.createBranch(Foreign)), "Terminators must be inserted at the end");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Integer, Target, Target)), "Terminators must be inserted at the end");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Condition, Target, Detached)), "Terminators must be inserted at the end");
    EXPECT_EQ(Source.instructionCount(), 1U);
    EXPECT_EQ(Builder.getInsertBlock(), &Source);
    EXPECT_EQ(Builder.getInsertBefore(), Branch);
    static_cast<void>(Builder.createAlloca(Types.getBoolType()));
    EXPECT_EQ(Source.instructionCount(), 2U);
    EXPECT_EQ(Builder.getInsertBefore(), Branch);
  }

  // Terminators inserted before an existing instruction and appends after a branch both violate block ordering contracts.
  TEST(IRBranchInstructionDeathTest, ChecksTerminatorInsertionPositionAndAppendAfterBranch)
  {
    TypePool Types;
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    BasicBlock &Source = FunctionValue.createBasicBlock();
    BasicBlock &Target = FunctionValue.createBasicBlock();
    const Argument &Condition = FunctionValue.argument(0);
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Source);
    AllocaInstruction *Anchor = &Builder.createAlloca(Types.getBoolType());
    ASSERT_TRUE(Builder.setInsertPoint(*Anchor));
    EXPECT_DEATH(static_cast<void>(Builder.createBranch(Target)), "at the end of a block");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Condition, Target, Target)), "at the end of a block");
    EXPECT_DEATH(static_cast<void>(Builder.createReturn()), "at the end of a block");
    EXPECT_EQ(Source.instructionCount(), 1U);
    EXPECT_EQ(Builder.getInsertBlock(), &Source);
    EXPECT_EQ(Builder.getInsertBefore(), Anchor);
    AllocaInstruction *Earlier = &Builder.createAlloca(Types.getBoolType());
    EXPECT_EQ(Builder.getInsertBefore(), Anchor);
    Builder.setInsertPoint(Source);
    BranchInstruction *Branch = &Builder.createBranch(Target);
    EXPECT_EQ(Source.terminator(), Branch);
    EXPECT_DEATH(static_cast<void>(Builder.createAlloca(Types.getBoolType())), "after a terminator");
    EXPECT_DEATH(static_cast<void>(Builder.createBranch(Target)), "after a terminator");
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
    EXPECT_EQ(Source.instructionCount(), 3U);
    ASSERT_TRUE(Builder.setInsertPoint(*Branch));
    EXPECT_DEATH(static_cast<void>(Builder.createBranch(Target)), "at the end of a block");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Condition, Source, Target)), "at the end of a block");
    EXPECT_DEATH(static_cast<void>(Builder.createReturn()), "at the end of a block");
    EXPECT_EQ(Builder.getInsertBefore(), Branch);
    EXPECT_EQ(Source.instructionCount(), 3U);
    auto Iterator = Source.instructions().begin();
    EXPECT_EQ(&*Iterator++, Earlier);
    EXPECT_EQ(&*Iterator++, Anchor);
    EXPECT_EQ(&*Iterator++, Branch);
    EXPECT_EQ(Iterator, Source.instructions().end());
  }

  // Appending any ordinary instruction or a second terminator after return violates the insertion contract.
  TEST(IRBranchInstructionDeathTest, ChecksEveryAppendAfterReturn)
  {
    TypePool Types;
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    BasicBlock &Source = FunctionValue.createBasicBlock();
    BasicBlock &Target = FunctionValue.createBasicBlock();
    const Argument &Condition = FunctionValue.argument(0);
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant Integer(Int32, 1);
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Source);
    AllocaInstruction &Slot = Builder.createAlloca(Int32);
    ReturnInstruction &Return = Builder.createReturn();
    EXPECT_DEATH(static_cast<void>(Builder.createAdd(Integer, Integer)), "after a terminator");
    EXPECT_DEATH(static_cast<void>(Builder.createAlloca(Int32)), "after a terminator");
    EXPECT_DEATH(static_cast<void>(Builder.createLoad(Slot)), "after a terminator");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(Slot, Integer)), "after a terminator");
    EXPECT_DEATH(static_cast<void>(Builder.createReturn()), "after a terminator");
    EXPECT_DEATH(static_cast<void>(Builder.createReturn(Integer)), "after a terminator");
    EXPECT_DEATH(static_cast<void>(Builder.createBranch(Target)), "after a terminator");
    EXPECT_DEATH(static_cast<void>(Builder.createCondBranch(Condition, Target, Target)), "after a terminator");
    EXPECT_EQ(Source.instructionCount(), 2U);
    EXPECT_EQ(Source.terminator(), &Return);
    EXPECT_EQ(Builder.getInsertBlock(), &Source);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
  }
#endif

  // A builder can insert ordinary instructions before an existing return and then create a branch in another block.
  TEST(IRBranchInstructionTest, ReusesBuilderForInsertionBeforeReturnAndBranchesInAnotherBlock)
  {
    TypePool Types;
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    BasicBlock &Source = FunctionValue.createBasicBlock();
    BasicBlock &Target = FunctionValue.createBasicBlock();
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant Integer(Int32, 1);
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Source);
    AllocaInstruction *Slot = &Builder.createAlloca(Int32);
    ReturnInstruction *Return = &Builder.createReturn();
    EXPECT_TRUE(Return->isTerminator());
    EXPECT_FALSE(Slot->isTerminator());
    EXPECT_EQ(Source.instructionCount(), 2U);
    EXPECT_EQ(Source.terminator(), Return);
    EXPECT_EQ(Builder.getInsertBlock(), &Source);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Return));
    AddInstruction *Sum = &Builder.createAdd(Integer, Integer);
    AllocaInstruction *SecondSlot = &Builder.createAlloca(Int32);
    LoadInstruction *Load = &Builder.createLoad(*Slot);
    StoreInstruction *Store = &Builder.createStore(*Slot, Integer);
    EXPECT_EQ(Builder.getInsertBefore(), Return);
    EXPECT_EQ(Source.terminator(), Return);
    const Instruction *Expected[] = {
        Slot,
        Sum,
        SecondSlot,
        Load,
        Store,
        Return,
    };
    auto Iterator = Source.instructions().begin();
    for (const Instruction *InstructionValue : Expected)
    {
      ASSERT_NE(Iterator, Source.instructions().end());
      EXPECT_EQ(&*Iterator++, InstructionValue);
    }
    EXPECT_EQ(Iterator, Source.instructions().end());
    Builder.setInsertPoint(Target);
    BranchInstruction *Branch = &Builder.createBranch(Source);
    EXPECT_EQ(Target.terminator(), Branch);
    EXPECT_EQ(&Branch->getTarget(), &Source);
    EXPECT_TRUE(FunctionValue.hasValidControlFlow());
  }
} // namespace ink::ir::test
