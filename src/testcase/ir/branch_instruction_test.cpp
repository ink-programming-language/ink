#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

namespace ink::ir::test
{
  namespace
  {
    struct BranchTestContext
    {
        core::CompilationContext Compilation;
        IRContext Context{Compilation};
        IRBuilder Builder{Context};
        const Type &Void = Context.typePool().getType<TypeKind::Void>();
        const BoolConstant &Condition = Context.constantPool().getBoolConstant(true);
        std::unique_ptr<Function> Main = Builder.createFunction(Context.namePool().intern("Main"), *Context.typePool().getType<TypeKind::Function>(Void));
        BasicBlock *Entry = Builder.createBasicBlock(*Main);
        BasicBlock *Then = Builder.createBasicBlock(*Main);
        BasicBlock *Else = Builder.createBasicBlock(*Main);
    };
  } // namespace

  // Detached branches preserve operand identities, have void result types, and do not change the selected insertion point.
  TEST(IRBranchInstructionTest, DetachedFactoriesPreserveOperandsAndInsertionPoint)
  {
    BranchTestContext Test;
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Test.Entry));
    auto Branch = Test.Builder.createDetachedBranchInstruction(*Test.Then);
    auto Conditional = Test.Builder.createDetachedConditionalBranchInstruction(Test.Condition, *Test.Then, *Test.Else);
    ASSERT_NE(Branch, nullptr);
    ASSERT_NE(Conditional, nullptr);
    EXPECT_EQ(&Branch->target(), Test.Then);
    EXPECT_EQ(&Conditional->condition(), &Test.Condition);
    EXPECT_EQ(&Conditional->trueTarget(), Test.Then);
    EXPECT_EQ(&Conditional->falseTarget(), Test.Else);
    EXPECT_EQ(&Branch->type(), &Test.Void);
    EXPECT_EQ(&Conditional->type(), &Test.Void);
    EXPECT_EQ(Branch->outer(), nullptr);
    EXPECT_EQ(Conditional->outer(), nullptr);
    EXPECT_TRUE(Branch->isTerminator());
    EXPECT_TRUE(Conditional->isTerminator());
    EXPECT_FALSE(Test.Condition.isTerminator());
    EXPECT_TRUE(BranchInstruction::classof(Branch.get()));
    EXPECT_TRUE(ConditionalBranchInstruction::classof(Conditional.get()));
    EXPECT_FALSE(BranchInstruction::classof(Conditional.get()));
    EXPECT_FALSE(ConditionalBranchInstruction::classof(Branch.get()));
    EXPECT_FALSE(BranchInstruction::classof(nullptr));
    EXPECT_FALSE(ConditionalBranchInstruction::classof(nullptr));
    EXPECT_EQ(Test.Entry->terminator(), nullptr);
    EXPECT_TRUE(Test.Entry->values().empty());
    EXPECT_EQ(Test.Builder.insertBlock(), Test.Entry);
  }

  // Branch factories reject foreign contexts, non-bool conditions, module blocks, and successors from different functions.
  TEST(IRBranchInstructionTest, RejectsInvalidConditionsAndTargets)
  {
    BranchTestContext Test;
    BranchTestContext Foreign;
    auto Other = Test.Builder.createFunction(Test.Context.namePool().intern("Other"), *Test.Context.typePool().getType<TypeKind::Function>(Test.Void));
    auto *OtherBlock = Test.Builder.createBasicBlock(*Other);
    auto *ModuleValue = Test.Builder.createModule(Test.Context.namePool().intern("Root"));
    const auto *Int32 = Test.Context.typePool().getType<TypeKind::Integer>(32, true);
    const auto *One = Test.Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 1));
    EXPECT_EQ(Test.Builder.createDetachedBranchInstruction(*Foreign.Entry), nullptr);
    EXPECT_EQ(Test.Builder.createDetachedBranchInstruction(ModuleValue->entryBlock()), nullptr);
    EXPECT_EQ(Test.Builder.createDetachedConditionalBranchInstruction(*One, *Test.Then, *Test.Else), nullptr);
    EXPECT_EQ(Test.Builder.createDetachedConditionalBranchInstruction(Foreign.Condition, *Test.Then, *Test.Else), nullptr);
    EXPECT_EQ(Test.Builder.createDetachedConditionalBranchInstruction(Test.Condition, *Foreign.Entry, *Test.Else), nullptr);
    EXPECT_EQ(Test.Builder.createDetachedConditionalBranchInstruction(Test.Condition, *Test.Then, *Foreign.Entry), nullptr);
    EXPECT_EQ(Test.Builder.createDetachedConditionalBranchInstruction(Test.Condition, *Test.Then, *OtherBlock), nullptr);
    EXPECT_EQ(Test.Builder.createDetachedConditionalBranchInstruction(Test.Condition, *Test.Then, ModuleValue->entryBlock()), nullptr);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Test.Entry));
    EXPECT_EQ(Test.Builder.createConditionalBranchInstruction(*One, *Test.Then, *Test.Else), nullptr);
    EXPECT_TRUE(Test.Entry->values().empty());
  }

  // Detached targets may be referenced while constructing IR, but attachment requires source and all targets in the same function.
  TEST(IRBranchInstructionTest, AttachmentChecksFunctionOwnershipAndPreservesRejectedOwners)
  {
    BranchTestContext Test;
    auto Detached = Test.Builder.createBasicBlock();
    auto Other = Test.Builder.createFunction(Test.Context.namePool().intern("Other"), *Test.Context.typePool().getType<TypeKind::Function>(Test.Void));
    auto *OtherBlock = Test.Builder.createBasicBlock(*Other);
    auto Unresolved = Test.Builder.createDetachedBranchInstruction(*Detached);
    auto UnresolvedConditional = Test.Builder.createDetachedConditionalBranchInstruction(Test.Condition, *Test.Then, *Detached);
    auto Branch = Test.Builder.createDetachedBranchInstruction(*Test.Then);
    auto Conditional = Test.Builder.createDetachedConditionalBranchInstruction(Test.Condition, *Test.Then, *Test.Else);
    ASSERT_NE(Unresolved, nullptr);
    ASSERT_NE(UnresolvedConditional, nullptr);
    ASSERT_NE(Branch, nullptr);
    ASSERT_NE(Conditional, nullptr);
    EXPECT_FALSE(Test.Builder.appendValue(*Test.Entry, std::move(Unresolved)));
    EXPECT_FALSE(Test.Builder.appendValue(*Test.Entry, std::move(UnresolvedConditional)));
    EXPECT_FALSE(Test.Builder.appendValue(*Detached, std::move(Branch)));
    EXPECT_FALSE(Test.Builder.appendValue(*OtherBlock, std::move(Branch)));
    EXPECT_FALSE(Test.Builder.appendValue(*OtherBlock, std::move(Conditional)));
    EXPECT_NE(Unresolved, nullptr);
    EXPECT_NE(UnresolvedConditional, nullptr);
    EXPECT_NE(Branch, nullptr);
    EXPECT_NE(Conditional, nullptr);
    EXPECT_TRUE(Test.Entry->values().empty());
    EXPECT_TRUE(OtherBlock->values().empty());
    ASSERT_TRUE(Test.Builder.appendValue(*Test.Entry, std::move(Conditional)));
    EXPECT_TRUE(Test.Entry->terminator()->isTerminator());
    auto *InsertedBranch = Branch.get();
    EXPECT_TRUE(Test.Builder.appendValue(*Test.Else, std::move(Branch)));
    auto Removed = Test.Builder.removeValue(*Test.Else, *InsertedBranch);
    ASSERT_NE(Removed, nullptr);
    EXPECT_EQ(Test.Else->terminator(), nullptr);
    EXPECT_FALSE(Test.Builder.appendValue(*OtherBlock, std::move(Removed)));
    EXPECT_TRUE(Test.Builder.appendValue(*Test.Then, std::move(Removed)));
  }

  // Both branch forms terminate blocks, reject later instructions or earlier terminators, and permit ordinary insertion before them.
  TEST(IRBranchInstructionTest, BranchesEnforceTerminalPositionAcrossAllInsertionAPIs)
  {
    for (bool Conditional : {false, true})
    {
      BranchTestContext Test;
      ASSERT_TRUE(Test.Builder.setInsertPoint(*Test.Entry));
      auto *Slot = Test.Builder.createAllocaInstruction(Test.Condition.type());
      ASSERT_NE(Slot, nullptr);
      Value *Terminator = Conditional ? static_cast<Value *>(Test.Builder.createConditionalBranchInstruction(Test.Condition, *Test.Then, *Test.Else)) : Test.Builder.createBranchInstruction(*Test.Then);
      ASSERT_NE(Terminator, nullptr);
      EXPECT_EQ(Test.Entry->terminator(), Terminator);
      EXPECT_EQ(Test.Builder.createAllocaInstruction(Test.Condition.type()), nullptr);
      EXPECT_EQ(Test.Builder.createReturnInstruction(), nullptr);
      EXPECT_EQ(Test.Builder.createBranchInstruction(*Test.Then), nullptr);
      EXPECT_EQ(Test.Builder.createConditionalBranchInstruction(Test.Condition, *Test.Then, *Test.Else), nullptr);
      auto Extra = Test.Builder.createDetachedAllocaInstruction(Test.Condition.type());
      auto Replacement = Test.Builder.createDetachedBranchInstruction(*Test.Else);
      ASSERT_NE(Extra, nullptr);
      ASSERT_NE(Replacement, nullptr);
      EXPECT_FALSE(Test.Builder.appendValue(*Test.Entry, std::move(Extra)));
      EXPECT_FALSE(Test.Builder.insertValue(*Test.Entry, std::move(Replacement), Slot));
      EXPECT_NE(Extra, nullptr);
      EXPECT_NE(Replacement, nullptr);
      ASSERT_TRUE(Test.Builder.insertValue(*Test.Entry, std::move(Extra), Terminator));
      ASSERT_TRUE(Test.Builder.setInsertPoint(*Terminator));
      EXPECT_NE(Test.Builder.createLoadInstruction(*Slot), nullptr);
      EXPECT_EQ(Test.Builder.createReturnInstruction(), nullptr);
      EXPECT_EQ(Test.Builder.createBranchInstruction(*Test.Then), nullptr);
      EXPECT_EQ(Test.Builder.createConditionalBranchInstruction(Test.Condition, *Test.Then, *Test.Else), nullptr);
      EXPECT_EQ(Test.Entry->terminator(), Terminator);
      ASSERT_TRUE(Test.Builder.eraseValue(*Test.Entry, *Terminator));
      EXPECT_EQ(Test.Entry->terminator(), nullptr);
      ASSERT_TRUE(Test.Builder.appendValue(*Test.Entry, std::move(Replacement)));
      EXPECT_TRUE(BranchInstruction::classof(Test.Entry->terminator()));
    }
  }

  // Branches require an insertion point, accept self or duplicate successors, and cannot be added after a return.
  TEST(IRBranchInstructionTest, HandlesSelfEdgesDuplicateTargetsAndExistingReturns)
  {
    BranchTestContext Test;
    EXPECT_EQ(Test.Builder.createBranchInstruction(*Test.Entry), nullptr);
    EXPECT_EQ(Test.Builder.createConditionalBranchInstruction(Test.Condition, *Test.Then, *Test.Then), nullptr);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Test.Entry));
    auto *Self = Test.Builder.createBranchInstruction(*Test.Entry);
    ASSERT_NE(Self, nullptr);
    EXPECT_EQ(&Self->target(), Test.Entry);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Test.Then));
    auto *Duplicate = Test.Builder.createConditionalBranchInstruction(Test.Condition, *Test.Else, *Test.Else);
    ASSERT_NE(Duplicate, nullptr);
    EXPECT_EQ(&Duplicate->trueTarget(), &Duplicate->falseTarget());
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Test.Else));
    auto *Return = Test.Builder.createReturnInstruction();
    ASSERT_NE(Return, nullptr);
    EXPECT_TRUE(Return->isTerminator());
    EXPECT_EQ(Test.Else->terminator(), Return);
    EXPECT_EQ(Test.Builder.createBranchInstruction(*Test.Entry), nullptr);
    EXPECT_EQ(Test.Builder.createConditionalBranchInstruction(Test.Condition, *Test.Entry, *Test.Then), nullptr);
  }
} // namespace ink::ir::test
