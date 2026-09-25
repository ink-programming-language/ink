#include "ink/semantic/model/function/function.h"
#include "ink/semantic/context.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

namespace ink::semantic::test
{
  static_assert(std::is_base_of_v<Value, Function>);
  static_assert(!std::is_base_of_v<Decl, Function>);
  static_assert(!std::is_copy_constructible_v<Function>);
  static_assert(!std::is_move_constructible_v<Function>);
  static_assert(std::is_same_v<decltype(std::declval<Function &>().blocks()), const std::vector<BasicBlock *> &>);

  // Same-named functions share a structural signature while keeping independent callable identities.
  TEST(SemanticFunctionTest, ClosedFunctionsHaveIndependentValueIdentity)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const FunctionType *Signature = Context.getFunctionType(Context.getBoolType());
    ASSERT_NE(Signature, nullptr);
    const Name NameValue = Context.namePool().intern("Ready");
    const Function *First = Context.createFunction(NameValue, *Signature);
    const Function *Second = Context.createFunction(NameValue, *Signature);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_EQ(First->callingConvention(), CallingConvention::C);
    EXPECT_EQ(First->languageLinkage(), LanguageLinkage::Ink);
    EXPECT_FALSE(First->hasBody());
    EXPECT_FALSE(Second->hasBody());
    EXPECT_TRUE(First->blocks().empty());
    EXPECT_TRUE(Second->blocks().empty());
    EXPECT_EQ(First->entryBlock(), nullptr);
    EXPECT_EQ(Second->entryBlock(), nullptr);
    EXPECT_EQ(First->name(), Second->name());
    EXPECT_EQ(&First->type(), Signature);
    EXPECT_TRUE(Function::classof(First));
    EXPECT_FALSE(Function::classof(Signature));
    EXPECT_FALSE(Function::classof(nullptr));
    EXPECT_FALSE(Constant::classof(First));
    const CallInstruction *Call = Context.createCallInstruction(*First);
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->directCallee(), First);
    EXPECT_EQ(&Call->type(), &Context.getBoolType());
  }

  // Calling conventions and language linkages vary independently on a shared signature and survive body creation.
  TEST(SemanticFunctionTest, CallingConventionAndLanguageLinkageSurviveDefinition)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType());
    ASSERT_NE(Signature, nullptr);
    const Name NameValue = Context.namePool().intern("Target");
    for (CallingConvention Convention : {CallingConvention::C, CallingConvention::Fast, CallingConvention::Cold})
    {
      for (LanguageLinkage Linkage : {LanguageLinkage::Ink, LanguageLinkage::C})
      {
        Function *Target = Context.createFunction(NameValue, *Signature, {}, {}, Convention, Linkage);
        ASSERT_NE(Target, nullptr);
        EXPECT_FALSE(Target->hasBody());
        EXPECT_EQ(Target->callingConvention(), Convention);
        EXPECT_EQ(Target->languageLinkage(), Linkage);
        CallInstruction *Call = Context.createCallInstruction(*Target);
        ASSERT_NE(Call, nullptr);
        ASSERT_NE(Context.createFunctionBody(*Target), nullptr);
        EXPECT_TRUE(Target->hasBody());
        EXPECT_EQ(Target->callingConvention(), Convention);
        EXPECT_EQ(Target->languageLinkage(), Linkage);
        EXPECT_EQ(&Target->type(), Signature);
        EXPECT_EQ(Call->directCallee(), Target);
      }
    }
  }

  // Defining a previously referenced function preserves its identity and keeps the body stable through storage growth.
  TEST(SemanticFunctionTest, BodyPreservesFunctionReferencesAndOwnsOrderedValues)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType());
    ASSERT_NE(Signature, nullptr);
    const Name NameValue = Context.namePool().intern("Main");
    Function *Target = Context.createFunction(NameValue, *Signature);
    ASSERT_NE(Target, nullptr);
    CallInstruction *FirstCall = Context.createCallInstruction(*Target);
    CallInstruction *SecondCall = Context.createCallInstruction(*Target);
    ASSERT_NE(FirstCall, nullptr);
    ASSERT_NE(SecondCall, nullptr);
    EXPECT_FALSE(Target->hasBody());
    EXPECT_EQ(Target->entryBlock(), nullptr);

    BasicBlock *Entry = Context.createFunctionBody(*Target);
    ASSERT_NE(Entry, nullptr);
    EXPECT_TRUE(Target->hasBody());
    EXPECT_EQ(Target->entryBlock(), Entry);
    const Function &View = *Target;
    ASSERT_EQ(View.blocks().size(), 1U);
    EXPECT_EQ(View.blocks().front(), Entry);
    EXPECT_EQ(View.entryBlock(), Entry);
    EXPECT_EQ(View.entryBlock()->outer(), &View);
    EXPECT_EQ(&Entry->context(), &Context);
    EXPECT_EQ(&Entry->type(), &Context.getLabelType());
    EXPECT_TRUE(Entry->values().empty());
    ASSERT_TRUE(Context.appendValue(*Entry, *FirstCall));
    ASSERT_TRUE(Context.appendValue(*Entry, *SecondCall));

    for (unsigned Index = 0; Index < 64; ++Index)
    {
      Function *Next = Context.createFunction(NameValue, *Signature);
      ASSERT_NE(Next, nullptr);
      BasicBlock *NextEntry = Context.createFunctionBody(*Next);
      ASSERT_NE(NextEntry, nullptr);
      EXPECT_NE(NextEntry, Entry);
      EXPECT_EQ(NextEntry->outer(), Next);
      EXPECT_TRUE(NextEntry->values().empty());
    }

    EXPECT_EQ(Target->entryBlock(), Entry);
    EXPECT_EQ(Entry->outer(), Target);
    EXPECT_EQ(Target->outer(), nullptr);
    EXPECT_EQ(&Target->type(), Signature);
    ASSERT_EQ(Entry->values().size(), 2U);
    EXPECT_EQ(Entry->values()[0], FirstCall);
    EXPECT_EQ(Entry->values()[1], SecondCall);
    EXPECT_EQ(FirstCall->outer(), Entry);
    EXPECT_EQ(SecondCall->outer(), Entry);
    EXPECT_EQ(FirstCall->directCallee(), Target);
    EXPECT_EQ(SecondCall->directCallee(), Target);
  }

  // Appending blocks preserves entry identity, insertion order and values across the function block list's growth.
  TEST(SemanticFunctionTest, BlocksPreserveEntryAndOrderDuringGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType());
    ASSERT_NE(Signature, nullptr);
    Function *Target = Context.createFunction(Context.namePool().intern("Main"), *Signature);
    ASSERT_NE(Target, nullptr);
    BasicBlock *Entry = Context.createBasicBlock(*Target);
    ASSERT_NE(Entry, nullptr);
    EXPECT_TRUE(Target->hasBody());
    EXPECT_EQ(Target->entryBlock(), Entry);
    EXPECT_EQ(Context.createFunctionBody(*Target), nullptr);
    CallInstruction *EntryCall = Context.createCallInstruction(*Target);
    ASSERT_NE(EntryCall, nullptr);
    ASSERT_TRUE(Context.appendValue(*Entry, *EntryCall));
    std::vector<BasicBlock *> Expected = {Entry};
    for (unsigned Index = 0; Index < 64; ++Index)
    {
      BasicBlock *Next = Context.createBasicBlock(*Target);
      ASSERT_NE(Next, nullptr);
      EXPECT_NE(Next, Entry);
      EXPECT_EQ(Next->outer(), Target);
      EXPECT_EQ(&Next->context(), &Context);
      EXPECT_EQ(&Next->type(), &Context.getLabelType());
      EXPECT_TRUE(Next->values().empty());
      Expected.push_back(Next);
    }
    CallInstruction *LastCall = Context.createCallInstruction(*Target);
    ASSERT_NE(LastCall, nullptr);
    ASSERT_TRUE(Context.appendValue(*Expected.back(), *LastCall));

    const Function &View = *Target;
    EXPECT_EQ(View.blocks(), Expected);
    EXPECT_EQ(View.entryBlock(), Entry);
    EXPECT_EQ(Entry->outer(), Target);
    ASSERT_EQ(Entry->values().size(), 1U);
    EXPECT_EQ(Entry->values().front(), EntryCall);
    EXPECT_EQ(EntryCall->outer(), Entry);
    ASSERT_EQ(View.blocks().back()->values().size(), 1U);
    EXPECT_EQ(View.blocks().back()->values().front(), LastCall);
    EXPECT_EQ(LastCall->outer(), View.blocks().back());
    EXPECT_EQ(&View.type(), Signature);
  }

  // Foreign contexts cannot create or append blocks, and a repeated definition preserves existing blocks and contents.
  TEST(SemanticFunctionTest, ForeignAndRepeatedBodyCreationAreRejected)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType());
    ASSERT_NE(Signature, nullptr);
    Function *Target = Context.createFunction(Context.namePool().intern("Target"), *Signature);
    ASSERT_NE(Target, nullptr);
    EXPECT_EQ(Other.createFunctionBody(*Target), nullptr);
    EXPECT_EQ(Other.createBasicBlock(*Target), nullptr);
    EXPECT_FALSE(Target->hasBody());
    EXPECT_EQ(Target->entryBlock(), nullptr);
    EXPECT_TRUE(Target->blocks().empty());
    BasicBlock *Entry = Context.createFunctionBody(*Target);
    ASSERT_NE(Entry, nullptr);
    CallInstruction *Call = Context.createCallInstruction(*Target);
    ASSERT_NE(Call, nullptr);
    ASSERT_TRUE(Context.appendValue(*Entry, *Call));
    BasicBlock *Tail = Context.createBasicBlock(*Target);
    ASSERT_NE(Tail, nullptr);

    EXPECT_EQ(Context.createFunctionBody(*Target), nullptr);
    EXPECT_EQ(Other.createFunctionBody(*Target), nullptr);
    EXPECT_EQ(Other.createBasicBlock(*Target), nullptr);
    EXPECT_EQ(Target->entryBlock(), Entry);
    EXPECT_EQ(Entry->outer(), Target);
    ASSERT_EQ(Target->blocks().size(), 2U);
    EXPECT_EQ(Target->blocks()[0], Entry);
    EXPECT_EQ(Target->blocks()[1], Tail);
    EXPECT_EQ(Tail->outer(), Target);
    ASSERT_EQ(Entry->values().size(), 1U);
    EXPECT_EQ(Entry->values()[0], Call);
    EXPECT_EQ(Call->outer(), Entry);
  }

  // Every function-to-block edge participates in cycle checks, and moving a function retains all of its blocks.
  TEST(SemanticFunctionTest, BodyParentLinksPreventCyclesAndEntryReparenting)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType());
    ASSERT_NE(Signature, nullptr);
    Function *Root = Context.createFunction(Context.namePool().intern("Root"), *Signature);
    Function *Nested = Context.createFunction(Context.namePool().intern("Nested"), *Signature);
    ASSERT_NE(Root, nullptr);
    ASSERT_NE(Nested, nullptr);
    BasicBlock *RootEntry = Context.createFunctionBody(*Root);
    BasicBlock *NestedEntry = Context.createFunctionBody(*Nested);
    BasicBlock *RootTail = Context.createBasicBlock(*Root);
    BasicBlock *NestedTail = Context.createBasicBlock(*Nested);
    BasicBlock *Detached = Context.createBasicBlock();
    ASSERT_NE(RootEntry, nullptr);
    ASSERT_NE(NestedEntry, nullptr);
    ASSERT_NE(RootTail, nullptr);
    ASSERT_NE(NestedTail, nullptr);
    ASSERT_NE(Detached, nullptr);
    EXPECT_FALSE(Context.appendValue(*RootEntry, *Root));
    EXPECT_FALSE(Context.appendValue(*RootTail, *Root));
    EXPECT_FALSE(Context.appendValue(*Detached, *RootEntry));
    EXPECT_FALSE(Context.appendValue(*Detached, *RootTail));
    EXPECT_FALSE(Context.removeValue(*Detached, *RootEntry));
    ASSERT_TRUE(Context.appendValue(*RootEntry, *Nested));
    EXPECT_FALSE(Context.appendValue(*NestedEntry, *Root));
    EXPECT_FALSE(Context.appendValue(*NestedEntry, *RootEntry));
    EXPECT_FALSE(Context.appendValue(*NestedTail, *Root));
    EXPECT_FALSE(Context.appendValue(*NestedTail, *RootTail));
    EXPECT_EQ(RootEntry->outer(), Root);
    EXPECT_EQ(NestedEntry->outer(), Nested);
    EXPECT_EQ(RootTail->outer(), Root);
    EXPECT_EQ(NestedTail->outer(), Nested);
    EXPECT_EQ(Nested->outer(), RootEntry);
    EXPECT_TRUE(NestedEntry->values().empty());
    EXPECT_TRUE(Detached->values().empty());

    ASSERT_TRUE(Context.removeValue(*RootEntry, *Nested));
    EXPECT_EQ(Nested->outer(), nullptr);
    EXPECT_EQ(NestedEntry->outer(), Nested);
    EXPECT_EQ(Nested->entryBlock(), NestedEntry);
    ASSERT_TRUE(Context.appendValue(*Detached, *Nested));
    EXPECT_EQ(Nested->outer(), Detached);
    EXPECT_EQ(NestedEntry->outer(), Nested);
    EXPECT_EQ(NestedTail->outer(), Nested);
    ASSERT_EQ(Nested->blocks().size(), 2U);
    EXPECT_EQ(Nested->blocks()[0], NestedEntry);
    EXPECT_EQ(Nested->blocks()[1], NestedTail);
    EXPECT_TRUE(RootEntry->values().empty());
  }

  // Closed functions reject invalid names, foreign signatures and undefined calling-convention or language-linkage values.
  TEST(SemanticFunctionTest, InvalidNamesSignaturesAndFunctionMetadataAreRejected)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType());
    const FunctionType *Foreign = Other.getFunctionType(Other.getVoidType());
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(Foreign, nullptr);
    EXPECT_EQ(Context.createFunction(Name{}, *Signature), nullptr);
    EXPECT_EQ(Context.createFunction(Other.namePool().intern("Foreign"), *Signature), nullptr);
    const Name NameValue = Context.namePool().intern("Local");
    EXPECT_EQ(Context.createFunction(NameValue, *Foreign), nullptr);
    EXPECT_EQ(Context.createFunction(NameValue, *Signature, {}, {}, static_cast<CallingConvention>(255)), nullptr);
    EXPECT_EQ(Context.createFunction(NameValue, *Signature, {}, {}, CallingConvention::C, static_cast<LanguageLinkage>(255)), nullptr);
    EXPECT_NE(Context.createFunction(NameValue, *Signature), nullptr);
  }
} // namespace ink::semantic::test
