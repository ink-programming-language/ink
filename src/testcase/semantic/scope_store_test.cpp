#include "ink/semantic/context.h"
#include "ink/semantic/ir_builder.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <optional>
#include <type_traits>

namespace ink::semantic::test
{
  namespace
  {
    using BindResult = NameResolver::BindResult;

    class ReusableValue final : public Value
    {
      public:
        explicit ReusableValue(const SemanticContext &Context)
            : Value(Context, static_cast<ValueKind>(255), Context.typePool().getType<TypeKind::Bool>())
        {
        }
    };
  } // namespace

  static_assert(!std::is_copy_constructible_v<ScopeStore>);
  static_assert(!std::is_move_constructible_v<ScopeStore>);

  // A later resolver resumes a saved member scope and observes the exact bindings created by a destroyed resolver.
  TEST(SemanticScopeStoreTest, PreservesBindingsAfterResolverDestruction)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Builder(Context);
    Module *Owner = Builder.createModule(Context.namePool().intern("Owner"));
    ASSERT_NE(Owner, nullptr);
    const Name MemberName = Context.namePool().intern("Member");
    auto Member = Builder.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    ASSERT_NE(Member, nullptr);
    Scope *Members = nullptr;
    const Binding<Value *> *Saved = nullptr;
    {
      NameResolver Original(Context);
      Members = Original.enterScope(*Owner);
      ASSERT_NE(Members, nullptr);
      ASSERT_EQ(Original.bind(MemberName, *Member), BindResult::Inserted);
      Saved = Original.lookupLocal(MemberName);
      ASSERT_NE(Saved, nullptr);
    }
    EXPECT_EQ(Context.scopeStore().memberScope(*Owner), Members);
    NameResolver Resumed(*Members);
    EXPECT_EQ(&Resumed.currentScope(), Members);
    EXPECT_EQ(Resumed.lookupLocal(MemberName), Saved);
    EXPECT_EQ(Resumed.lookupMember(*Owner, MemberName), Saved);
    NameResolver Root(Context);
    EXPECT_EQ(Root.lookup(MemberName), nullptr);
    EXPECT_EQ(Root.lookupMember(*Owner, MemberName), Saved);
    EXPECT_EQ(Root.enterScope(*Owner), nullptr);
    EXPECT_EQ(&Root.currentScope(), &Context.scopeStore().rootScope());
  }

  // Nested module analysis shares root bindings while keeping module locals and each resolver's current scope independent.
  TEST(SemanticScopeStoreTest, KeepsModuleCursorsAndLocalsIndependent)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Builder(Context);
    Module *First = Builder.createModule(Context.namePool().intern("First"));
    Module *Second = Builder.createModule(Context.namePool().intern("Second"));
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    const Name Local = Context.namePool().intern("Local");
    const Name Shared = Context.namePool().intern("Shared");
    NameResolver Outer(Context);
    ASSERT_EQ(Outer.bind(Shared, *First), BindResult::Inserted);
    Scope *FirstScope = Outer.enterScope(*First);
    ASSERT_NE(FirstScope, nullptr);
    ASSERT_EQ(Outer.bind(Local, *First), BindResult::Inserted);
    {
      NameResolver Inner(Context);
      EXPECT_EQ(&Inner.currentScope(), &Outer.rootScope());
      Scope *SecondScope = Inner.enterScope(*Second);
      ASSERT_NE(SecondScope, nullptr);
      EXPECT_EQ(SecondScope->parent(), FirstScope->parent());
      EXPECT_EQ(Inner.lookup(Local), nullptr);
      ASSERT_EQ(Inner.bind(Local, *Second), BindResult::Inserted);
      EXPECT_EQ(Inner.lookup(Local)->targets()[0], Second);
      EXPECT_EQ(Inner.lookup(Shared)->targets()[0], First);
      EXPECT_EQ(&Outer.currentScope(), FirstScope);
      EXPECT_EQ(Outer.lookup(Local)->targets()[0], First);
      EXPECT_EQ(Outer.lookupMember(*Second, Local), Inner.lookupLocal(Local));
    }
    EXPECT_EQ(&Outer.currentScope(), FirstScope);
    EXPECT_EQ(Outer.lookup(Local)->targets()[0], First);
  }

  // Starting from a saved scope uses that scope's context and rejects targets from another store.
  TEST(SemanticScopeStoreTest, ResumesWithTheStartingScopesContext)
  {
    core::CompilationContext Compilation;
    SemanticContext First(Compilation);
    SemanticContext Second(Compilation);
    IRBuilder FirstBuilder(First);
    IRBuilder SecondBuilder(Second);
    Module *Local = FirstBuilder.createModule(First.namePool().intern("Local"));
    Module *Foreign = SecondBuilder.createModule(Second.namePool().intern("Foreign"));
    ASSERT_NE(Local, nullptr);
    ASSERT_NE(Foreign, nullptr);
    NameResolver Initial(First);
    Scope &Child = Initial.enterScope();
    NameResolver Resumed(Child);
    const Name BoundName = First.namePool().intern("Bound");
    EXPECT_EQ(&Resumed.rootScope(), &First.scopeStore().rootScope());
    EXPECT_NE(&Resumed.rootScope(), &Second.scopeStore().rootScope());
    EXPECT_EQ(Resumed.bind(BoundName, *Foreign), BindResult::ForeignValue);
    EXPECT_EQ(Resumed.enterScope(*Foreign), nullptr);
    EXPECT_EQ(&Resumed.currentScope(), &Child);
    EXPECT_EQ(Resumed.bind(BoundName, *Local), BindResult::Inserted);
    EXPECT_EQ(Initial.lookupLocal(BoundName), Resumed.lookupLocal(BoundName));
  }

  // Destroying a value clears all aliases and its member index even when a new value reuses the same address.
  TEST(SemanticScopeStoreTest, RemovesDestroyedValueBindingsAndMemberScope)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    NameResolver Resolver(Context);
    std::optional<ReusableValue> Owner(std::in_place, Context);
    const void *Address = &*Owner;
    const Name BoundName = Context.namePool().intern("Owner");
    const Name Alias = Context.namePool().intern("Alias");
    ASSERT_EQ(Resolver.bind(BoundName, *Owner), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Alias, *Owner), BindResult::Inserted);
    Scope *Members = Resolver.enterScope(*Owner);
    ASSERT_NE(Members, nullptr);
    ASSERT_EQ(Resolver.bind(BoundName, *Owner), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(BoundName, *Owner), BindResult::AlreadyBound);
    Owner.reset();
    EXPECT_EQ(Resolver.lookupLocal(BoundName), nullptr);
    EXPECT_EQ(Resolver.lookup(BoundName), nullptr);
    EXPECT_EQ(Resolver.lookup(Alias), nullptr);
    Owner.emplace(Context);
    EXPECT_EQ(static_cast<const void *>(&*Owner), Address);
    EXPECT_EQ(Context.scopeStore().memberScope(*Owner), nullptr);
    ASSERT_TRUE(Resolver.exitScope());
    Scope *NewMembers = Resolver.enterScope(*Owner);
    ASSERT_NE(NewMembers, nullptr);
    EXPECT_NE(NewMembers, Members);
  }

  // Removing one overload preserves surviving candidates and binding identity; destroying the final target removes the binding.
  TEST(SemanticScopeStoreTest, RemovesOnlyDestroyedOverloadCandidates)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Builder(Context);
    NameResolver Resolver(Context);
    const Name F = Context.namePool().intern("F");
    const Name Alias = Context.namePool().intern("Alias");
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    auto First = Builder.createFunction(F, *Signature);
    auto Second = Builder.createFunction(F, *Signature);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_EQ(Resolver.bind(F, *First), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(F, *Second), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Alias, *First), BindResult::Inserted);
    const auto *Saved = Resolver.lookup(F);
    ASSERT_NE(Saved, nullptr);
    First.reset();
    EXPECT_EQ(Resolver.lookup(F), Saved);
    ASSERT_EQ(Saved->targets().size(), 1U);
    EXPECT_EQ(Saved->targets()[0], Second.get());
    EXPECT_EQ(Resolver.lookup(Alias), nullptr);
    Second.reset();
    EXPECT_EQ(Resolver.lookup(F), nullptr);
  }
} // namespace ink::semantic::test
