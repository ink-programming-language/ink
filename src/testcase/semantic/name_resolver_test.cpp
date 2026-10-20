#include "ink/core/name/name_pool.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"
#include "ink/semantic/name_resolve/scope_guard.h"

#include <gtest/gtest.h>

#include <span>
#include <type_traits>
#include <utility>

namespace ink::semantic::test
{
  using namespace ink::ir;
  using core::Name;

  static_assert(std::is_same_v<decltype(std::declval<const Binding &>().targets()), std::span<Value *const>>);
  static_assert(!std::is_copy_constructible_v<NameResolver>);
  static_assert(!std::is_move_constructible_v<NameResolver>);
  static_assert(!std::is_copy_constructible_v<Scope>);
  static_assert(!std::is_move_constructible_v<Scope>);
  static_assert(!std::is_copy_constructible_v<ScopeGuard>);
  static_assert(!std::is_move_constructible_v<ScopeGuard>);

  // Explicit object accessors preserve constness while borrowing lvalue references even from rvalue resolvers.
  static_assert(std::is_same_v<decltype(std::declval<NameResolver &>().scopeStore()), ScopeStore &>);
  static_assert(std::is_same_v<decltype(std::declval<const NameResolver &>().scopeStore()), const ScopeStore &>);
  static_assert(std::is_same_v<decltype(std::declval<NameResolver &&>().scopeStore()), ScopeStore &>);
  static_assert(std::is_same_v<decltype(std::declval<const NameResolver &&>().scopeStore()), const ScopeStore &>);
  static_assert(std::is_same_v<decltype(std::declval<NameResolver &>().rootScope()), Scope &>);
  static_assert(std::is_same_v<decltype(std::declval<const NameResolver &>().rootScope()), const Scope &>);
  static_assert(std::is_same_v<decltype(std::declval<NameResolver &&>().rootScope()), Scope &>);
  static_assert(std::is_same_v<decltype(std::declval<const NameResolver &&>().rootScope()), const Scope &>);
  static_assert(std::is_same_v<decltype(std::declval<NameResolver &>().currentScope()), Scope &>);
  static_assert(std::is_same_v<decltype(std::declval<const NameResolver &>().currentScope()), const Scope &>);
  static_assert(std::is_same_v<decltype(std::declval<NameResolver &&>().currentScope()), Scope &>);
  static_assert(std::is_same_v<decltype(std::declval<const NameResolver &&>().currentScope()), const Scope &>);

