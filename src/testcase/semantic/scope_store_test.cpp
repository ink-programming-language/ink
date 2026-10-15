#include "ink/core/name/name_pool.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <string>
#include <type_traits>
#include <utility>

namespace ink::semantic::test
{
  using namespace ink::ir;
  using core::Name;

  static_assert(!std::is_copy_constructible_v<ScopeStore>);
  static_assert(!std::is_move_constructible_v<ScopeStore>);

  // Store accessors preserve constness for direct scope storage and module-to-value conversions through the member index.
  static_assert(std::is_same_v<decltype(std::declval<ScopeStore &>().rootScope()), Scope &>);
  static_assert(std::is_same_v<decltype(std::declval<const ScopeStore &>().rootScope()), const Scope &>);
  static_assert(std::is_same_v<decltype(std::declval<ScopeStore &&>().rootScope()), Scope &>);
  static_assert(std::is_same_v<decltype(std::declval<const ScopeStore &&>().rootScope()), const Scope &>);
  static_assert(std::is_same_v<decltype(std::declval<ScopeStore &>().memberScope(std::declval<const Value &>())), Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<const ScopeStore &>().memberScope(std::declval<const Value &>())), const Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<ScopeStore &&>().memberScope(std::declval<const Value &>())), Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<const ScopeStore &&>().memberScope(std::declval<const Value &>())), const Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<ScopeStore &>().memberScope(std::declval<const Module &>())), Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<const ScopeStore &>().memberScope(std::declval<const Module &>())), const Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<ScopeStore &&>().memberScope(std::declval<const Module &>())), Scope *>);
  static_assert(std::is_same_v<decltype(std::declval<const ScopeStore &&>().memberScope(std::declval<const Module &>())), const Scope *>);

  // Module and base references find one explicitly created member scope, while duplicate owners and foreign parents are rejected.
  TEST(SemanticScopeStoreTest, SharesOwnerIndexAndRejectsInvalidCreation)
  {
    IRContext Context(target::TargetContext::native());
    Module Owner(Context);
    Module Other(Context);
    Value &OwnerValue = Owner;
    Value &OtherValue = Other;
    auto *ValueOwner = Owner.createGlobalVariable(*Context.typePool().getBoolType());
    ASSERT_NE(ValueOwner, nullptr);
    ScopeStore Store;
    ScopeStore Foreign;
    Scope &Root = Store.rootScope();
    EXPECT_EQ(&Root.scopeStore(), &Store);
    EXPECT_EQ(Root.parent(), nullptr);
    EXPECT_FALSE(Root.owner().has_value());
    EXPECT_EQ(Store.createScope(Foreign.rootScope()), nullptr);
    EXPECT_EQ(Store.createScope(Foreign.rootScope(), Owner), nullptr);
    EXPECT_EQ(Store.createScope(Foreign.rootScope(), *ValueOwner), nullptr);
    EXPECT_EQ(Store.memberScope(Owner), nullptr);
    EXPECT_EQ(Store.memberScope(*ValueOwner), nullptr);
    Scope *Members = Store.createScope(Root, Owner);
    ASSERT_NE(Members, nullptr);
    EXPECT_EQ(Members->parent(), &Root);
    EXPECT_EQ(Store.memberScope(Owner), Members);
    EXPECT_EQ(Store.memberScope(OwnerValue), Members);
    EXPECT_EQ(Store.createScope(Root, Owner), nullptr);
    EXPECT_EQ(Store.createScope(Root, OwnerValue), nullptr);
    EXPECT_EQ(Store.createScope(*Members, Owner), nullptr);
    Scope *ValueMembers = Store.createScope(*Members, *ValueOwner);
    ASSERT_NE(ValueMembers, nullptr);
    EXPECT_EQ(ValueMembers->parent(), Members);
    EXPECT_EQ(Store.createScope(Root, *ValueOwner), nullptr);
    EXPECT_EQ(Store.memberScope(*ValueOwner), ValueMembers);
    Scope *OtherMembers = Store.createScope(*Members, OtherValue);
    ASSERT_NE(OtherMembers, nullptr);
    EXPECT_EQ(Store.memberScope(Other), OtherMembers);
    EXPECT_EQ(Store.memberScope(OtherValue), OtherMembers);
    EXPECT_EQ(Store.createScope(Root, Other), nullptr);
    EXPECT_EQ(OtherMembers->parent(), Members);
    const ScopeStore &ConstStore = Store;
    EXPECT_EQ(&ConstStore.rootScope(), &Root);
    EXPECT_EQ(ConstStore.memberScope(Owner), Members);
    EXPECT_EQ(ConstStore.memberScope(OwnerValue), Members);
    EXPECT_EQ(ConstStore.memberScope(*ValueOwner), ValueMembers);
    EXPECT_EQ(Foreign.memberScope(Owner), nullptr);
  }

  // Growing scope storage and local binding tables preserves previously returned scope and binding addresses.
  TEST(SemanticScopeStoreTest, PreservesAddressesAcrossStorageGrowth)
  {
    IRContext Context(target::TargetContext::native());
    Module ModuleValue(Context);
    auto *Target = ModuleValue.createGlobalVariable(*Context.typePool().getBoolType());
    ASSERT_NE(Target, nullptr);
    ScopeStore Store;
    NameResolver Resolver(Store);
    Scope &Root = Store.rootScope();
    const Name SavedName = core::NamePool::instance().intern("Saved");
    ASSERT_EQ(Resolver.bind(SavedName, *Target), BindResult::Inserted);
    const Binding *SavedBinding = Resolver.lookupLocal(SavedName);
    ASSERT_NE(SavedBinding, nullptr);
    Scope *SavedScope = Store.createScope(Root, ModuleValue);
    ASSERT_NE(SavedScope, nullptr);
    for (std::size_t Index = 0; Index < 1024; ++Index)
    {
      ASSERT_NE(Store.createScope(Root), nullptr);
      const Name Added = core::NamePool::instance().intern("ScopeGrowth" + std::to_string(Index));
      ASSERT_EQ(Resolver.bind(Added, *Target), BindResult::Inserted);
    }
    EXPECT_EQ(&Store.rootScope(), &Root);
    EXPECT_EQ(Store.memberScope(ModuleValue), SavedScope);
    EXPECT_EQ(SavedScope->parent(), &Root);
    EXPECT_EQ(&SavedScope->scopeStore(), &Store);
    EXPECT_EQ(Resolver.lookupLocal(SavedName), SavedBinding);
    ASSERT_EQ(SavedBinding->targets().size(), 1U);
    EXPECT_EQ(SavedBinding->targets()[0], static_cast<Value *>(Target));
    ASSERT_TRUE(Resolver.enterScope(*SavedScope));
    EXPECT_EQ(Resolver.lookup(SavedName), SavedBinding);
  }

  // Removing a value clears all aliases and its owner index, while keeping scopes and unrelated bindings usable.
  TEST(SemanticScopeStoreTest, RemovesValueAliasesAndOwnerIdentity)
  {
    IRContext Context(target::TargetContext::native());
    Module ModuleValue(Context);
    auto *Removed = ModuleValue.createGlobalVariable(*Context.typePool().getBoolType());
    auto *Survivor = ModuleValue.createGlobalVariable(*Context.typePool().getBoolType());
    ASSERT_NE(Removed, nullptr);
    ASSERT_NE(Survivor, nullptr);
    ScopeStore Store;
    NameResolver Resolver(Store);
    const Name X = core::NamePool::instance().intern("X");
    const Name Alias = core::NamePool::instance().intern("Alias");
    const Name Other = core::NamePool::instance().intern("Other");
    ASSERT_EQ(Resolver.bind(X, *Removed), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Alias, *Removed), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Other, *Survivor), BindResult::Inserted);
    const Binding *SurvivingBinding = Resolver.lookupLocal(Other);
    Scope *Members = Resolver.enterScope(*Removed);
    ASSERT_NE(Members, nullptr);
    ASSERT_EQ(Resolver.bind(Alias, *Removed), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Other, *Survivor), BindResult::Inserted);
    const Binding *SurvivingMember = Resolver.lookupLocal(Other);
    Scope &Child = Resolver.enterScope();
    ASSERT_EQ(Resolver.bind(X, *Removed), BindResult::Inserted);
    Store.removeTarget(*Removed);
    EXPECT_EQ(&Resolver.currentScope(), &Child);
    EXPECT_EQ(Child.parent(), Members);
    EXPECT_FALSE(Members->owner().has_value());
    EXPECT_EQ(Store.memberScope(*Removed), nullptr);
    EXPECT_EQ(Resolver.lookup(X), nullptr);
    EXPECT_EQ(Resolver.lookup(Alias), nullptr);
    EXPECT_EQ(Resolver.lookup(Other), SurvivingMember);
    EXPECT_EQ(Store.rootScope().lookupLocal(X), nullptr);
    EXPECT_EQ(Store.rootScope().lookupLocal(Alias), nullptr);
    EXPECT_EQ(Store.rootScope().lookupLocal(Other), SurvivingBinding);
    Store.removeTarget(*Removed);
    Scope *Replacement = Store.createScope(Store.rootScope(), *Removed);
    ASSERT_NE(Replacement, nullptr);
    EXPECT_NE(Replacement, Members);
    EXPECT_EQ(Store.memberScope(*Removed), Replacement);
    EXPECT_EQ(Members->lookupLocal(Other), SurvivingMember);
  }

  // Removing one overload preserves candidate order and binding identity until its final target is removed.
  TEST(SemanticScopeStoreTest, RemovesOnlyMatchingOverloadCandidates)
  {
    IRContext Context(target::TargetContext::native());
    Module ModuleValue(Context);
    const auto *Signature = Context.typePool().getFunctionType(Context.typePool().getVoidType());
    ASSERT_NE(Signature, nullptr);
    auto *First = ModuleValue.createGlobalFunction(*Signature);
    auto *Second = ModuleValue.createGlobalFunction(*Signature);
    auto *Third = ModuleValue.createGlobalFunction(*Signature);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Third, nullptr);
    ScopeStore Store;
    NameResolver Resolver(Store);
    const Name F = core::NamePool::instance().intern("F");
    const Name Alias = core::NamePool::instance().intern("Alias");
    ASSERT_EQ(Resolver.bindOverload(F, *First), BindResult::Inserted);
    ASSERT_EQ(Resolver.bindOverload(F, *Second), BindResult::Inserted);
    ASSERT_EQ(Resolver.bindOverload(F, *Third), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Alias, *Second), BindResult::Inserted);
    const Binding *Saved = Resolver.lookup(F);
    ASSERT_NE(Saved, nullptr);
    Store.removeTarget(*Second);
    EXPECT_EQ(Resolver.lookup(F), Saved);
    EXPECT_EQ(Resolver.lookup(Alias), nullptr);
    ASSERT_EQ(Saved->targets().size(), 2U);
    EXPECT_EQ(Saved->targets()[0], static_cast<Value *>(First));
    EXPECT_EQ(Saved->targets()[1], static_cast<Value *>(Third));
    EXPECT_TRUE(Saved->isOverloadSet());
    Store.removeTarget(*First);
    EXPECT_EQ(Resolver.lookup(F), Saved);
    ASSERT_EQ(Saved->targets().size(), 1U);
    EXPECT_EQ(Saved->targets()[0], static_cast<Value *>(Third));
    EXPECT_TRUE(Saved->isOverloadSet());
    Store.removeTarget(*Third);
    EXPECT_EQ(Resolver.lookup(F), nullptr);
  }

  // Removing a module through Value clears its aliases and owner identity while preserving declarations and other modules.
  TEST(SemanticScopeStoreTest, RemovesModuleWithoutRemovingItsDeclarations)
  {
    IRContext Context(target::TargetContext::native());
    Module Removed(Context);
    Module Other(Context);
    Value &RemovedValue = Removed;
    auto *Member = Removed.createGlobalVariable(*Context.typePool().getBoolType());
    ASSERT_NE(Member, nullptr);
    ScopeStore Store;
    NameResolver Resolver(Store);
    const Name ModuleName = core::NamePool::instance().intern("Module");
    const Name Alias = core::NamePool::instance().intern("Alias");
    const Name OtherName = core::NamePool::instance().intern("Other");
    const Name Field = core::NamePool::instance().intern("Field");
    ASSERT_EQ(Resolver.bind(ModuleName, Removed), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Alias, RemovedValue), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(OtherName, Other), BindResult::Inserted);
    Scope *OtherMembers = Store.createScope(Store.rootScope(), Other);
    ASSERT_NE(OtherMembers, nullptr);
    Scope *Members = Resolver.enterScope(RemovedValue);
    ASSERT_NE(Members, nullptr);
    ASSERT_EQ(Resolver.bind(Alias, Removed), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Field, *Member), BindResult::Inserted);
    const Binding *SavedMember = Resolver.lookupLocal(Field);
    Store.removeTarget(RemovedValue);
    EXPECT_EQ(&Resolver.currentScope(), Members);
    EXPECT_FALSE(Members->owner().has_value());
    EXPECT_EQ(Store.memberScope(Removed), nullptr);
    EXPECT_EQ(Store.memberScope(RemovedValue), nullptr);
    EXPECT_EQ(Resolver.lookup(ModuleName), nullptr);
    EXPECT_EQ(Resolver.lookup(Alias), nullptr);
    EXPECT_EQ(Resolver.lookupLocal(Field), SavedMember);
    EXPECT_EQ(Store.memberScope(Other), OtherMembers);
    ASSERT_NE(Resolver.lookup(OtherName), nullptr);
    EXPECT_EQ(Resolver.lookup(OtherName)->targets()[0], static_cast<Value *>(&Other));
    Scope *Replacement = Store.createScope(Store.rootScope(), Removed);
    ASSERT_NE(Replacement, nullptr);
    EXPECT_NE(Replacement, Members);
    EXPECT_EQ(Replacement->parent(), &Store.rootScope());
    EXPECT_EQ(Store.memberScope(Removed), Replacement);
    EXPECT_EQ(Store.memberScope(RemovedValue), Replacement);
    EXPECT_EQ(Members->lookupLocal(Field), SavedMember);
    Store.removeTarget(*Member);
    EXPECT_EQ(Members->lookupLocal(Field), nullptr);
  }
} // namespace ink::semantic::test
