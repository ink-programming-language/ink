#include "ink/ir/ir_builder.h"
#include "ink/ir/module/module.h"
#include "ink/ir/context.h"

#include <gtest/gtest.h>

namespace ink::ir::test
{
  // A module entry block retains mixed value kinds and their order across context storage growth.
  TEST(IRModuleTest, EntryBlockPreservesMixedValuesAndIdentity)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const Name ModuleName = Context.namePool().intern("Example");
    Module *Object = Factory.createModule(ModuleName);
    ASSERT_NE(Object, nullptr);
    BasicBlock *Entry = &Object->entryBlock();
    EXPECT_EQ(Object->outer(), nullptr);
    EXPECT_EQ(Entry->outer(), Object);
    EXPECT_TRUE(Entry->values().empty());
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>());
    ASSERT_NE(Signature, nullptr);
    auto TargetOwner = Factory.createFunction(Context.namePool().intern("Ready"), *Signature);
    Function *Target = TargetOwner.get();
    ASSERT_NE(Target, nullptr);
    auto CallOwner = Factory.createDetachedCallInstruction(*Target);
    CallInstruction *Call = CallOwner.get();
    auto SlotOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *Slot = SlotOwner.get();
    Module *Nested = Factory.createModule(Context.namePool().intern("Nested"));
    auto NestedOwner = Factory.removeModule(*Nested);
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(Target->outer(), nullptr);
    EXPECT_EQ(Call->outer(), nullptr);
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(TargetOwner)));
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(CallOwner)));
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(SlotOwner)));
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(NestedOwner)));
    for (unsigned Index = 0; Index < 64; ++Index)
    {
      Module *Next = Factory.createModule(ModuleName);
      ASSERT_NE(Next, nullptr);
      EXPECT_NE(Next, Object);
      EXPECT_NE(&Next->entryBlock(), Entry);
      EXPECT_TRUE(Next->entryBlock().values().empty());
      EXPECT_EQ(Next->entryBlock().outer(), Next);
    }

    const Module &View = *Object;
    EXPECT_EQ(View.name(), ModuleName);
    EXPECT_EQ(&View.type(), &Context.typePool().getType<TypeKind::Module>());
    EXPECT_EQ(&View.entryBlock(), Entry);
    EXPECT_EQ(&View.entryBlock().type(), &Context.typePool().getType<TypeKind::Label>());
    EXPECT_EQ(View.entryBlock().outer(), &View);
    const auto &Values = View.entryBlock().values();
    ASSERT_EQ(Values.size(), 4U);
    EXPECT_EQ(Values[0].get(), Target);
    EXPECT_EQ(Values[1].get(), Call);
    EXPECT_EQ(Values[2].get(), Slot);
    EXPECT_EQ(Values[3].get(), Nested);
    EXPECT_TRUE(Function::classof(Values[0].get()));
    EXPECT_TRUE(CallInstruction::classof(Values[1].get()));
    EXPECT_TRUE(AllocaInstruction::classof(Values[2].get()));
    EXPECT_TRUE(Module::classof(Values[3].get()));
    EXPECT_EQ(Call->directCallee(), Target);
    EXPECT_EQ(Target->outer(), Entry);
    EXPECT_EQ(Call->outer(), Entry);
    EXPECT_EQ(Slot->outer(), Entry);
    EXPECT_EQ(Nested->outer(), Entry);
    EXPECT_EQ(Nested->entryBlock().outer(), Nested);
    EXPECT_TRUE(Nested->entryBlock().values().empty());
  }

  // Module factories reject invalid name indices and keep module and entry-block types local to each context.
  TEST(IRModuleTest, NamesAreValidatedAndTypesRemainContextLocal)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    IRContext Other(Compilation);
    IRBuilder OtherFactory(Other);
    EXPECT_EQ(Factory.createModule(Name{}), nullptr);
    const Name ForeignName = Other.namePool().intern("Foreign");
    EXPECT_EQ(Factory.createModule(ForeignName), nullptr);
    Module *Local = Factory.createModule(Context.namePool().intern("Local"));
    Module *Foreign = OtherFactory.createModule(ForeignName);
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
    EXPECT_EQ(Factory.createDetachedCallInstruction(*Local), nullptr);
  }
} // namespace ink::ir::test
