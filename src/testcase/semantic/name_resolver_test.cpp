#include "ink/semantic/name_resolve/name_resolver.h"

#include "ink/semantic/context.h"

#include <gtest/gtest.h>

#include <string>
#include <type_traits>

namespace ink::semantic::test
{
  namespace
  {
    using BindResult = NameResolver::BindResult;

    Function *createUnaryFunction(SemanticContext &Context, Name FunctionName, const Type &ParameterType)
    {
      const Type *Parameters[] = {&ParameterType};
      const FunctionType *Signature = Context.getFunctionType(Context.getVoidType(), Parameters);
      return Signature ? Context.createFunction(FunctionName, *Signature) : nullptr;
    }
  } // namespace

  static_assert(!std::is_copy_constructible_v<NameResolver>);
  static_assert(!std::is_move_constructible_v<NameResolver>);
  static_assert(!std::is_copy_constructible_v<Scope>);
  static_assert(!std::is_move_constructible_v<Scope>);

  // Enter and exit restore lexical lookup, isolate siblings and keep exited scopes and bindings alive.
  TEST(SemanticNameResolverTest, ResolvesNearestLexicalBinding)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    NameResolver Resolver(Context);
    auto &Root = Resolver.rootScope();
    EXPECT_EQ(&Resolver.currentScope(), &Root);
    EXPECT_EQ(Root.parent(), nullptr);
    EXPECT_FALSE(Resolver.exitScope());
    EXPECT_EQ(&Resolver.currentScope(), &Root);
    const Name X = Context.namePool().intern("X");
    const Name Local = Context.namePool().intern("Local");
    Value *Outer = Context.createAllocaInstruction(Context.getBoolType());
    Value *Inner = Context.createAllocaInstruction(Context.getBoolType());
    ASSERT_NE(Outer, nullptr);
    ASSERT_NE(Inner, nullptr);
    ASSERT_EQ(Resolver.bind(X, *Outer), BindResult::Inserted);
    const auto *RootBinding = Resolver.lookup(X);
    ASSERT_NE(RootBinding, nullptr);
    EXPECT_EQ(RootBinding->name(), X);
    EXPECT_FALSE(RootBinding->isOverloadSet());
    EXPECT_EQ(Resolver.lookupLocal(X), RootBinding);
    auto &Child = Resolver.enterScope();
    EXPECT_EQ(&Resolver.currentScope(), &Child);
    EXPECT_EQ(Child.parent(), &Root);
    EXPECT_EQ(Resolver.lookup(X), RootBinding);
    EXPECT_EQ(Resolver.lookupLocal(X), nullptr);
    ASSERT_EQ(Resolver.bind(X, *Inner), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Local, *Inner), BindResult::Inserted);
    const auto *ChildBinding = Resolver.lookup(X);
    ASSERT_NE(ChildBinding, nullptr);
    ASSERT_EQ(ChildBinding->targets().size(), 1U);
    EXPECT_EQ(ChildBinding->targets()[0], Inner);
    auto &Grandchild = Resolver.enterScope();
    EXPECT_EQ(Grandchild.parent(), &Child);
    EXPECT_EQ(Resolver.lookup(X), ChildBinding);
    EXPECT_EQ(Resolver.lookupLocal(X), nullptr);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(&Resolver.currentScope(), &Child);
    EXPECT_EQ(Resolver.lookupLocal(X), ChildBinding);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(&Resolver.currentScope(), &Root);
    EXPECT_EQ(Resolver.lookup(X), RootBinding);
    EXPECT_EQ(Resolver.lookupLocal(X), RootBinding);
    EXPECT_EQ(Resolver.lookup(Local), nullptr);
    auto &Sibling = Resolver.enterScope();
    EXPECT_NE(&Sibling, &Child);
    EXPECT_EQ(Sibling.parent(), &Root);
    EXPECT_EQ(Resolver.lookup(X), RootBinding);
    EXPECT_EQ(Resolver.lookupLocal(X), nullptr);
    EXPECT_EQ(Resolver.lookup(Local), nullptr);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_FALSE(Resolver.exitScope());
    EXPECT_EQ(&Resolver.currentScope(), &Root);
    EXPECT_EQ(Child.parent(), &Root);
    EXPECT_EQ(Grandchild.parent(), &Child);
    EXPECT_EQ(ChildBinding->targets()[0], Inner);
  }

  // Distinct functions remain ordered candidates, including unresolved signature conflicts; identical targets are not added twice.
  TEST(SemanticNameResolverTest, CollectsOverloadsWithoutSelectingOrMergingEntities)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    NameResolver Resolver(Context);
    const Name F = Context.namePool().intern("F");
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    ASSERT_NE(Int32, nullptr);
    Function *IntegerOverload = createUnaryFunction(Context, F, *Int32);
    Function *BoolOverload = createUnaryFunction(Context, F, Context.getBoolType());
    Function *RepeatedSignature = createUnaryFunction(Context, F, *Int32);
    Value *Variable = Context.createAllocaInstruction(Context.getBoolType());
    ASSERT_NE(IntegerOverload, nullptr);
    ASSERT_NE(BoolOverload, nullptr);
    ASSERT_NE(RepeatedSignature, nullptr);
    ASSERT_NE(Variable, nullptr);
    ASSERT_EQ(Resolver.bind(F, *IntegerOverload), BindResult::Inserted);
    const auto *Binding = Resolver.lookup(F);
    ASSERT_NE(Binding, nullptr);
    EXPECT_TRUE(Binding->isOverloadSet());
    ASSERT_EQ(Resolver.bind(F, *BoolOverload), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(F, *RepeatedSignature), BindResult::Inserted);
    EXPECT_EQ(Resolver.bind(F, *IntegerOverload), BindResult::AlreadyBound);
    EXPECT_EQ(Resolver.bind(F, *Variable), BindResult::Conflict);
    EXPECT_EQ(Resolver.lookup(F), Binding);
    ASSERT_EQ(Binding->targets().size(), 3U);
    EXPECT_EQ(Binding->targets()[0], IntegerOverload);
    EXPECT_EQ(Binding->targets()[1], BoolOverload);
    EXPECT_EQ(Binding->targets()[2], RepeatedSignature);
  }

  // A function-valued call result is an ordinary binding, and distinct non-function values cannot share a name.
  TEST(SemanticNameResolverTest, RejectsNonFunctionCollisionsWithoutReplacingBindings)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    NameResolver Resolver(Context);
    const Name F = Context.namePool().intern("F");
    const Name Variable = Context.namePool().intern("Variable");
    Function *FunctionValue = createUnaryFunction(Context, F, Context.getBoolType());
    ASSERT_NE(FunctionValue, nullptr);
    const FunctionType *FactoryType = Context.getFunctionType(FunctionValue->functionType());
    ASSERT_NE(FactoryType, nullptr);
    Function *Factory = Context.createFunction(Context.namePool().intern("Factory"), *FactoryType);
    ASSERT_NE(Factory, nullptr);
    Value *Result = Context.createCallInstruction(*Factory);
    Value *First = Context.createAllocaInstruction(Context.getBoolType());
    Value *Second = Context.createAllocaInstruction(Context.getBoolType());
    ASSERT_NE(Result, nullptr);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_EQ(Resolver.bind(F, *Result), BindResult::Inserted);
    EXPECT_EQ(Resolver.bind(F, *Result), BindResult::AlreadyBound);
    EXPECT_EQ(Resolver.bind(F, *FunctionValue), BindResult::Conflict);
    EXPECT_EQ(Resolver.bind(F, *First), BindResult::Conflict);
    const auto *Binding = Resolver.lookup(F);
    ASSERT_NE(Binding, nullptr);
    EXPECT_FALSE(Binding->isOverloadSet());
    ASSERT_EQ(Binding->targets().size(), 1U);
    EXPECT_EQ(Binding->targets()[0], Result);
    ASSERT_EQ(Resolver.bind(Variable, *First), BindResult::Inserted);
    EXPECT_EQ(Resolver.bind(Variable, *Second), BindResult::Conflict);
    const auto *VariableBinding = Resolver.lookup(Variable);
    ASSERT_NE(VariableBinding, nullptr);
    EXPECT_EQ(VariableBinding->targets()[0], First);
  }

  // Nested bindings hide complete overload sets, and exiting each scope restores the previous set.
  TEST(SemanticNameResolverTest, HidesOuterOverloadSetsAsAWhole)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    NameResolver Resolver(Context);
    const Name F = Context.namePool().intern("F");
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    ASSERT_NE(Int32, nullptr);
    Function *Outer = createUnaryFunction(Context, F, *Int32);
    Function *SecondOuter = createUnaryFunction(Context, F, Context.getBoolType());
    Function *Inner = createUnaryFunction(Context, F, Context.getBoolType());
    ASSERT_NE(Outer, nullptr);
    ASSERT_NE(SecondOuter, nullptr);
    ASSERT_NE(Inner, nullptr);
    ASSERT_EQ(Resolver.bind(F, *Outer), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(F, *SecondOuter), BindResult::Inserted);
    const auto *OuterBinding = Resolver.lookup(F);
    ASSERT_NE(OuterBinding, nullptr);
    ASSERT_EQ(OuterBinding->targets().size(), 2U);
    Resolver.enterScope();
    EXPECT_EQ(Resolver.lookup(F), OuterBinding);
    EXPECT_EQ(Resolver.lookupLocal(F), nullptr);
    ASSERT_EQ(Resolver.bind(F, *Inner), BindResult::Inserted);
    const auto *Binding = Resolver.lookup(F);
    ASSERT_NE(Binding, nullptr);
    ASSERT_EQ(Binding->targets().size(), 1U);
    EXPECT_EQ(Binding->targets()[0], Inner);
    Resolver.enterScope();
    Value *Variable = Context.createAllocaInstruction(Context.getBoolType());
    ASSERT_NE(Variable, nullptr);
    ASSERT_EQ(Resolver.bind(F, *Variable), BindResult::Inserted);
    const auto *Local = Resolver.lookup(F);
    ASSERT_NE(Local, nullptr);
    EXPECT_FALSE(Local->isOverloadSet());
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookup(F), Binding);
    EXPECT_EQ(Resolver.lookupLocal(F), Binding);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookup(F), OuterBinding);
    EXPECT_EQ(Resolver.lookupLocal(F), OuterBinding);
    ASSERT_EQ(OuterBinding->targets().size(), 2U);
    EXPECT_EQ(OuterBinding->targets()[0], Outer);
    EXPECT_EQ(OuterBinding->targets()[1], SecondOuter);
    EXPECT_EQ(Local->targets()[0], Variable);
  }

  // Invalid names and foreign values fail without changing bindings or names, and resolvers keep independent scope state.
  TEST(SemanticNameResolverTest, RejectsForeignInputsAndLeavesMissesUnchanged)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext OtherContext(Compilation);
    NameResolver Resolver(Context);
    NameResolver OtherResolver(Context);
    const Name Missing = Context.namePool().intern("Missing");
    OtherContext.namePool().intern("Other");
    const Name OutOfRange = OtherContext.namePool().intern("OutOfRange");
    Value *LocalValue = Context.createAllocaInstruction(Context.getBoolType());
    Value *ForeignValue = OtherContext.createAllocaInstruction(OtherContext.getBoolType());
    ASSERT_NE(LocalValue, nullptr);
    ASSERT_NE(ForeignValue, nullptr);
    const std::size_t NameCount = Context.namePool().size();
    EXPECT_EQ(Resolver.lookup(Missing), nullptr);
    EXPECT_EQ(Resolver.lookupLocal(Missing), nullptr);
    EXPECT_EQ(Resolver.lookup(Name{}), nullptr);
    EXPECT_EQ(Resolver.lookupLocal(OutOfRange), nullptr);
    EXPECT_EQ(Resolver.bind(Name{}, *LocalValue), BindResult::InvalidName);
    EXPECT_EQ(Resolver.bind(OutOfRange, *LocalValue), BindResult::InvalidName);
    EXPECT_EQ(Resolver.bind(Missing, *ForeignValue), BindResult::ForeignValue);
    Resolver.enterScope();
    EXPECT_EQ(&OtherResolver.currentScope(), &OtherResolver.rootScope());
    ASSERT_EQ(OtherResolver.bind(Missing, *LocalValue), BindResult::Inserted);
    EXPECT_EQ(Resolver.lookup(Missing), nullptr);
    EXPECT_EQ(Resolver.lookupLocal(Missing), nullptr);
    EXPECT_NE(OtherResolver.lookup(Missing), nullptr);
    EXPECT_EQ(Context.namePool().size(), NameCount);
  }

  // Qualified paths, relative paths and owner aliases resolve to the same bindings after member scopes are exited.
  TEST(SemanticNameResolverTest, QualifiedAndRelativePathsShareMemberBindings)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    NameResolver Resolver(Context);
    const Name A = Context.namePool().intern("A");
    const Name B = Context.namePool().intern("B");
    const Name C = Context.namePool().intern("C");
    const Name Alias = Context.namePool().intern("Alias");
    Module *AValue = Context.createModule(A);
    Module *BValue = Context.createModule(B);
    Value *CValue = Context.createAllocaInstruction(Context.getBoolType());
    ASSERT_NE(AValue, nullptr);
    ASSERT_NE(BValue, nullptr);
    ASSERT_NE(CValue, nullptr);
    auto &Root = Resolver.currentScope();
    ASSERT_EQ(Resolver.bind(A, *AValue), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Alias, *AValue), BindResult::Inserted);
    auto *AScope = Resolver.enterScope(*AValue);
    ASSERT_NE(AScope, nullptr);
    EXPECT_EQ(&Resolver.currentScope(), AScope);
    EXPECT_EQ(AScope->parent(), &Root);
    ASSERT_EQ(Resolver.bind(B, *BValue), BindResult::Inserted);
    auto *BScope = Resolver.enterScope(*BValue);
    ASSERT_NE(BScope, nullptr);
    EXPECT_EQ(BScope->parent(), AScope);
    ASSERT_EQ(Resolver.bind(C, *CValue), BindResult::Inserted);
    const auto *CBinding = Resolver.lookupLocal(C);
    ASSERT_NE(CBinding, nullptr);
    ASSERT_EQ(CBinding->targets().size(), 1U);
    EXPECT_EQ(CBinding->targets()[0], CValue);
    ASSERT_TRUE(Resolver.exitScope());
    const auto *RelativeB = Resolver.lookup(B);
    ASSERT_NE(RelativeB, nullptr);
    ASSERT_EQ(RelativeB->targets().size(), 1U);
    const auto *RelativeC = Resolver.lookupMember(*RelativeB->targets()[0], C);
    EXPECT_EQ(RelativeC, CBinding);
    EXPECT_EQ(&Resolver.currentScope(), AScope);
    ASSERT_TRUE(Resolver.exitScope());
    const auto *RootA = Resolver.lookup(A);
    ASSERT_NE(RootA, nullptr);
    ASSERT_EQ(RootA->targets().size(), 1U);
    const auto *QualifiedB = Resolver.lookupMember(*RootA->targets()[0], B);
    ASSERT_EQ(QualifiedB, RelativeB);
    EXPECT_EQ(Resolver.lookupMember(*QualifiedB->targets()[0], C), RelativeC);
    const auto *AliasA = Resolver.lookup(Alias);
    ASSERT_NE(AliasA, nullptr);
    ASSERT_EQ(AliasA->targets().size(), 1U);
    EXPECT_EQ(Resolver.lookupMember(*AliasA->targets()[0], B), QualifiedB);
    const NameResolver &View = Resolver;
    EXPECT_EQ(View.lookupMember(*BValue, C), CBinding);
    EXPECT_EQ(Resolver.lookup(B), nullptr);
    EXPECT_EQ(Resolver.lookup(C), nullptr);
    EXPECT_EQ(&Resolver.currentScope(), &Root);
  }

  // Member lookup returns complete overload sets but never searches lexical parents or nested anonymous blocks.
  TEST(SemanticNameResolverTest, MemberLookupStaysWithinOwnerScope)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    NameResolver Resolver(Context);
    const Name OwnerName = Context.namePool().intern("Owner");
    const Name ParentName = Context.namePool().intern("ParentOnly");
    const Name ChildName = Context.namePool().intern("ChildOnly");
    const Name F = Context.namePool().intern("F");
    Module *Owner = Context.createModule(OwnerName);
    Value *Variable = Context.createAllocaInstruction(Context.getBoolType());
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    ASSERT_NE(Owner, nullptr);
    ASSERT_NE(Variable, nullptr);
    ASSERT_NE(Int32, nullptr);
    Function *First = createUnaryFunction(Context, F, Context.getBoolType());
    Function *Second = createUnaryFunction(Context, F, *Int32);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_EQ(Resolver.bind(ParentName, *Variable), BindResult::Inserted);
    const auto *ParentBinding = Resolver.lookupLocal(ParentName);
    ASSERT_NE(ParentBinding, nullptr);
    auto *OwnerScope = Resolver.enterScope(*Owner);
    ASSERT_NE(OwnerScope, nullptr);
    EXPECT_EQ(Resolver.lookup(ParentName), ParentBinding);
    EXPECT_EQ(Resolver.lookupMember(*Owner, ParentName), nullptr);
    ASSERT_EQ(Resolver.bind(F, *First), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(F, *Second), BindResult::Inserted);
    const auto *Overloads = Resolver.lookupLocal(F);
    ASSERT_NE(Overloads, nullptr);
    EXPECT_TRUE(Overloads->isOverloadSet());
    ASSERT_EQ(Overloads->targets().size(), 2U);
    auto &Block = Resolver.enterScope();
    ASSERT_EQ(Resolver.bind(ChildName, *Variable), BindResult::Inserted);
    EXPECT_NE(Resolver.lookupLocal(ChildName), nullptr);
    EXPECT_EQ(Resolver.lookupMember(*Owner, ChildName), nullptr);
    EXPECT_EQ(Resolver.lookupMember(*Owner, F), Overloads);
    EXPECT_EQ(&Resolver.currentScope(), &Block);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(&Resolver.currentScope(), OwnerScope);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookup(F), nullptr);
    EXPECT_EQ(Resolver.lookupMember(*Owner, F), Overloads);
    EXPECT_EQ(Overloads->targets()[0], First);
    EXPECT_EQ(Overloads->targets()[1], Second);
  }

  // Scope creation rejects foreign and duplicate owners without changing state, while same-named entities stay independent.
  TEST(SemanticNameResolverTest, MemberScopesUseEntityIdentityAndRejectInvalidRequests)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext OtherContext(Compilation);
    NameResolver Resolver(Context);
    const Name OwnerName = Context.namePool().intern("Owner");
    const Name MemberName = Context.namePool().intern("Member");
    Module *First = Context.createModule(OwnerName);
    Module *Second = Context.createModule(OwnerName);
    Value *FirstMember = Context.createAllocaInstruction(Context.getBoolType());
    Value *SecondMember = Context.createAllocaInstruction(Context.getBoolType());
    Module *Foreign = OtherContext.createModule(OtherContext.namePool().intern("Owner"));
    OtherContext.namePool().intern("Member");
    const Name OutOfRange = OtherContext.namePool().intern("OutOfRange");
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(FirstMember, nullptr);
    ASSERT_NE(SecondMember, nullptr);
    ASSERT_NE(Foreign, nullptr);
    auto &Root = Resolver.currentScope();
    const std::size_t NameCount = Context.namePool().size();
    EXPECT_EQ(Resolver.lookupMember(*First, MemberName), nullptr);
    EXPECT_EQ(Resolver.enterScope(*Foreign), nullptr);
    EXPECT_EQ(&Resolver.currentScope(), &Root);
    auto *FirstScope = Resolver.enterScope(*First);
    ASSERT_NE(FirstScope, nullptr);
    ASSERT_EQ(Resolver.bind(MemberName, *FirstMember), BindResult::Inserted);
    const auto *FirstBinding = Resolver.lookupLocal(MemberName);
    ASSERT_NE(FirstBinding, nullptr);
    EXPECT_EQ(Resolver.enterScope(*First), nullptr);
    EXPECT_EQ(Resolver.enterScope(*Foreign), nullptr);
    EXPECT_EQ(&Resolver.currentScope(), FirstScope);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.enterScope(*First), nullptr);
    EXPECT_EQ(&Resolver.currentScope(), &Root);
    EXPECT_EQ(Resolver.lookupMember(*First, MemberName), FirstBinding);
    EXPECT_EQ(Resolver.lookupMember(*Second, MemberName), nullptr);
    auto *SecondScope = Resolver.enterScope(*Second);
    ASSERT_NE(SecondScope, nullptr);
    EXPECT_NE(SecondScope, FirstScope);
    ASSERT_EQ(Resolver.bind(MemberName, *SecondMember), BindResult::Inserted);
    const auto *SecondBinding = Resolver.lookupLocal(MemberName);
    ASSERT_NE(SecondBinding, nullptr);
    EXPECT_NE(SecondBinding, FirstBinding);
    EXPECT_EQ(Resolver.lookupMember(*First, MemberName), FirstBinding);
    EXPECT_EQ(Resolver.lookupMember(*Second, MemberName), SecondBinding);
    EXPECT_EQ(FirstBinding->targets()[0], FirstMember);
    EXPECT_EQ(SecondBinding->targets()[0], SecondMember);
    EXPECT_EQ(Resolver.lookupMember(*First, Name{}), nullptr);
    EXPECT_EQ(Resolver.lookupMember(*First, OutOfRange), nullptr);
    EXPECT_EQ(Resolver.lookupMember(*Foreign, MemberName), nullptr);
    EXPECT_EQ(&Resolver.currentScope(), SecondScope);
    EXPECT_EQ(Context.namePool().size(), NameCount);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(&Resolver.currentScope(), &Root);
  }

  // Scope and binding identities survive storage growth, and deep parent lookup uses no recursive traversal.
  TEST(SemanticNameResolverTest, PreservesScopesAndBindingsAcrossGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    NameResolver Resolver(Context);
    auto &Root = Resolver.rootScope();
    const Name NameValue = Context.namePool().intern("Visible");
    Value *Variable = Context.createAllocaInstruction(Context.getBoolType());
    ASSERT_NE(Variable, nullptr);
    ASSERT_EQ(Resolver.bind(NameValue, *Variable), BindResult::Inserted);
    const auto *Binding = Resolver.lookup(NameValue);
    ASSERT_NE(Binding, nullptr);
    for (unsigned Index = 0; Index < 4096; ++Index)
    {
      const Name AddedName = Context.namePool().intern("Added" + std::to_string(Index));
      ASSERT_EQ(Resolver.bind(AddedName, *Variable), BindResult::Inserted);
    }
    EXPECT_EQ(&Resolver.rootScope(), &Root);
    EXPECT_EQ(Resolver.lookup(NameValue), Binding);
    for (unsigned Index = 0; Index < 4096; ++Index)
    {
      Resolver.enterScope();
    }
    auto &Deepest = Resolver.currentScope();
    const Name LocalName = Context.namePool().intern("DeepLocal");
    ASSERT_EQ(Resolver.bind(LocalName, *Variable), BindResult::Inserted);
    const auto *LocalBinding = Resolver.lookupLocal(LocalName);
    ASSERT_NE(LocalBinding, nullptr);
    EXPECT_EQ(Resolver.lookup(NameValue), Binding);
    EXPECT_EQ(Resolver.lookupLocal(NameValue), nullptr);
    ASSERT_EQ(Binding->targets().size(), 1U);
    EXPECT_EQ(Binding->targets()[0], Variable);
    const NameResolver &View = Resolver;
    EXPECT_EQ(&View.rootScope(), &Root);
    EXPECT_EQ(&View.currentScope(), &Deepest);
    for (unsigned Index = 0; Index < 4096; ++Index)
    {
      const auto *Parent = Resolver.currentScope().parent();
      ASSERT_TRUE(Resolver.exitScope());
      EXPECT_EQ(&Resolver.currentScope(), Parent);
    }
    EXPECT_EQ(&Resolver.currentScope(), &Root);
    EXPECT_EQ(&View.currentScope(), &Root);
    EXPECT_FALSE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookupLocal(NameValue), Binding);
    EXPECT_EQ(Resolver.lookup(LocalName), nullptr);
    EXPECT_NE(Deepest.parent(), nullptr);
    EXPECT_EQ(LocalBinding->targets()[0], Variable);
  }
} // namespace ink::semantic::test
