#include "ink/ir/ir_builder.h"
#include "ownership_test_support.h"

#include "ink/ir/function/basic_block.h"
#include "ink/ir/context.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_same_v<decltype(std::declval<BasicBlock &>().values()), const std::vector<std::unique_ptr<Value>> &>);

  // Failed transfers preserve the caller's owner; successful transfers empty it and reject reuse.
  TEST(IRBasicBlockTest, AppendRejectsExistingParentsAndForeignValues)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    IRContext Other(Compilation);
    IRBuilder OtherFactory(Other);
    auto FirstOwner = Factory.createBasicBlock();
    BasicBlock *First = FirstOwner.get();
    auto SecondOwner = Factory.createBasicBlock();
    BasicBlock *Second = SecondOwner.get();
    auto ChildOwner = Factory.createBasicBlock();
    BasicBlock *Child = ChildOwner.get();
    auto ForeignOwner = OtherFactory.createBasicBlock();
    BasicBlock *Foreign = ForeignOwner.get();
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Child, nullptr);
    ASSERT_NE(Foreign, nullptr);
    EXPECT_EQ(Child->outer(), nullptr);
    EXPECT_FALSE(OtherFactory.appendValue(*First, std::move(ChildOwner)));
    EXPECT_FALSE(Factory.appendValue(*Foreign, std::move(ChildOwner)));
    EXPECT_EQ(ChildOwner.get(), Child);
    ASSERT_TRUE(Factory.appendValue(*First, std::move(ChildOwner)));
    EXPECT_EQ(ChildOwner, nullptr);
    EXPECT_EQ(Child->outer(), First);
    EXPECT_FALSE(Factory.appendValue(*First, std::move(ChildOwner)));
    EXPECT_FALSE(Factory.appendValue(*Second, std::move(ChildOwner)));
    EXPECT_FALSE(Factory.appendValue(*First, std::move(ForeignOwner)));
    EXPECT_FALSE(OtherFactory.removeValue(*First, *Child));
    ASSERT_EQ(First->values().size(), 1U);
    EXPECT_EQ(First->values()[0].get(), Child);
    EXPECT_TRUE(Second->values().empty());
    EXPECT_EQ(Child->outer(), First);
    EXPECT_EQ(Foreign->outer(), nullptr);
  }

  // Ancestor checks include module-to-entry-block edges and prevent self-insertion and entry-block theft.
  TEST(IRBasicBlockTest, CyclesAndModuleEntryReparentingAreRejected)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    auto DetachedOwner = Factory.createBasicBlock();
    BasicBlock *Detached = DetachedOwner.get();
    Module *Root = Factory.createModule(Context.namePool().intern("Root"));
    Module *Nested = Factory.createModule(Context.namePool().intern("Nested"));
    ASSERT_NE(Detached, nullptr);
    ASSERT_NE(Root, nullptr);
    ASSERT_NE(Nested, nullptr);
    auto RootOwner = Factory.removeModule(*Root);
    auto NestedOwner = Factory.removeModule(*Nested);
    ASSERT_NE(RootOwner, nullptr);
    ASSERT_NE(NestedOwner, nullptr);
    EXPECT_FALSE(Factory.appendValue(*Detached, std::move(DetachedOwner)));
    EXPECT_EQ(Factory.removeValue(*Detached, Root->entryBlock()), nullptr);
    EXPECT_FALSE(Factory.appendValue(Root->entryBlock(), std::move(RootOwner)));
    ASSERT_TRUE(Factory.appendValue(Root->entryBlock(), std::move(NestedOwner)));
    EXPECT_FALSE(Factory.appendValue(Nested->entryBlock(), std::move(RootOwner)));
    EXPECT_EQ(Factory.removeValue(Nested->entryBlock(), Root->entryBlock()), nullptr);
    EXPECT_EQ(Detached->outer(), nullptr);
    EXPECT_TRUE(Detached->values().empty());
    EXPECT_EQ(Root->outer(), nullptr);
    EXPECT_EQ(Root->entryBlock().outer(), Root);
    EXPECT_EQ(Nested->outer(), &Root->entryBlock());
    EXPECT_EQ(Nested->entryBlock().outer(), Nested);
    EXPECT_TRUE(Nested->entryBlock().values().empty());
    ASSERT_EQ(Root->entryBlock().values().size(), 1U);
    EXPECT_EQ(Root->entryBlock().values()[0].get(), Nested);
    ASSERT_TRUE(Factory.appendModule(std::move(RootOwner)));
    EXPECT_EQ(Context.modules().size(), 1U);
  }

  // Detaching a function permits explicit reparenting without changing calls or parenting shared types and constants.
  TEST(IRBasicBlockTest, RemovalPreservesReferencesAndAllowsReparenting)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    auto FirstOwner = Factory.createBasicBlock();
    BasicBlock *First = FirstOwner.get();
    auto SecondOwner = Factory.createBasicBlock();
    BasicBlock *Second = SecondOwner.get();
    const Type *Parameters[] = {&Context.typePool().getType<TypeKind::Bool>()};
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>(), Parameters);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Signature, nullptr);
    auto TargetOwner = Factory.createFunction(Context.namePool().intern("Target"), *Signature);
    Function *Target = TargetOwner.get();
    ASSERT_NE(Target, nullptr);
    const Value *Arguments[] = {&Context.constantPool().getBoolConstant(true)};
    auto CallOwner = Factory.createDetachedCallInstruction(*Target, Arguments);
    CallInstruction *Call = CallOwner.get();
    ASSERT_NE(Call, nullptr);
    ASSERT_TRUE(Factory.appendValue(*First, std::move(TargetOwner)));
    ASSERT_TRUE(Factory.appendValue(*First, std::move(CallOwner)));
    EXPECT_FALSE(Factory.removeValue(*Second, *Target));
    EXPECT_EQ(Target->outer(), First);
    auto RemovedTargetOwner = Factory.removeValue(*First, *Target);
    ASSERT_EQ(RemovedTargetOwner.get(), Target);
    EXPECT_EQ(Target->outer(), nullptr);
    EXPECT_FALSE(Factory.removeValue(*First, *Target));
    ASSERT_TRUE(Factory.appendValue(*Second, std::move(RemovedTargetOwner)));
    EXPECT_EQ(Target->outer(), Second);
    EXPECT_EQ(Call->outer(), First);
    EXPECT_EQ(Call->directCallee(), Target);
    EXPECT_EQ(Call->arguments()[0], Arguments[0]);
    EXPECT_EQ(Arguments[0]->outer(), nullptr);
    EXPECT_EQ(Signature->outer(), nullptr);
    EXPECT_EQ(Context.typePool().getType<TypeKind::Bool>().outer(), nullptr);
    ASSERT_EQ(First->values().size(), 1U);
    EXPECT_EQ(First->values()[0].get(), Call);
    ASSERT_EQ(Second->values().size(), 1U);
    EXPECT_EQ(Second->values()[0].get(), Target);
  }

  // Insertion rejects detached, foreign and wrong-block anchors while preserving ownership and cycle checks.
  TEST(IRBasicBlockTest, InsertBeforeValidatesAnchorsAndParentRelationships)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    IRContext Foreign(Compilation);
    IRBuilder ForeignFactory(Foreign);
    auto FirstOwner = Factory.createBasicBlock();
    BasicBlock *First = FirstOwner.get();
    auto SecondOwner = Factory.createBasicBlock();
    BasicBlock *Second = SecondOwner.get();
    auto ForeignBlockOwner = ForeignFactory.createBasicBlock();
    BasicBlock *ForeignBlock = ForeignBlockOwner.get();
    auto AnchorOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *Anchor = AnchorOwner.get();
    auto ChildOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *Child = ChildOwner.get();
    auto ForeignChildOwner = ForeignFactory.createDetachedAllocaInstruction(Foreign.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *ForeignChild = ForeignChildOwner.get();
    EXPECT_FALSE(Factory.insertValue(*First, std::move(ChildOwner), Anchor));
    EXPECT_EQ(Child->outer(), nullptr);
    ASSERT_TRUE(Factory.appendValue(*Second, std::move(AnchorOwner)));
    EXPECT_FALSE(Factory.insertValue(*First, std::move(ChildOwner), Anchor));
    EXPECT_FALSE(Factory.insertValue(*ForeignBlock, std::move(ChildOwner)));
    EXPECT_FALSE(ForeignFactory.insertValue(*Second, std::move(ChildOwner), Anchor));
    EXPECT_FALSE(Factory.insertValue(*Second, std::move(ForeignChildOwner), Anchor));
    EXPECT_TRUE(First->values().empty());
    EXPECT_EQ(borrowedPointers(Second->values()), std::vector<Value *>({Anchor}));
    ASSERT_TRUE(Factory.insertValue(*Second, std::move(ChildOwner), Anchor));
    EXPECT_EQ(Child->outer(), Second);
    EXPECT_EQ(borrowedPointers(Second->values()), std::vector<Value *>({Child, Anchor}));
    EXPECT_FALSE(Factory.insertValue(*Second, std::move(ChildOwner), Anchor));
    EXPECT_FALSE(Factory.insertValue(*Second, std::move(SecondOwner), Anchor));
    ASSERT_TRUE(Factory.insertValue(*First, std::move(SecondOwner)));
    EXPECT_FALSE(Factory.insertValue(*Second, std::move(FirstOwner), Anchor));
    EXPECT_EQ(First->outer(), nullptr);
    EXPECT_EQ(Second->outer(), First);
    EXPECT_EQ(Child->outer(), Second);
    EXPECT_EQ(ForeignChild->outer(), nullptr);
  }

  // Direct context edits enforce terminal returns, allow insertion before them, and allow replacing a removed return.
  TEST(IRBasicBlockTest, DirectInsertionPreservesTerminalReturns)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    auto MainOwner = Factory.createFunction(Context.namePool().intern("Main"), *Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>()));
    Function *Main = MainOwner.get();
    BasicBlock *Block = Factory.createFunctionBody(*Main);
    auto SlotOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *Slot = SlotOwner.get();
    auto ReturnOwner = Factory.createDetachedReturnInstruction();
    ReturnInstruction *Return = ReturnOwner.get();
    auto ReplacementOwner = Factory.createDetachedReturnInstruction();
    ReturnInstruction *Replacement = ReplacementOwner.get();
    ASSERT_TRUE(Factory.appendValue(*Block, std::move(SlotOwner)));
    EXPECT_FALSE(Factory.insertValue(*Block, std::move(ReturnOwner), Slot));
    EXPECT_EQ(Return->outer(), nullptr);
    ASSERT_TRUE(Factory.appendValue(*Block, std::move(ReturnOwner)));
    auto ExtraOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *Extra = ExtraOwner.get();
    EXPECT_FALSE(Factory.appendValue(*Block, std::move(ExtraOwner)));
    EXPECT_FALSE(Factory.appendValue(*Block, std::move(ReplacementOwner)));
    EXPECT_FALSE(Factory.insertValue(*Block, std::move(ReplacementOwner), Return));
    EXPECT_EQ(Extra->outer(), nullptr);
    EXPECT_EQ(Replacement->outer(), nullptr);
    ASSERT_TRUE(Factory.insertValue(*Block, std::move(ExtraOwner), Return));
    EXPECT_EQ(borrowedPointers(Block->values()), std::vector<Value *>({Slot, Extra, Return}));
    auto RemovedReturnOwner = Factory.removeValue(*Block, *Return);
    ASSERT_EQ(RemovedReturnOwner.get(), Return);
    ASSERT_TRUE(Factory.appendValue(*Block, std::move(ReplacementOwner)));
    EXPECT_EQ(borrowedPointers(Block->values()), std::vector<Value *>({Slot, Extra, Replacement}));
    EXPECT_EQ(Return->outer(), nullptr);
    EXPECT_EQ(Replacement->outer(), Block);
  }
} // namespace ink::ir::test
