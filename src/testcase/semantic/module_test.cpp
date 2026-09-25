#include "ink/semantic/model/module/module.h"
#include "ink/semantic/context.h"

#include <gtest/gtest.h>

namespace ink::semantic::test
{
  // A module entry block retains mixed value kinds and their order across context storage growth.
  TEST(SemanticModuleTest, EntryBlockPreservesMixedValuesAndIdentity)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Name ModuleName = Context.namePool().intern("Example");
    Module *Object = Context.createModule(ModuleName);
    ASSERT_NE(Object, nullptr);
    BasicBlock *Entry = &Object->entryBlock();
    EXPECT_EQ(Object->outer(), nullptr);
    EXPECT_EQ(Entry->outer(), Object);
    EXPECT_TRUE(Entry->values().empty());
    const FunctionType *Signature = Context.getFunctionType(Context.getBoolType());
    ASSERT_NE(Signature, nullptr);
    Function *Target = Context.createFunction(Context.namePool().intern("Ready"), *Signature);
    ASSERT_NE(Target, nullptr);
    CallInstruction *Call = Context.createCallInstruction(*Target);
    AllocaInstruction *Slot = Context.createAllocaInstruction(Context.getBoolType());
    Module *Nested = Context.createModule(Context.namePool().intern("Nested"));
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(Target->outer(), nullptr);
    EXPECT_EQ(Call->outer(), nullptr);
    ASSERT_TRUE(Context.appendValue(*Entry, *Target));
    ASSERT_TRUE(Context.appendValue(*Entry, *Call));
    ASSERT_TRUE(Context.appendValue(*Entry, *Slot));
    ASSERT_TRUE(Context.appendValue(*Entry, *Nested));
    for (unsigned Index = 0; Index < 64; ++Index)
    {
      Module *Next = Context.createModule(ModuleName);
      ASSERT_NE(Next, nullptr);
      EXPECT_NE(Next, Object);
      EXPECT_NE(&Next->entryBlock(), Entry);
      EXPECT_TRUE(Next->entryBlock().values().empty());
      EXPECT_EQ(Next->entryBlock().outer(), Next);
    }

    const Module &View = *Object;
    EXPECT_EQ(View.name(), ModuleName);
    EXPECT_EQ(&View.type(), &Context.getModuleType());
    EXPECT_EQ(&View.entryBlock(), Entry);
    EXPECT_EQ(&View.entryBlock().type(), &Context.getLabelType());
    EXPECT_EQ(View.entryBlock().outer(), &View);
    const auto &Values = View.entryBlock().values();
    ASSERT_EQ(Values.size(), 4U);
    EXPECT_EQ(Values[0], Target);
    EXPECT_EQ(Values[1], Call);
    EXPECT_EQ(Values[2], Slot);
    EXPECT_EQ(Values[3], Nested);
    EXPECT_TRUE(Function::classof(Values[0]));
    EXPECT_TRUE(CallInstruction::classof(Values[1]));
    EXPECT_TRUE(AllocaInstruction::classof(Values[2]));
    EXPECT_TRUE(Module::classof(Values[3]));
    EXPECT_EQ(Call->directCallee(), Target);
    EXPECT_EQ(Target->outer(), Entry);
    EXPECT_EQ(Call->outer(), Entry);
    EXPECT_EQ(Slot->outer(), Entry);
    EXPECT_EQ(Nested->outer(), Entry);
    EXPECT_EQ(Nested->entryBlock().outer(), Nested);
    EXPECT_TRUE(Nested->entryBlock().values().empty());
  }

  // Module factories reject invalid name indices and keep module and entry-block types local to each context.
  TEST(SemanticModuleTest, NamesAreValidatedAndTypesRemainContextLocal)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    EXPECT_EQ(Context.createModule(Name{}), nullptr);
    const Name ForeignName = Other.namePool().intern("Foreign");
    EXPECT_EQ(Context.createModule(ForeignName), nullptr);
    Module *Local = Context.createModule(Context.namePool().intern("Local"));
    Module *Foreign = Other.createModule(ForeignName);
    ASSERT_NE(Local, nullptr);
    ASSERT_NE(Foreign, nullptr);
    EXPECT_EQ(&Local->context(), &Context);
    EXPECT_EQ(&Local->entryBlock().context(), &Context);
    EXPECT_EQ(&Foreign->context(), &Other);
    EXPECT_EQ(&Foreign->entryBlock().context(), &Other);
    EXPECT_EQ(&Local->type().context(), &Context);
    EXPECT_EQ(&Local->entryBlock().type().context(), &Context);
    EXPECT_EQ(&Foreign->type().context(), &Other);
    EXPECT_EQ(&Foreign->entryBlock().type().context(), &Other);
    EXPECT_NE(&Local->type(), &Foreign->type());
    EXPECT_EQ(Local->type().typeKind(), TypeKind::Module);
    EXPECT_EQ(Context.createCallInstruction(*Local), nullptr);
  }
} // namespace ink::semantic::test