  // Scope and guard accessors propagate constness through borrowed references and nullable pointers.
  static_assert(std::is_same_v<decltype(std::declval<Scope &>().scopeStore()), ScopeStore &>);
  static_assert(std::is_same_v<decltype(std::declval<const Scope &>().scopeStore()), const ScopeStore &>);
  static_assert(std::is_same_v<decltype(std::declval<Scope &&>().scopeStore()), ScopeStore &>);
  static_assert(std::is_same_v<decltype(std::declval<const Scope &&>().scopeStore()), const ScopeStore &>);
  static_assert(std::is_same_v<decltype(std::declval<Scope &>().parent()), Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<const Scope &>().parent()), const Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<Scope &&>().parent()), Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<const Scope &&>().parent()), const Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<ScopeGuard &>().scope()), Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<const ScopeGuard &>().scope()), const Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<ScopeGuard &&>().scope()), Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<const ScopeGuard &&>().scope()), const Scope *>);

  // Lookup chooses the nearest lexical binding, isolates sibling locals, and preserves exited scopes.
  TEST(SemanticNameResolverTest, ResolvesNearestBindingAndIsolatesSiblings)
  {
    IRContext Context(target::TargetContext::native());
    Module ModuleValue(Context);
    auto *Outer = ModuleValue.createGlobalVariable(*Context.typePool().getBoolType());
    auto *Inner = ModuleValue.createGlobalVariable(*Context.typePool().getBoolType());
    ASSERT_NE(Outer, nullptr);
    ASSERT_NE(Inner, nullptr);
    ScopeStore Store;
    NameResolver Resolver(Store);
    const Name X = core::NamePool::instance().intern("X");
    const Name Local = core::NamePool::instance().intern("Local");
    Scope &Root = Resolver.rootScope();
    EXPECT_EQ(&Resolver.currentScope(), &Root);
    EXPECT_EQ(Root.parent(), nullptr);
    EXPECT_FALSE(Resolver.exitScope());
    ASSERT_EQ(Resolver.bind(X, *Outer), BindResult::Inserted);
    const Binding *OuterBinding = Resolver.lookup(X);
    ASSERT_NE(OuterBinding, nullptr);
    EXPECT_EQ(OuterBinding->name(), X);
    EXPECT_FALSE(OuterBinding->isOverloadSet());
    Scope &Child = Resolver.enterScope();
    EXPECT_EQ(Child.parent(), &Root);
    EXPECT_EQ(Resolver.lookup(X), OuterBinding);
    EXPECT_EQ(Resolver.lookupLocal(X), nullptr);
    ASSERT_EQ(Resolver.bind(X, *Inner), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Local, *Inner), BindResult::Inserted);
    const Binding *ChildBinding = Resolver.lookup(X);
    ASSERT_NE(ChildBinding, nullptr);
    ASSERT_EQ(ChildBinding->targets().size(), 1U);
    EXPECT_EQ(ChildBinding->targets()[0], static_cast<Value *>(Inner));
    Scope &Grandchild = Resolver.enterScope();
    EXPECT_EQ(Grandchild.parent(), &Child);
    EXPECT_EQ(Resolver.lookup(X), ChildBinding);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(&Resolver.currentScope(), &Child);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookup(X), OuterBinding);
    EXPECT_EQ(Resolver.lookup(Local), nullptr);
    Scope &Sibling = Resolver.enterScope();
    EXPECT_NE(&Sibling, &Child);
    EXPECT_EQ(Sibling.parent(), &Root);
    EXPECT_EQ(Resolver.lookup(X), OuterBinding);
    EXPECT_EQ(Resolver.lookup(Local), nullptr);
    NameResolver Resumed(Grandchild);
    EXPECT_EQ(Resumed.lookup(X), ChildBinding);
    EXPECT_EQ(Resumed.lookupLocal(X), nullptr);
  }

  // Only explicit overload bindings accumulate functions, and mixing binding policies never changes an existing binding.
  TEST(SemanticNameResolverTest, AccumulatesOnlyExplicitOverloads)
  {
    IRContext Context(target::TargetContext::native());
    Module ModuleValue(Context);
    const auto *Signature = Context.typePool().getFunctionType(Context.typePool().getVoidType());
    ASSERT_NE(Signature, nullptr);
    auto *First = ModuleValue.createGlobalFunction(*Signature);
    auto *Second = ModuleValue.createGlobalFunction(*Signature);
    auto *Variable = ModuleValue.createGlobalVariable(*Context.typePool().getBoolType());
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Variable, nullptr);
    ScopeStore Store;
    NameResolver Resolver(Store);
    const Name F = core::NamePool::instance().intern("F");
    const Name Plain = core::NamePool::instance().intern("Plain");
    ASSERT_EQ(Resolver.bindOverload(F, *First), BindResult::Inserted);
    const Binding *Overloads = Resolver.lookup(F);
    ASSERT_NE(Overloads, nullptr);
    EXPECT_TRUE(Overloads->isOverloadSet());
    EXPECT_EQ(Resolver.bindOverload(F, *First), BindResult::AlreadyBound);
    EXPECT_EQ(Resolver.bind(F, *First), BindResult::Conflict);
    EXPECT_EQ(Resolver.bind(F, *Variable), BindResult::Conflict);
    ASSERT_EQ(Resolver.bindOverload(F, *Second), BindResult::Inserted);
    EXPECT_EQ(Resolver.lookup(F), Overloads);
    ASSERT_EQ(Overloads->targets().size(), 2U);
    EXPECT_EQ(Overloads->targets()[0], static_cast<Value *>(First));
    EXPECT_EQ(Overloads->targets()[1], static_cast<Value *>(Second));
    ASSERT_EQ(Resolver.bind(Plain, *First), BindResult::Inserted);
    EXPECT_EQ(Resolver.bind(Plain, *First), BindResult::AlreadyBound);
    EXPECT_EQ(Resolver.bind(Plain, *Second), BindResult::Conflict);
    EXPECT_EQ(Resolver.bindOverload(Plain, *First), BindResult::Conflict);
    EXPECT_EQ(Resolver.bindOverload(Plain, *Second), BindResult::Conflict);
    const Binding *PlainBinding = Resolver.lookup(Plain);
    ASSERT_NE(PlainBinding, nullptr);
    EXPECT_FALSE(PlainBinding->isOverloadSet());
    ASSERT_EQ(PlainBinding->targets().size(), 1U);
    EXPECT_EQ(PlainBinding->targets()[0], static_cast<Value *>(First));
  }

  // An inner overload set hides the entire outer set instead of merging candidates across lexical scopes.
  TEST(SemanticNameResolverTest, HidesOuterOverloadSetAsAWhole)
  {
    IRContext Context(target::TargetContext::native());
    Module ModuleValue(Context);
    const auto *Signature = Context.typePool().getFunctionType(Context.typePool().getVoidType());
    ASSERT_NE(Signature, nullptr);
    auto *First = ModuleValue.createGlobalFunction(*Signature);
    auto *Second = ModuleValue.createGlobalFunction(*Signature);
    auto *Inner = ModuleValue.createGlobalFunction(*Signature);
    auto *Variable = ModuleValue.createGlobalVariable(*Context.typePool().getBoolType());
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Inner, nullptr);
    ASSERT_NE(Variable, nullptr);
    ScopeStore Store;
    NameResolver Resolver(Store);
    const Name F = core::NamePool::instance().intern("F");
    ASSERT_EQ(Resolver.bindOverload(F, *First), BindResult::Inserted);
    ASSERT_EQ(Resolver.bindOverload(F, *Second), BindResult::Inserted);
    const Binding *OuterBinding = Resolver.lookup(F);
    Resolver.enterScope();
    ASSERT_EQ(Resolver.bindOverload(F, *Inner), BindResult::Inserted);
    const Binding *InnerBinding = Resolver.lookup(F);
    ASSERT_NE(InnerBinding, nullptr);
    ASSERT_EQ(InnerBinding->targets().size(), 1U);
    EXPECT_EQ(InnerBinding->targets()[0], static_cast<Value *>(Inner));
    Resolver.enterScope();
    ASSERT_EQ(Resolver.bind(F, *Variable), BindResult::Inserted);
    ASSERT_NE(Resolver.lookup(F), nullptr);
    EXPECT_FALSE(Resolver.lookup(F)->isOverloadSet());
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookup(F), InnerBinding);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookup(F), OuterBinding);
    ASSERT_NE(OuterBinding, nullptr);
    EXPECT_EQ(OuterBinding->targets().size(), 2U);
  }

  // Invalid names and conflicting values fail without replacing bindings or mutating the process-wide name pool.
  TEST(SemanticNameResolverTest, RejectsInvalidNamesAndConflictingValues)
  {
    IRContext Context(target::TargetContext::native());
    Module ModuleValue(Context);
    auto *First = ModuleValue.createGlobalVariable(*Context.typePool().getBoolType());
    auto *Second = ModuleValue.createGlobalVariable(*Context.typePool().getBoolType());
    const auto *Signature = Context.typePool().getFunctionType(Context.typePool().getVoidType());
    ASSERT_NE(Signature, nullptr);
    auto *FunctionValue = ModuleValue.createGlobalFunction(*Signature);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(FunctionValue, nullptr);
    ScopeStore Store;
    NameResolver Resolver(Store);
    const Name X = core::NamePool::instance().intern("X");
    const Name Missing = core::NamePool::instance().intern("Missing");
    const std::size_t NameCount = core::NamePool::instance().size();
    EXPECT_EQ(Resolver.bind(Name{}, *First), BindResult::InvalidName);
    EXPECT_EQ(Resolver.bind(Name{}, ModuleValue), BindResult::InvalidName);
    EXPECT_EQ(Resolver.bindOverload(Name{}, *FunctionValue), BindResult::InvalidName);
    EXPECT_EQ(Resolver.lookup(Name{}), nullptr);
    EXPECT_EQ(Resolver.lookupLocal(Name{}), nullptr);
    EXPECT_EQ(Resolver.lookup(Missing), nullptr);
    ASSERT_EQ(Resolver.bind(X, *First), BindResult::Inserted);
    const Binding *Saved = Resolver.lookup(X);
    EXPECT_EQ(Resolver.bind(X, *First), BindResult::AlreadyBound);
    EXPECT_EQ(Resolver.bind(X, *Second), BindResult::Conflict);
    EXPECT_EQ(Resolver.bind(X, ModuleValue), BindResult::Conflict);
    EXPECT_EQ(Resolver.lookup(X), Saved);
    ASSERT_NE(Saved, nullptr);
    ASSERT_EQ(Saved->targets().size(), 1U);
    EXPECT_EQ(Saved->targets()[0], static_cast<Value *>(First));
    EXPECT_EQ(core::NamePool::instance().size(), NameCount);
  }

  // Module and base references share binding identity and member scopes, and member lookup never searches lexical parents.
  TEST(SemanticNameResolverTest, ResolvesModuleAndValueMembersWithoutParentFallback)
  {
    IRContext Context(target::TargetContext::native());
    Module Owner(Context);
    Module Unregistered(Context);
    Value &OwnerValue = Owner;
    auto *ValueOwner = Owner.createGlobalVariable(*Context.typePool().getBoolType());
    auto *Member = Owner.createGlobalVariable(*Context.typePool().getBoolType());
    ASSERT_NE(ValueOwner, nullptr);
    ASSERT_NE(Member, nullptr);
    ScopeStore Store;
    NameResolver Resolver(Store);
    const Name ModuleName = core::NamePool::instance().intern("Owner");
    const Name Alias = core::NamePool::instance().intern("Alias");
    const Name Outer = core::NamePool::instance().intern("Outer");
    const Name Field = core::NamePool::instance().intern("Field");
    ASSERT_EQ(Resolver.bind(ModuleName, Owner), BindResult::Inserted);
    const Binding *ModuleBinding = Resolver.lookupLocal(ModuleName);
    ASSERT_NE(ModuleBinding, nullptr);
    ASSERT_EQ(Resolver.bind(Alias, OwnerValue), BindResult::Inserted);
    EXPECT_EQ(Resolver.bind(ModuleName, Owner), BindResult::AlreadyBound);
    EXPECT_EQ(Resolver.bind(ModuleName, OwnerValue), BindResult::AlreadyBound);
    EXPECT_EQ(Resolver.lookupLocal(ModuleName), ModuleBinding);
    ASSERT_EQ(ModuleBinding->targets().size(), 1U);
    EXPECT_EQ(ModuleBinding->targets()[0], &OwnerValue);
    EXPECT_EQ(Resolver.bind(ModuleName, Unregistered), BindResult::Conflict);
    ASSERT_NE(Resolver.lookup(ModuleName), nullptr);
    EXPECT_EQ(Resolver.lookup(ModuleName)->targets()[0], static_cast<Value *>(&Owner));
    EXPECT_EQ(Resolver.lookup(Alias)->targets()[0], static_cast<Value *>(&Owner));
    ASSERT_EQ(Resolver.bind(Outer, *Member), BindResult::Inserted);
    Scope *Members = Resolver.enterScope(Owner);
    ASSERT_NE(Members, nullptr);
    EXPECT_EQ(Resolver.enterScope(OwnerValue), nullptr);
    EXPECT_EQ(&Resolver.currentScope(), Members);
    EXPECT_EQ(Store.memberScope(OwnerValue), Members);
    EXPECT_EQ(Store.memberScope(Owner), Members);
    ASSERT_TRUE(Members->owner().has_value());
    EXPECT_EQ(*Members->owner(), static_cast<Value *>(&Owner));
    ASSERT_EQ(Resolver.bind(Field, *Member), BindResult::Inserted);
    const Binding *ModuleField = Resolver.lookupLocal(Field);
    EXPECT_NE(Resolver.lookup(Outer), nullptr);
    EXPECT_EQ(Resolver.lookupMember(Owner, Outer), nullptr);
    EXPECT_EQ(Resolver.lookupMember(Owner, Name{}), nullptr);
    EXPECT_EQ(Resolver.lookupMember(Unregistered, Field), nullptr);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookupMember(Owner, Field), ModuleField);
    EXPECT_EQ(Resolver.lookupMember(OwnerValue, Field), ModuleField);
    EXPECT_EQ(Resolver.lookupMember(OwnerValue, Outer), nullptr);
    Scope *ValueMembers = Resolver.enterScope(*ValueOwner);
    ASSERT_NE(ValueMembers, nullptr);
    ASSERT_TRUE(ValueMembers->owner().has_value());
    EXPECT_EQ(*ValueMembers->owner(), static_cast<Value *>(ValueOwner));
    ASSERT_EQ(Resolver.bind(Field, *Member), BindResult::Inserted);
    EXPECT_EQ(Resolver.lookupMember(*ValueOwner, Field), Resolver.lookupLocal(Field));
    EXPECT_EQ(Resolver.lookupMember(*ValueOwner, Outer), nullptr);
    EXPECT_EQ(Resolver.lookupMember(*Member, Field), nullptr);
  }

  // Resolvers sharing a context keep independent cursors, and a later resolver can resume a persisted scope.
  TEST(SemanticNameResolverTest, ContextSharesStorageWithoutSharingResolverCursor)
  {
    core::SourceManager Sources;
    core::DiagnosticEngine Diagnostics;
    SemanticContext Context(Sources, Diagnostics, target::TargetContext::native());
    Module ModuleValue(Context.irContext());
    auto *Target = ModuleValue.createGlobalVariable(*Context.typePool().getBoolType());
    ASSERT_NE(Target, nullptr);
    const Name X = core::NamePool::instance().intern("X");
    const auto CheckResolvers = [&]()
    {
      Scope *Saved = nullptr;
      {
        NameResolver First(Context);
        NameResolver Second(Context);
        EXPECT_EQ(&First.scopeStore(), &Context.scopeStore());
        EXPECT_EQ(&First.rootScope(), &Second.rootScope());
        Saved = &First.enterScope();
        EXPECT_EQ(&Second.currentScope(), &Second.rootScope());
        ASSERT_EQ(First.bind(X, *Target), BindResult::Inserted);
        EXPECT_EQ(Second.lookup(X), nullptr);
        ASSERT_TRUE(Second.enterScope(*Saved));
        EXPECT_EQ(Second.lookupLocal(X), First.lookupLocal(X));
        First.enterScope();
        EXPECT_EQ(&Second.currentScope(), Saved);
      }
      {
        ASSERT_NE(Saved, nullptr);
        NameResolver Resumed(*Saved);
        EXPECT_EQ(&Resumed.scopeStore(), &Context.scopeStore());
        ASSERT_NE(Resumed.lookupLocal(X), nullptr);
        EXPECT_EQ(Resumed.lookupLocal(X)->targets()[0], static_cast<Value *>(Target));
      }
    };
    CheckResolvers();
    Context.scopeStore().removeTarget(*Target);
  }

  // Nested guards restore the exact saved scope on early return despite additional manual scope changes.
  TEST(SemanticNameResolverTest, GuardsRestoreExactScopeOnEarlyReturn)
  {
    IRContext Context(target::TargetContext::native());
    Module ModuleValue(Context);
    auto *Target = ModuleValue.createGlobalVariable(*Context.typePool().getBoolType());
    ASSERT_NE(Target, nullptr);
    ScopeStore Store;
    NameResolver Resolver(Store);
    Scope &Original = Resolver.enterScope();
    const Name X = core::NamePool::instance().intern("X");
    Scope *SavedChild = nullptr;
    const auto Analyze = [&]()
    {
      ScopeGuard Child(Resolver);
      SavedChild = Child.scope();
      ASSERT_NE(SavedChild, nullptr);
      ASSERT_EQ(Resolver.bind(X, *Target), BindResult::Inserted);
      {
        ScopeGuard Nested(Resolver);
        ASSERT_NE(Nested.scope(), nullptr);
        EXPECT_EQ(Nested.scope()->parent(), SavedChild);
        Resolver.enterScope();
      }
      EXPECT_EQ(&Resolver.currentScope(), SavedChild);
      {
        ScopeGuard Nested(Resolver);
        Resolver.enterScope();
        return;
      }
    };
    Analyze();
    EXPECT_EQ(&Resolver.currentScope(), &Original);
    EXPECT_EQ(Resolver.lookup(X), nullptr);
    ASSERT_NE(SavedChild, nullptr);
    NameResolver Resumed(*SavedChild);
    ASSERT_NE(Resumed.lookupLocal(X), nullptr);
    EXPECT_EQ(Resumed.lookupLocal(X)->targets()[0], static_cast<Value *>(Target));
    {
      ScopeGuard Existing(Resolver, *SavedChild);
      EXPECT_EQ(Existing.scope(), SavedChild);
      EXPECT_EQ(&Resolver.currentScope(), SavedChild);
    }
    EXPECT_EQ(&Resolver.currentScope(), &Original);
  }

  // Module and base-reference guards share owner identity; failed entries preserve the cursor and do not undo later entries.
  TEST(SemanticNameResolverTest, FailedEntriesAndGuardsPreserveCursorChanges)
  {
    IRContext Context(target::TargetContext::native());
    Module Owner(Context);
    Value &OwnerValue = Owner;
    auto *ValueOwner = Owner.createGlobalVariable(*Context.typePool().getBoolType());
    ASSERT_NE(ValueOwner, nullptr);
    ScopeStore Store;
    ScopeStore Foreign;
    NameResolver Resolver(Store);
    Scope &Root = Resolver.rootScope();
    Scope *Members = nullptr;
    {
      ScopeGuard OwnerGuard(Resolver, Owner);
      Members = OwnerGuard.scope();
      ASSERT_NE(Members, nullptr);
      EXPECT_EQ(Resolver.enterScope(Owner), nullptr);
      EXPECT_EQ(Resolver.enterScope(OwnerValue), nullptr);
      EXPECT_EQ(&Resolver.currentScope(), Members);
      EXPECT_FALSE(Resolver.enterScope(Foreign.rootScope()));
      EXPECT_EQ(&Resolver.currentScope(), Members);
      Scope *Deeper = nullptr;
      {
        ScopeGuard Duplicate(Resolver, OwnerValue);
        EXPECT_EQ(Duplicate.scope(), nullptr);
        Deeper = &Resolver.enterScope();
      }
      EXPECT_EQ(&Resolver.currentScope(), Deeper);
      {
        ScopeGuard Failed(Resolver, Foreign.rootScope());
        EXPECT_EQ(Failed.scope(), nullptr);
        Deeper = &Resolver.enterScope();
      }
      EXPECT_EQ(&Resolver.currentScope(), Deeper);
    }
    EXPECT_EQ(&Resolver.currentScope(), &Root);
    EXPECT_EQ(Store.memberScope(Owner), Members);
    EXPECT_EQ(Store.memberScope(OwnerValue), Members);
    Store.removeTarget(OwnerValue);
    {
      ScopeGuard BaseOwnerGuard(Resolver, OwnerValue);
      ASSERT_NE(BaseOwnerGuard.scope(), nullptr);
      EXPECT_NE(BaseOwnerGuard.scope(), Members);
      EXPECT_EQ(Store.memberScope(Owner), BaseOwnerGuard.scope());
      EXPECT_EQ(Store.memberScope(OwnerValue), BaseOwnerGuard.scope());
      ScopeGuard Duplicate(Resolver, Owner);
      EXPECT_EQ(Duplicate.scope(), nullptr);
      EXPECT_EQ(&Resolver.currentScope(), BaseOwnerGuard.scope());
    }
    EXPECT_EQ(&Resolver.currentScope(), &Root);
    {
      ScopeGuard ValueGuard(Resolver, *ValueOwner);
      ASSERT_NE(ValueGuard.scope(), nullptr);
      EXPECT_EQ(Resolver.enterScope(*ValueOwner), nullptr);
      EXPECT_EQ(&Resolver.currentScope(), ValueGuard.scope());
    }
    EXPECT_EQ(&Resolver.currentScope(), &Root);
  }

  // Variable aliases share the target's storage properties, and conflicting targets leave an existing binding intact.
  TEST(SemanticNameResolverTest, PreservesStorageIdentityAndExclusiveRegistration)
  {
    IRContext Context(target::TargetContext::native());
    Module ModuleValue(Context);
    const auto *Integer = Context.typePool().getInt32Type();
    auto *Storage = ModuleValue.createGlobalVariable(*Integer, core::NAME_None, false);
    auto *OtherStorage = ModuleValue.createGlobalVariable(*Integer);
    auto *FunctionValue = ModuleValue.createGlobalFunction(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    ASSERT_NE(Storage, nullptr);
    ASSERT_NE(OtherStorage, nullptr);
    ASSERT_NE(FunctionValue, nullptr);
    ScopeStore Store;
    NameResolver Resolver(Store);
    const Name Variable = core::NamePool::instance().intern("VariableStorage");
    const Name Alias = core::NamePool::instance().intern("VariableStorageAlias");
    EXPECT_EQ(Resolver.bind(core::NAME_None, *Storage), BindResult::InvalidName);
    EXPECT_EQ(Resolver.lookupLocal(core::NAME_None), nullptr);
    ASSERT_EQ(Resolver.bind(Variable, *Storage), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Alias, *Storage), BindResult::Inserted);
    const Binding *Saved = Resolver.lookupLocal(Variable);
    const Binding *AliasBinding = Resolver.lookupLocal(Alias);
    ASSERT_NE(Saved, nullptr);
    ASSERT_NE(AliasBinding, nullptr);
    EXPECT_EQ(Saved->name(), Variable);
    EXPECT_FALSE(Saved->isOverloadSet());
    ASSERT_EQ(Saved->targets().size(), 1U);
    ASSERT_EQ(AliasBinding->targets().size(), 1U);
    EXPECT_EQ(Saved->targets()[0], Storage);
    EXPECT_EQ(AliasBinding->targets()[0], Storage);
    EXPECT_EQ(Saved->targets()[0]->type(), Context.typePool().getQualifiedType(Integer, false));
    EXPECT_TRUE(Saved->targets()[0]->isStorage());
    EXPECT_FALSE(Saved->targets()[0]->isMutable());
    EXPECT_FALSE(Saved->targets()[0]->isInitialized());
    ASSERT_TRUE(Storage->setInitializer(Context.constantPool().getSignedIntegerConstant(Integer, 1)));
    EXPECT_TRUE(Saved->targets()[0]->isInitialized());
    EXPECT_TRUE(AliasBinding->targets()[0]->isInitialized());
    EXPECT_EQ(Resolver.bind(Variable, *Storage), BindResult::AlreadyBound);
    EXPECT_EQ(Resolver.bind(Variable, *OtherStorage), BindResult::Conflict);
    EXPECT_EQ(Resolver.bindOverload(Variable, *FunctionValue), BindResult::Conflict);
    EXPECT_EQ(Resolver.lookupLocal(Variable), Saved);
    EXPECT_EQ(Saved->targets()[0], Storage);
    const Name Overloaded = core::NamePool::instance().intern("OverloadStorageConflict");
    ASSERT_EQ(Resolver.bindOverload(Overloaded, *FunctionValue), BindResult::Inserted);
    EXPECT_EQ(Resolver.bind(Overloaded, *Storage), BindResult::Conflict);
    EXPECT_EQ(Resolver.bind(Overloaded, *FunctionValue), BindResult::Conflict);
    EXPECT_TRUE(Resolver.lookupLocal(Overloaded)->isOverloadSet());
  }

  // Shadowed names retain distinct storage values and their properties when a new resolver resumes a saved scope.
  TEST(SemanticNameResolverTest, ShadowsAndResumesDistinctStorageValues)
  {
    IRContext Context(target::TargetContext::native());
    Module ModuleValue(Context);
    const auto *Integer = Context.typePool().getInt32Type();
    const auto *Boolean = Context.typePool().getBoolType();
    auto *OuterStorage = ModuleValue.createGlobalVariable(*Integer);
    auto *InnerStorage = ModuleValue.createGlobalVariable(*Boolean, core::NAME_None, false);
    ASSERT_NE(OuterStorage, nullptr);
    ASSERT_NE(InnerStorage, nullptr);
    ASSERT_TRUE(InnerStorage->setInitializer(Context.constantPool().getBoolConstant(true)));
    ScopeStore Store;
    NameResolver Resolver(Store);
    const Name Variable = core::NamePool::instance().intern("ShadowedStorage");
    ASSERT_EQ(Resolver.bind(Variable, *OuterStorage), BindResult::Inserted);
    const Binding *OuterBinding = Resolver.lookup(Variable);
    ASSERT_NE(OuterBinding, nullptr);
    Scope *Members = Resolver.enterScope(ModuleValue);
    ASSERT_NE(Members, nullptr);
    ASSERT_EQ(Resolver.bind(Variable, *InnerStorage), BindResult::Inserted);
    const Binding *InnerBinding = Resolver.lookup(Variable);
    ASSERT_NE(InnerBinding, nullptr);
    EXPECT_EQ(InnerBinding->targets()[0], InnerStorage);
    EXPECT_EQ(InnerBinding->targets()[0]->type(), Context.typePool().getQualifiedType(Boolean, false));
    EXPECT_FALSE(InnerBinding->targets()[0]->isMutable());
    EXPECT_TRUE(InnerBinding->targets()[0]->isInitialized());
    EXPECT_EQ(Resolver.lookupMember(ModuleValue, Variable), InnerBinding);
    Scope &Nested = Resolver.enterScope();
    EXPECT_EQ(Resolver.lookup(Variable), InnerBinding);
    EXPECT_EQ(Resolver.lookupLocal(Variable), nullptr);
    ASSERT_TRUE(Resolver.exitScope());
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookup(Variable), OuterBinding);
    EXPECT_EQ(OuterBinding->targets()[0], OuterStorage);
    EXPECT_EQ(OuterBinding->targets()[0]->type(), Integer);
    EXPECT_TRUE(OuterBinding->targets()[0]->isMutable());
    EXPECT_FALSE(OuterBinding->targets()[0]->isInitialized());
    Scope &Sibling = Resolver.enterScope();
    EXPECT_EQ(Resolver.lookup(Variable), OuterBinding);
    EXPECT_EQ(Sibling.lookupLocal(Variable), nullptr);
    NameResolver Resumed(Nested);
    EXPECT_EQ(Resumed.lookup(Variable), InnerBinding);
    ASSERT_TRUE(Resumed.enterScope(*Members));
    ASSERT_NE(Resumed.lookupLocal(Variable), nullptr);
    EXPECT_EQ(Resumed.lookupLocal(Variable)->targets()[0], InnerStorage);
    EXPECT_EQ(Resumed.bind(Variable, *InnerStorage), BindResult::AlreadyBound);
    EXPECT_EQ(Resumed.bind(Variable, *OuterStorage), BindResult::Conflict);
    EXPECT_FALSE(InnerStorage->isMutable());
    EXPECT_TRUE(InnerStorage->isInitialized());
  }

  // Name registration preserves arbitrary borrowed entities and leaves their context and runtime suitability to the analyzer.
  TEST(SemanticNameResolverTest, LeavesValueValidationToAnalyzer)
  {
    IRContext Context(target::TargetContext::native());
    IRContext Foreign(target::TargetContext::native());
    Module ModuleValue(Context);
    Module ForeignModule(Foreign);
    auto *ForeignStorage = ForeignModule.createGlobalVariable(*Foreign.typePool().getBoolType());
    ASSERT_NE(ForeignStorage, nullptr);
    ScopeStore Store;
    NameResolver Resolver(Store);
    const Name Variable = core::NamePool::instance().intern("UnvalidatedStorage");
    const Name Package = core::NamePool::instance().intern("UnvalidatedModule");
    ASSERT_EQ(Resolver.bind(Variable, *ForeignStorage), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Package, ModuleValue), BindResult::Inserted);
    const Binding *BindingValue = Resolver.lookupLocal(Variable);
    ASSERT_NE(BindingValue, nullptr);
    ASSERT_EQ(BindingValue->targets().size(), 1U);
    EXPECT_EQ(BindingValue->targets()[0], ForeignStorage);
    EXPECT_EQ(Resolver.lookupLocal(Package)->targets()[0], &ModuleValue);
    EXPECT_EQ(Resolver.bind(Variable, *ForeignStorage), BindResult::AlreadyBound);
  }
} // namespace ink::semantic::test
