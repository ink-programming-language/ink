#include "ink/semantic/model/function/basic_block.h"
#include "ink/semantic/context.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

namespace ink::semantic::test
{
  static_assert(std::is_same_v<decltype(std::declval<BasicBlock &>().values()), const std::vector<Value *> &>);

  // Failed insertions preserve both containers and reject duplicate, foreign and already-parented values.
  TEST(SemanticBasicBlockTest, AppendRejectsExistingParentsAndForeignValues)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    BasicBlock *First = Context.createBasicBlock();
    BasicBlock *Second = Context.createBasicBlock();
    BasicBlock *Child = Context.createBasicBlock();
    BasicBlock *Foreign = Other.createBasicBlock();
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Child, nullptr);
    ASSERT_NE(Foreign, nullptr);
    EXPECT_EQ(Child->outer(), nullptr);
    EXPECT_FALSE(Other.appendValue(*First, *Child));
    EXPECT_FALSE(Context.appendValue(*Foreign, *Child));
    ASSERT_TRUE(Context.appendValue(*First, *Child));
    EXPECT_EQ(Child->outer(), First);
    EXPECT_FALSE(Context.appendValue(*First, *Child));
    EXPECT_FALSE(Context.appendValue(*Second, *Child));
    EXPECT_FALSE(Context.appendValue(*First, *Foreign));
    EXPECT_FALSE(Other.removeValue(*First, *Child));
    ASSERT_EQ(First->values().size(), 1U);
    EXPECT_EQ(First->values()[0], Child);
    EXPECT_TRUE(Second->values().empty());
    EXPECT_EQ(Child->outer(), First);
    EXPECT_EQ(Foreign->outer(), nullptr);
  }

  // Ancestor checks include module-to-entry-block edges and prevent self-insertion and entry-block theft.
  TEST(SemanticBasicBlockTest, CyclesAndModuleEntryReparentingAreRejected)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    BasicBlock *Detached = Context.createBasicBlock();
    Module *Root = Context.createModule(Context.namePool().intern("Root"));
    Module *Nested = Context.createModule(Context.namePool().intern("Nested"));
    ASSERT_NE(Detached, nullptr);
    ASSERT_NE(Root, nullptr);
    ASSERT_NE(Nested, nullptr);
    EXPECT_FALSE(Context.appendValue(*Detached, *Detached));
    EXPECT_FALSE(Context.appendValue(*Detached, Root->entryBlock()));
    EXPECT_FALSE(Context.appendValue(Root->entryBlock(), *Root));
    ASSERT_TRUE(Context.appendValue(Root->entryBlock(), *Nested));
    EXPECT_FALSE(Context.appendValue(Nested->entryBlock(), *Root));
    EXPECT_FALSE(Context.appendValue(Nested->entryBlock(), Root->entryBlock()));
    EXPECT_EQ(Detached->outer(), nullptr);
    EXPECT_TRUE(Detached->values().empty());
    EXPECT_EQ(Root->outer(), nullptr);
    EXPECT_EQ(Root->entryBlock().outer(), Root);
    EXPECT_EQ(Nested->outer(), &Root->entryBlock());
    EXPECT_EQ(Nested->entryBlock().outer(), Nested);
    EXPECT_TRUE(Nested->entryBlock().values().empty());
    ASSERT_EQ(Root->entryBlock().values().size(), 1U);
    EXPECT_EQ(Root->entryBlock().values()[0], Nested);
  }

  // Detaching a function permits explicit reparenting without changing calls or parenting shared types and constants.
  TEST(SemanticBasicBlockTest, RemovalPreservesReferencesAndAllowsReparenting)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    BasicBlock *First = Context.createBasicBlock();
    BasicBlock *Second = Context.createBasicBlock();
    const Type *Parameters[] = {&Context.getBoolType()};
    const FunctionType *Signature = Context.getFunctionType(Context.getBoolType(), Parameters);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Signature, nullptr);
    Function *Target = Context.createFunction(Context.namePool().intern("Target"), *Signature);
    ASSERT_NE(Target, nullptr);
    const Value *Arguments[] = {&Context.getBoolConstant(true)};
    CallInstruction *Call = Context.createCallInstruction(*Target, Arguments);
    ASSERT_NE(Call, nullptr);
    ASSERT_TRUE(Context.appendValue(*First, *Target));
    ASSERT_TRUE(Context.appendValue(*First, *Call));
    EXPECT_FALSE(Context.removeValue(*Second, *Target));
    EXPECT_EQ(Target->outer(), First);
    ASSERT_TRUE(Context.removeValue(*First, *Target));
    EXPECT_EQ(Target->outer(), nullptr);
    EXPECT_FALSE(Context.removeValue(*First, *Target));
    ASSERT_TRUE(Context.appendValue(*Second, *Target));
    EXPECT_EQ(Target->outer(), Second);
    EXPECT_EQ(Call->outer(), First);
    EXPECT_EQ(Call->directCallee(), Target);
    EXPECT_EQ(Call->arguments()[0], Arguments[0]);
    EXPECT_EQ(Arguments[0]->outer(), nullptr);
    EXPECT_EQ(Signature->outer(), nullptr);
    EXPECT_EQ(Context.getBoolType().outer(), nullptr);
    ASSERT_EQ(First->values().size(), 1U);
    EXPECT_EQ(First->values()[0], Call);
    ASSERT_EQ(Second->values().size(), 1U);
    EXPECT_EQ(Second->values()[0], Target);
  }
} // namespace ink::semantic::test
