#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir::test
{
  static_assert(std::is_same_v<decltype(std::declval<BasicBlock &>().terminator()), Instruction *>);
  static_assert(std::is_same_v<decltype(std::declval<const BasicBlock &>().terminator()), const Instruction *>);
  static_assert(std::is_same_v<decltype(std::declval<BasicBlock &>().successor(0)), BasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<const BasicBlock &>().successor(0)), const BasicBlock &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<BasicBlock &>().successors())>, BasicBlock &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const BasicBlock &>().successors())>, const BasicBlock &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<BasicBlock &>().predecessors())>, BasicBlock &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const BasicBlock &>().predecessors())>, const BasicBlock &>);

  namespace
  {
    template <typename Range>
    std::vector<const BasicBlock *> blockAddresses(Range &&Blocks)
    {
      std::vector<const BasicBlock *> Result;
      for (auto &Block : Blocks)
      {
        Result.push_back(&Block);
      }
      return Result;
    }
  } // namespace

  // Declarations have valid empty control flow, while each defined block remains incomplete until it ends in a terminator.
  TEST(IRControlFlowTest, DistinguishesDeclarationsFromIncompleteAndCompletedDefinitions)
  {
    TypePool Types;
    Function FunctionValue(Types.getFunctionType(Types.getVoidType()));
    EXPECT_TRUE(FunctionValue.hasValidControlFlow());
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    EXPECT_FALSE(FunctionValue.hasValidControlFlow());
    EXPECT_EQ(Entry.terminator(), nullptr);
    EXPECT_FALSE(Entry.isTerminated());
    EXPECT_EQ(Entry.successorCount(), 0U);
    EXPECT_TRUE(std::ranges::empty(Entry.successors()));
    EXPECT_TRUE(std::ranges::empty(Entry.predecessors()));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    static_cast<void>(Builder.createAlloca(Types.getBoolType()));
    EXPECT_FALSE(Entry.isTerminated());
    EXPECT_EQ(Entry.terminator(), nullptr);
    EXPECT_EQ(Entry.successorCount(), 0U);
    EXPECT_FALSE(FunctionValue.hasValidControlFlow());
    ReturnInstruction *Return = &Builder.createReturn();
    EXPECT_TRUE(Entry.isTerminated());
    EXPECT_EQ(Entry.terminator(), Return);
    EXPECT_EQ(Entry.successorCount(), 0U);
    EXPECT_TRUE(std::ranges::empty(Entry.successors()));
    EXPECT_TRUE(FunctionValue.hasValidControlFlow());
    BasicBlock &Incomplete = FunctionValue.createBasicBlock();
    EXPECT_FALSE(FunctionValue.hasValidControlFlow());
    Builder.setInsertPoint(Incomplete);
    static_cast<void>(Builder.createReturn());
    const Function &ConstFunction = FunctionValue;
    const BasicBlock &ConstEntry = Entry;
    EXPECT_TRUE(ConstFunction.hasValidControlFlow());
    EXPECT_EQ(ConstEntry.terminator(), Return);
    EXPECT_TRUE(ConstEntry.isTerminated());
    EXPECT_EQ(ConstEntry.successorCount(), 0U);
    EXPECT_TRUE(std::ranges::empty(ConstEntry.successors()));
    EXPECT_TRUE(std::ranges::empty(ConstEntry.predecessors()));
    BasicBlock Detached;
    EXPECT_EQ(Detached.terminator(), nullptr);
    EXPECT_FALSE(Detached.isTerminated());
    EXPECT_EQ(Detached.successorCount(), 0U);
    EXPECT_TRUE(std::ranges::empty(Detached.successors()));
    EXPECT_TRUE(std::ranges::empty(Detached.predecessors()));
  }

  // A diamond exposes ordered true and false successors and finds both incoming edges to its merge block through const queries.
  TEST(IRControlFlowTest, QueriesDiamondSuccessorsAndPredecessorsWithConstPropagation)
  {
    TypePool Types;
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    BasicBlock &Left = FunctionValue.createBasicBlock();
    BasicBlock &Right = FunctionValue.createBasicBlock();
    BasicBlock &Merge = FunctionValue.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    CondBranchInstruction *Conditional = &Builder.createCondBranch(FunctionValue.argument(0), Left, Right);
    Builder.setInsertPoint(Left);
    static_cast<void>(Builder.createBranch(Merge));
    Builder.setInsertPoint(Right);
    static_cast<void>(Builder.createBranch(Merge));
    Builder.setInsertPoint(Merge);
    static_cast<void>(Builder.createReturn());
    EXPECT_TRUE(FunctionValue.hasValidControlFlow());
    EXPECT_EQ(Entry.terminator(), Conditional);
    EXPECT_EQ(Entry.successorCount(), 2U);
    EXPECT_EQ(&Entry.successor(0), &Left);
    EXPECT_EQ(&Entry.successor(1), &Right);
    EXPECT_EQ(blockAddresses(Entry.successors()), (std::vector<const BasicBlock *>{&Left, &Right}));
    EXPECT_EQ(blockAddresses(Left.successors()), (std::vector<const BasicBlock *>{&Merge}));
    EXPECT_EQ(blockAddresses(Right.successors()), (std::vector<const BasicBlock *>{&Merge}));
    EXPECT_EQ(blockAddresses(Merge.predecessors()), (std::vector<const BasicBlock *>{&Left, &Right}));
    EXPECT_EQ(blockAddresses(Left.predecessors()), (std::vector<const BasicBlock *>{&Entry}));
    EXPECT_EQ(blockAddresses(Right.predecessors()), (std::vector<const BasicBlock *>{&Entry}));
    EXPECT_TRUE(std::ranges::empty(Entry.predecessors()));
    EXPECT_TRUE(std::ranges::empty(Merge.successors()));
    const BasicBlock &ConstEntry = Entry;
    const BasicBlock &ConstMerge = Merge;
    EXPECT_EQ(&ConstEntry.successor(0), &Left);
    EXPECT_EQ(&ConstEntry.successor(1), &Right);
    EXPECT_EQ(blockAddresses(ConstEntry.successors()), (std::vector<const BasicBlock *>{&Left, &Right}));
    EXPECT_EQ(blockAddresses(ConstMerge.predecessors()), (std::vector<const BasicBlock *>{&Left, &Right}));
  }

  // Back edges and self loops remain valid control flow and appear in predecessor scans without recursive traversal.
  TEST(IRControlFlowTest, PreservesLoopBackEdgesAndSelfLoops)
  {
    TypePool Types;
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    BasicBlock &Header = FunctionValue.createBasicBlock();
    BasicBlock &Body = FunctionValue.createBasicBlock();
    BasicBlock &Exit = FunctionValue.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    static_cast<void>(Builder.createBranch(Header));
    Builder.setInsertPoint(Header);
    static_cast<void>(Builder.createCondBranch(FunctionValue.argument(0), Body, Exit));
    Builder.setInsertPoint(Body);
    static_cast<void>(Builder.createBranch(Header));
    Builder.setInsertPoint(Exit);
    static_cast<void>(Builder.createReturn());
    EXPECT_TRUE(FunctionValue.hasValidControlFlow());
    EXPECT_EQ(blockAddresses(Header.predecessors()), (std::vector<const BasicBlock *>{&Entry, &Body}));
    EXPECT_EQ(blockAddresses(Header.successors()), (std::vector<const BasicBlock *>{&Body, &Exit}));
    EXPECT_EQ(blockAddresses(Body.predecessors()), (std::vector<const BasicBlock *>{&Header}));
    EXPECT_EQ(blockAddresses(Body.successors()), (std::vector<const BasicBlock *>{&Header}));
    EXPECT_EQ(blockAddresses(Exit.predecessors()), (std::vector<const BasicBlock *>{&Header}));
    Function SelfLoopFunction(Types.getFunctionType(Types.getVoidType()));
    BasicBlock &SelfLoop = SelfLoopFunction.createBasicBlock();
    Builder.setInsertPoint(SelfLoop);
    static_cast<void>(Builder.createBranch(SelfLoop));
    EXPECT_TRUE(SelfLoopFunction.hasValidControlFlow());
    EXPECT_EQ(&SelfLoop.successor(0), &SelfLoop);
    EXPECT_EQ(blockAddresses(SelfLoop.successors()), (std::vector<const BasicBlock *>{&SelfLoop}));
    EXPECT_EQ(blockAddresses(SelfLoop.predecessors()), (std::vector<const BasicBlock *>{&SelfLoop}));
  }

  // A conditional branch with equal targets retains two successor edges and two occurrences of its predecessor.
  TEST(IRControlFlowTest, PreservesDuplicateConditionalEdgesInBothDirections)
  {
    TypePool Types;
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    BasicBlock &Target = FunctionValue.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    static_cast<void>(Builder.createCondBranch(FunctionValue.argument(0), Target, Target));
    Builder.setInsertPoint(Target);
    static_cast<void>(Builder.createReturn());
    EXPECT_TRUE(FunctionValue.hasValidControlFlow());
    EXPECT_EQ(Entry.successorCount(), 2U);
    EXPECT_EQ(&Entry.successor(0), &Target);
    EXPECT_EQ(&Entry.successor(1), &Target);
    EXPECT_EQ(blockAddresses(Entry.successors()), (std::vector<const BasicBlock *>{&Target, &Target}));
    EXPECT_EQ(blockAddresses(Target.predecessors()), (std::vector<const BasicBlock *>{&Entry, &Entry}));
    const BasicBlock &ConstTarget = Target;
    EXPECT_EQ(blockAddresses(ConstTarget.predecessors()), (std::vector<const BasicBlock *>{&Entry, &Entry}));
  }

  // Block and branch identities survive builder destruction, later block growth, and ordinary insertion before an existing branch.
  TEST(IRControlFlowTest, KeepsBranchTargetsAndOwnedInstructionsStableWhileGrowing)
  {
    TypePool Types;
    Function FunctionValue(Types.getFunctionType(Types.getVoidType()));
    BasicBlock &Entry = FunctionValue.createBasicBlock();
    BasicBlock &Exit = FunctionValue.createBasicBlock();
    BranchInstruction *Branch = nullptr;
    ReturnInstruction *Return = nullptr;
    {
      InstructionBuilder Builder(Types);
      Builder.setInsertPoint(Entry);
      Branch = &Builder.createBranch(Exit);
      Builder.setInsertPoint(Exit);
      Return = &Builder.createReturn();
    }
    auto OriginalBlock = FunctionValue.basicBlocks().begin();
    auto OriginalInstruction = Entry.instructions().begin();
    InstructionBuilder Builder(Types);
    for (unsigned Index = 0; Index < 128; ++Index)
    {
      BasicBlock &NewBlock = FunctionValue.createBasicBlock();
      Builder.setInsertPoint(NewBlock);
      static_cast<void>(Builder.createReturn());
    }
    ASSERT_TRUE(Builder.setInsertPoint(*Branch));
    static_cast<void>(Builder.createAlloca(Types.getBoolType()));
    EXPECT_EQ(&*OriginalBlock, &Entry);
    EXPECT_EQ(&*OriginalInstruction, Branch);
    EXPECT_EQ(Branch->parent(), &Entry);
    EXPECT_EQ(Return->parent(), &Exit);
    EXPECT_EQ(Entry.terminator(), Branch);
    EXPECT_EQ(Exit.terminator(), Return);
    EXPECT_EQ(&Branch->getTarget(), &Exit);
    EXPECT_EQ(&Entry.successor(0), &Exit);
    EXPECT_EQ(blockAddresses(Exit.predecessors()), (std::vector<const BasicBlock *>{&Entry}));
    EXPECT_TRUE(FunctionValue.hasValidControlFlow());
  }
} // namespace ink::ir::test
