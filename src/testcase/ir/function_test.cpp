#include "ink/ir/ir_builder.h"
#include "ownership_test_support.h"

#include "ink/ir/function/function.h"
#include "ink/ir/context.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_base_of_v<Value, Function>);
  static_assert(!std::is_base_of_v<Decl, Function>);
  static_assert(!std::is_copy_constructible_v<Function>);
  static_assert(!std::is_move_constructible_v<Function>);
  static_assert(std::is_same_v<decltype(std::declval<Function &>().blocks()), const std::vector<std::unique_ptr<BasicBlock>> &>);

  // Same-named functions share a structural signature while keeping independent callable identities.
  TEST(IRFunctionTest, ClosedFunctionsHaveIndependentValueIdentity)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>());
    ASSERT_NE(Signature, nullptr);
    const Name NameValue = Context.namePool().intern("Ready");
    auto FirstOwner = Factory.createFunction(NameValue, *Signature);
    const Function *First = FirstOwner.get();
    auto SecondOwner = Factory.createFunction(NameValue, *Signature);
    const Function *Second = SecondOwner.get();
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_EQ(First->callingConvention(), CallingConvention::C);
    EXPECT_EQ(First->languageLinkage(), LanguageLinkage::Ink);
    EXPECT_EQ(First->binding(), FunctionBinding::Local);
    EXPECT_FALSE(First->isNativeImport());
    EXPECT_FALSE(First->isNativeExport());
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
    auto CallOwner = Factory.createDetachedCallInstruction(*First);
    const CallInstruction *Call = CallOwner.get();
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->directCallee(), First);
    EXPECT_EQ(&Call->type(), &Context.typePool().getType<TypeKind::Bool>());
  }

  // Calling conventions and language linkages vary independently on a shared signature and survive body creation.
  TEST(IRFunctionTest, CallingConventionAndLanguageLinkageSurviveDefinition)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    const Name NameValue = Context.namePool().intern("Target");
    for (CallingConvention Convention : {CallingConvention::C, CallingConvention::Fast, CallingConvention::Cold})
    {
      for (LanguageLinkage Linkage : {LanguageLinkage::Ink, LanguageLinkage::C})
      {
        auto TargetOwner = Factory.createFunction(NameValue, *Signature, {}, {}, Convention, Linkage);
        Function *Target = TargetOwner.get();
        ASSERT_NE(Target, nullptr);
        EXPECT_FALSE(Target->hasBody());
        EXPECT_EQ(Target->callingConvention(), Convention);
        EXPECT_EQ(Target->languageLinkage(), Linkage);
        auto CallOwner = Factory.createDetachedCallInstruction(*Target);
        CallInstruction *Call = CallOwner.get();
        ASSERT_NE(Call, nullptr);
        ASSERT_NE(Factory.createFunctionBody(*Target), nullptr);
        EXPECT_TRUE(Target->hasBody());
        EXPECT_EQ(Target->callingConvention(), Convention);
        EXPECT_EQ(Target->languageLinkage(), Linkage);
        EXPECT_EQ(&Target->type(), Signature);
        EXPECT_EQ(Call->directCallee(), Target);
      }
    }
  }

  // C ABI callbacks, imported declarations and private exports keep separate native direction and source visibility.
  TEST(IRFunctionTest, NativeBindingsAreIndependentOfVisibilityAndLocalCABI)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    for (FunctionBinding Binding : {FunctionBinding::Local, FunctionBinding::Import, FunctionBinding::Export})
    {
      auto Target = Factory.createFunction(Context.namePool().intern("Native"), *Signature, {}, {}, CallingConvention::C, LanguageLinkage::C, Binding);
      ASSERT_NE(Target, nullptr);
      EXPECT_EQ(Target->binding(), Binding);
      EXPECT_EQ(Target->isNativeImport(), Binding == FunctionBinding::Import);
      EXPECT_EQ(Target->isNativeExport(), Binding == FunctionBinding::Export);
      ASSERT_TRUE(Factory.setFunctionVisibility(*Target, VisibilityKind::Private));
      EXPECT_EQ(Target->visibility(), VisibilityKind::Private);
      EXPECT_EQ(Target->binding(), Binding);
      if (Binding == FunctionBinding::Import)
      {
        EXPECT_EQ(Factory.createFunctionBody(*Target), nullptr);
        EXPECT_EQ(Factory.createBasicBlock(*Target), nullptr);
        EXPECT_FALSE(Target->hasBody());
      }
      else
      {
        EXPECT_NE(Factory.createFunctionBody(*Target), nullptr);
        EXPECT_TRUE(Target->hasBody());
      }
    }
  }

  // Changing native direction invalidates cached code only on success and rejects bodies, foreign contexts and non-C ABIs.
  TEST(IRFunctionTest, BindingEditsValidateMetadataAndAdvanceRevision)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRContext Other(Compilation);
    IRBuilder Factory(Context);
    IRBuilder OtherFactory(Other);
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    const auto NameValue = Context.namePool().intern("Native");
    auto Target = Factory.createFunction(NameValue, *Signature, {}, {}, CallingConvention::C, LanguageLinkage::C);
    auto InkTarget = Factory.createFunction(NameValue, *Signature);
    auto FastTarget = Factory.createFunction(NameValue, *Signature, {}, {}, CallingConvention::Fast, LanguageLinkage::C);
    ASSERT_NE(Target, nullptr);
    ASSERT_NE(InkTarget, nullptr);
    ASSERT_NE(FastTarget, nullptr);
    std::uint64_t Previous = Context.revision();
    EXPECT_FALSE(OtherFactory.setFunctionBinding(*Target, FunctionBinding::Import));
    EXPECT_FALSE(Factory.setFunctionBinding(*InkTarget, FunctionBinding::Import));
    EXPECT_FALSE(Factory.setFunctionBinding(*FastTarget, FunctionBinding::Export));
    EXPECT_FALSE(Factory.setFunctionBinding(*Target, static_cast<FunctionBinding>(255)));
    EXPECT_EQ(Context.revision(), Previous);
    ASSERT_TRUE(Factory.setFunctionBinding(*Target, FunctionBinding::Import));
    EXPECT_GT(Context.revision(), Previous);
    EXPECT_TRUE(Target->isNativeImport());
    Previous = Context.revision();
    EXPECT_TRUE(Factory.setFunctionBinding(*Target, FunctionBinding::Import));
    EXPECT_EQ(Context.revision(), Previous);
    ASSERT_TRUE(Factory.setFunctionBinding(*Target, FunctionBinding::Export));
    EXPECT_GT(Context.revision(), Previous);
    ASSERT_NE(Factory.createFunctionBody(*Target), nullptr);
    Previous = Context.revision();
    EXPECT_FALSE(Factory.setFunctionBinding(*Target, FunctionBinding::Import));
    EXPECT_EQ(Context.revision(), Previous);
    EXPECT_TRUE(Target->isNativeExport());
    EXPECT_TRUE(Target->hasBody());
    EXPECT_TRUE(Factory.setFunctionBinding(*Target, FunctionBinding::Local));
    EXPECT_EQ(Target->binding(), FunctionBinding::Local);
    EXPECT_TRUE(Target->hasBody());
  }

  // Defining a previously referenced function preserves its identity and keeps the body stable through storage growth.
  TEST(IRFunctionTest, BodyPreservesFunctionReferencesAndOwnsOrderedValues)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    const Name NameValue = Context.namePool().intern("Main");
    auto TargetOwner = Factory.createFunction(NameValue, *Signature);
    Function *Target = TargetOwner.get();
    ASSERT_NE(Target, nullptr);
    auto FirstCallOwner = Factory.createDetachedCallInstruction(*Target);
    CallInstruction *FirstCall = FirstCallOwner.get();
    auto SecondCallOwner = Factory.createDetachedCallInstruction(*Target);
    CallInstruction *SecondCall = SecondCallOwner.get();
    ASSERT_NE(FirstCall, nullptr);
    ASSERT_NE(SecondCall, nullptr);
    EXPECT_FALSE(Target->hasBody());
    EXPECT_EQ(Target->entryBlock(), nullptr);

    BasicBlock *Entry = Factory.createFunctionBody(*Target);
    ASSERT_NE(Entry, nullptr);
    EXPECT_TRUE(Target->hasBody());
    EXPECT_EQ(Target->entryBlock(), Entry);
    const Function &View = *Target;
    ASSERT_EQ(View.blocks().size(), 1U);
    EXPECT_EQ(View.blocks().front().get(), Entry);
    EXPECT_EQ(View.entryBlock(), Entry);
    EXPECT_EQ(View.entryBlock()->outer(), &View);
    EXPECT_EQ(&Entry->context(), &Context);
    EXPECT_EQ(&Entry->type(), &Context.typePool().getType<TypeKind::Label>());
    EXPECT_TRUE(Entry->values().empty());
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(FirstCallOwner)));
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(SecondCallOwner)));

    auto Container = Factory.createBasicBlock();
    ASSERT_TRUE(Factory.appendValue(*Container, std::move(TargetOwner)));
    for (unsigned Index = 0; Index < 64; ++Index)
    {
      auto NextOwner = Factory.createFunction(NameValue, *Signature);
      Function *Next = NextOwner.get();
      ASSERT_NE(Next, nullptr);
      BasicBlock *NextEntry = Factory.createFunctionBody(*Next);
      ASSERT_NE(NextEntry, nullptr);
      EXPECT_NE(NextEntry, Entry);
      EXPECT_EQ(NextEntry->outer(), Next);
      EXPECT_TRUE(NextEntry->values().empty());
      ASSERT_TRUE(Factory.appendValue(*Container, std::move(NextOwner)));
    }

    EXPECT_EQ(Target->entryBlock(), Entry);
    EXPECT_EQ(Entry->outer(), Target);
    EXPECT_EQ(Target->outer(), Container.get());
    EXPECT_EQ(&Target->type(), Signature);
    ASSERT_EQ(Entry->values().size(), 2U);
    EXPECT_EQ(Entry->values()[0].get(), FirstCall);
    EXPECT_EQ(Entry->values()[1].get(), SecondCall);
    EXPECT_EQ(FirstCall->outer(), Entry);
    EXPECT_EQ(SecondCall->outer(), Entry);
    EXPECT_EQ(FirstCall->directCallee(), Target);
    EXPECT_EQ(SecondCall->directCallee(), Target);
  }

  // Appending blocks preserves entry identity, insertion order and values across the function block list's growth.
  TEST(IRFunctionTest, BlocksPreserveEntryAndOrderDuringGrowth)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    auto TargetOwner = Factory.createFunction(Context.namePool().intern("Main"), *Signature);
    Function *Target = TargetOwner.get();
    ASSERT_NE(Target, nullptr);
    BasicBlock *Entry = Factory.createBasicBlock(*Target);
    ASSERT_NE(Entry, nullptr);
    EXPECT_TRUE(Target->hasBody());
    EXPECT_EQ(Target->entryBlock(), Entry);
    EXPECT_EQ(Factory.createFunctionBody(*Target), nullptr);
    auto EntryCallOwner = Factory.createDetachedCallInstruction(*Target);
    CallInstruction *EntryCall = EntryCallOwner.get();
    ASSERT_NE(EntryCall, nullptr);
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(EntryCallOwner)));
    std::vector<BasicBlock *> Expected = {Entry};
    for (unsigned Index = 0; Index < 64; ++Index)
    {
      BasicBlock *Next = Factory.createBasicBlock(*Target);
      ASSERT_NE(Next, nullptr);
      EXPECT_NE(Next, Entry);
      EXPECT_EQ(Next->outer(), Target);
      EXPECT_EQ(&Next->context(), &Context);
      EXPECT_EQ(&Next->type(), &Context.typePool().getType<TypeKind::Label>());
      EXPECT_TRUE(Next->values().empty());
      Expected.push_back(Next);
    }
    auto LastCallOwner = Factory.createDetachedCallInstruction(*Target);
    CallInstruction *LastCall = LastCallOwner.get();
    ASSERT_NE(LastCall, nullptr);
    ASSERT_TRUE(Factory.appendValue(*Expected.back(), std::move(LastCallOwner)));

    const Function &View = *Target;
    EXPECT_EQ(borrowedPointers(View.blocks()), Expected);
    EXPECT_EQ(View.entryBlock(), Entry);
    EXPECT_EQ(Entry->outer(), Target);
    ASSERT_EQ(Entry->values().size(), 1U);
    EXPECT_EQ(Entry->values().front().get(), EntryCall);
    EXPECT_EQ(EntryCall->outer(), Entry);
    ASSERT_EQ(View.blocks().back()->values().size(), 1U);
    EXPECT_EQ(View.blocks().back()->values().front().get(), LastCall);
    EXPECT_EQ(LastCall->outer(), View.blocks().back().get());
    EXPECT_EQ(&View.type(), Signature);
  }

  // Foreign contexts cannot create or append blocks, and a repeated definition preserves existing blocks and contents.
  TEST(IRFunctionTest, ForeignAndRepeatedBodyCreationAreRejected)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    IRContext Other(Compilation);
    IRBuilder OtherFactory(Other);
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    auto TargetOwner = Factory.createFunction(Context.namePool().intern("Target"), *Signature);
    Function *Target = TargetOwner.get();
    ASSERT_NE(Target, nullptr);
    EXPECT_EQ(OtherFactory.createFunctionBody(*Target), nullptr);
    EXPECT_EQ(OtherFactory.createBasicBlock(*Target), nullptr);
    EXPECT_FALSE(Target->hasBody());
    EXPECT_EQ(Target->entryBlock(), nullptr);
    EXPECT_TRUE(Target->blocks().empty());
    BasicBlock *Entry = Factory.createFunctionBody(*Target);
    ASSERT_NE(Entry, nullptr);
    auto CallOwner = Factory.createDetachedCallInstruction(*Target);
    CallInstruction *Call = CallOwner.get();
    ASSERT_NE(Call, nullptr);
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(CallOwner)));
    BasicBlock *Tail = Factory.createBasicBlock(*Target);
    ASSERT_NE(Tail, nullptr);

    EXPECT_EQ(Factory.createFunctionBody(*Target), nullptr);
    EXPECT_EQ(OtherFactory.createFunctionBody(*Target), nullptr);
    EXPECT_EQ(OtherFactory.createBasicBlock(*Target), nullptr);
    EXPECT_EQ(Target->entryBlock(), Entry);
    EXPECT_EQ(Entry->outer(), Target);
    ASSERT_EQ(Target->blocks().size(), 2U);
    EXPECT_EQ(Target->blocks()[0].get(), Entry);
    EXPECT_EQ(Target->blocks()[1].get(), Tail);
    EXPECT_EQ(Tail->outer(), Target);
    ASSERT_EQ(Entry->values().size(), 1U);
    EXPECT_EQ(Entry->values()[0].get(), Call);
    EXPECT_EQ(Call->outer(), Entry);
  }

  // Every function-to-block edge participates in cycle checks, and moving a function retains all of its blocks.
  TEST(IRFunctionTest, BodyParentLinksPreventCyclesAndEntryReparenting)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    auto RootOwner = Factory.createFunction(Context.namePool().intern("Root"), *Signature);
    Function *Root = RootOwner.get();
    auto NestedOwner = Factory.createFunction(Context.namePool().intern("Nested"), *Signature);
    Function *Nested = NestedOwner.get();
    ASSERT_NE(Root, nullptr);
    ASSERT_NE(Nested, nullptr);
    BasicBlock *RootEntry = Factory.createFunctionBody(*Root);
    BasicBlock *NestedEntry = Factory.createFunctionBody(*Nested);
    BasicBlock *RootTail = Factory.createBasicBlock(*Root);
    BasicBlock *NestedTail = Factory.createBasicBlock(*Nested);
    auto DetachedOwner = Factory.createBasicBlock();
    BasicBlock *Detached = DetachedOwner.get();
    ASSERT_NE(RootEntry, nullptr);
    ASSERT_NE(NestedEntry, nullptr);
    ASSERT_NE(RootTail, nullptr);
    ASSERT_NE(NestedTail, nullptr);
    ASSERT_NE(Detached, nullptr);
    EXPECT_FALSE(Factory.appendValue(*RootEntry, std::move(RootOwner)));
    EXPECT_FALSE(Factory.appendValue(*RootTail, std::move(RootOwner)));
    EXPECT_EQ(Factory.removeValue(*Detached, *RootTail), nullptr);
    EXPECT_FALSE(Factory.removeValue(*Detached, *RootEntry));
    ASSERT_TRUE(Factory.appendValue(*RootEntry, std::move(NestedOwner)));
    EXPECT_FALSE(Factory.appendValue(*NestedEntry, std::move(RootOwner)));
    EXPECT_EQ(Factory.removeValue(*NestedEntry, *RootEntry), nullptr);
    EXPECT_FALSE(Factory.appendValue(*NestedTail, std::move(RootOwner)));
    EXPECT_EQ(Factory.removeValue(*NestedTail, *RootTail), nullptr);
    EXPECT_EQ(RootEntry->outer(), Root);
    EXPECT_EQ(NestedEntry->outer(), Nested);
    EXPECT_EQ(RootTail->outer(), Root);
    EXPECT_EQ(NestedTail->outer(), Nested);
    EXPECT_EQ(Nested->outer(), RootEntry);
    EXPECT_TRUE(NestedEntry->values().empty());
    EXPECT_TRUE(Detached->values().empty());

    auto RemovedNestedOwner = Factory.removeValue(*RootEntry, *Nested);
    ASSERT_EQ(RemovedNestedOwner.get(), Nested);
    EXPECT_EQ(Nested->outer(), nullptr);
    EXPECT_EQ(NestedEntry->outer(), Nested);
    EXPECT_EQ(Nested->entryBlock(), NestedEntry);
    ASSERT_TRUE(Factory.appendValue(*Detached, std::move(RemovedNestedOwner)));
    EXPECT_EQ(Nested->outer(), Detached);
    EXPECT_EQ(NestedEntry->outer(), Nested);
    EXPECT_EQ(NestedTail->outer(), Nested);
    ASSERT_EQ(Nested->blocks().size(), 2U);
    EXPECT_EQ(Nested->blocks()[0].get(), NestedEntry);
    EXPECT_EQ(Nested->blocks()[1].get(), NestedTail);
    EXPECT_TRUE(RootEntry->values().empty());
  }

  // Closed functions reject invalid names, foreign signatures and undefined calling-convention or language-linkage values.
  TEST(IRFunctionTest, InvalidNamesSignaturesAndFunctionMetadataAreRejected)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    IRContext Other(Compilation);
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    const FunctionType *Foreign = Other.typePool().getType<TypeKind::Function>(Other.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(Foreign, nullptr);
    EXPECT_EQ(Factory.createFunction(Name{}, *Signature), nullptr);
    EXPECT_EQ(Factory.createFunction(Other.namePool().intern("Foreign"), *Signature), nullptr);
    const Name NameValue = Context.namePool().intern("Local");
    EXPECT_EQ(Factory.createFunction(NameValue, *Foreign), nullptr);
    EXPECT_EQ(Factory.createFunction(NameValue, *Signature, {}, {}, static_cast<CallingConvention>(255)), nullptr);
    EXPECT_EQ(Factory.createFunction(NameValue, *Signature, {}, {}, CallingConvention::C, static_cast<LanguageLinkage>(255)), nullptr);
    EXPECT_EQ(Factory.createFunction(NameValue, *Signature, {}, {}, CallingConvention::C, LanguageLinkage::C, static_cast<FunctionBinding>(255)), nullptr);
    EXPECT_EQ(Factory.createFunction(NameValue, *Signature, {}, {}, CallingConvention::C, LanguageLinkage::Ink, FunctionBinding::Import), nullptr);
    EXPECT_EQ(Factory.createFunction(NameValue, *Signature, {}, {}, CallingConvention::Cold, LanguageLinkage::C, FunctionBinding::Export), nullptr);
    EXPECT_NE(Factory.createFunction(NameValue, *Signature), nullptr);
  }
} // namespace ink::ir::test
