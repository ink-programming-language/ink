#include "ink/ir/ir_builder.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include "ink/parser/parser.h"
#include "ink/semantic/context.h"

#include <gtest/gtest.h>

#include <string>
#include <type_traits>

namespace ink::semantic::test
{
  using namespace ink::ir;

  static_assert(BindingTarget<Value *> && BindingTarget<Decl *>);
  static_assert(!BindingTarget<Value> && !BindingTarget<Function *> && !BindingTarget<const Decl *>);
  static_assert(std::is_same_v<decltype(std::declval<const Binding<Value *> &>().targets()), std::span<Value *const>>);
  static_assert(std::is_same_v<decltype(std::declval<const Binding<Decl *> &>().targets()), std::span<Decl *const>>);

  class GenericNameResolverTest : public ::testing::Test
  {
    protected:
      GenericNameResolverTest()
          : Frontend(Compilation),
            Parsed(parser::parse(Frontend, tokenizer::tokenize(Frontend, "func F[T: type](Value: T): T { return Value; } func F[U: type](Value: U): U { return Value; } class Box[T: type] { field Item: T; }; class Other[T: type] { field Item: T; };"))),
            Context(Compilation),
            Factory(Context.irContext()),
            Resolver(Context)
      {
      }

      void SetUp() override
      {
        ASSERT_TRUE(Parsed.succeeded());
        ASSERT_EQ(Parsed.Unit->root()->statements().size(), 4U);
        Module *Owner = Factory.createModule(Context.namePool().intern("Root"));
        ASSERT_NE(Owner, nullptr);
        Root = Factory.createModuleDecl(*Owner, *Parsed.Unit->root());
        ASSERT_NE(Root, nullptr);
        First = makeFunction(0);
        Second = makeFunction(1);
        Box = makeClass(2);
        Other = makeClass(3);
        ASSERT_NE(First, nullptr);
        ASSERT_NE(Second, nullptr);
        ASSERT_NE(Box, nullptr);
        ASSERT_NE(Other, nullptr);
      }

      FunctionDecl *makeFunction(std::size_t Index)
      {
        const auto *Statement = static_cast<const parser::DeclStmt *>(Parsed.Unit->root()->statements()[Index]);
        const auto &AST = static_cast<const parser::FunctionDecl &>(*Statement->declaration());
        return Factory.createFunctionDecl(*Root, Context.namePool().intern(AST.name().Text), AST);
      }

      ClassDecl *makeClass(std::size_t Index)
      {
        const auto *Statement = static_cast<const parser::DeclStmt *>(Parsed.Unit->root()->statements()[Index]);
        const auto &AST = static_cast<const parser::ClassDecl &>(*Statement->declaration());
        return Factory.createClassDecl(*Root, Context.namePool().intern(AST.name().Text), AST);
      }

      using BindResult = NameResolver::BindResult;
      core::CompilationContext Compilation;
      core::FrontendContext Frontend;
      parser::ParseResult Parsed;
      SemanticContext Context;
      IRBuilder Factory;
      NameResolver Resolver;
      ModuleDecl *Root = nullptr;
      FunctionDecl *First = nullptr;
      FunctionDecl *Second = nullptr;
      ClassDecl *Box = nullptr;
      ClassDecl *Other = nullptr;
  };

  // Generic functions form ordered candidates, deduplicate by identity and leave ordinary value lookup empty.
  TEST_F(GenericNameResolverTest, CollectsGenericOverloads)
  {
    const Name F = First->name();
    ASSERT_EQ(Resolver.bind(F, *First), BindResult::Inserted);
    const Binding<Decl *> *BindingValue = Resolver.lookup<Decl *>(F);
    ASSERT_NE(BindingValue, nullptr);
    EXPECT_EQ(BindingValue->name(), F);
    EXPECT_TRUE(BindingValue->isOverloadSet());
    EXPECT_EQ(Resolver.lookup(F), nullptr);
    ASSERT_EQ(Resolver.bind(F, *Second), BindResult::Inserted);
    EXPECT_EQ(Resolver.bind(F, *First), BindResult::AlreadyBound);
    ASSERT_EQ(BindingValue->targets().size(), 2U);
    EXPECT_EQ(BindingValue->targets()[0], First);
    EXPECT_EQ(BindingValue->targets()[1], Second);
    EXPECT_EQ(Resolver.lookupLocal<Decl *>(F), BindingValue);
    const NameResolver &View = Resolver;
    EXPECT_EQ(View.lookup<Decl *>(F), BindingValue);
  }

  // Closed and generic functions coexist in either insertion order, while variables and generic classes conflict with both.
  TEST_F(GenericNameResolverTest, CollectsMixedFunctionCandidates)
  {
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    auto FunctionValueOwner = Factory.createFunction(First->name(), *Signature);
    Function *FunctionValue = FunctionValueOwner.get();
    auto VariableOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    Value *Variable = VariableOwner.get();
    ASSERT_NE(FunctionValue, nullptr);
    ASSERT_NE(Variable, nullptr);
    for (bool GenericFirst : {false, true})
    {
      Resolver.enterScope();
      const Name F = First->name();
      if (GenericFirst)
      {
        ASSERT_EQ(Resolver.bind(F, *First), BindResult::Inserted);
        ASSERT_EQ(Resolver.bind(F, *FunctionValue), BindResult::Inserted);
      }
      else
      {
        ASSERT_EQ(Resolver.bind(F, *FunctionValue), BindResult::Inserted);
        ASSERT_EQ(Resolver.bind(F, *First), BindResult::Inserted);
      }
      EXPECT_EQ(Resolver.bind(F, *Variable), BindResult::Conflict);
      EXPECT_EQ(Resolver.bind(F, *Box), BindResult::Conflict);
      const auto *Values = Resolver.lookup(F);
      const auto *Definitions = Resolver.lookup<Decl *>(F);
      ASSERT_NE(Values, nullptr);
      ASSERT_NE(Definitions, nullptr);
      EXPECT_TRUE(Values->isOverloadSet());
      EXPECT_TRUE(Definitions->isOverloadSet());
      ASSERT_EQ(Values->targets().size(), 1U);
      ASSERT_EQ(Definitions->targets().size(), 1U);
      EXPECT_EQ(Values->targets()[0], FunctionValue);
      EXPECT_EQ(Definitions->targets()[0], First);
      ASSERT_TRUE(Resolver.exitScope());
    }
    EXPECT_EQ(Resolver.definitionScope(*Box), nullptr);
  }

  // Generic class names are exclusive, and rejected cross-category bindings create no hidden empty entry.
  TEST_F(GenericNameResolverTest, RejectsClassAndValueCollisions)
  {
    const Name NameValue = Box->name();
    auto VariableOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    Value *Variable = VariableOwner.get();
    ASSERT_NE(Variable, nullptr);
    ASSERT_EQ(Resolver.bind(NameValue, *Box), BindResult::Inserted);
    EXPECT_EQ(Resolver.bind(NameValue, *Box), BindResult::AlreadyBound);
    EXPECT_EQ(Resolver.bind(NameValue, *Other), BindResult::Conflict);
    EXPECT_EQ(Resolver.bind(NameValue, *First), BindResult::Conflict);
    EXPECT_EQ(Resolver.bind(NameValue, *Variable), BindResult::Conflict);
    EXPECT_EQ(Resolver.lookup(NameValue), nullptr);
    const auto *Found = Resolver.lookup<Decl *>(NameValue);
    ASSERT_NE(Found, nullptr);
    EXPECT_FALSE(Found->isOverloadSet());
    ASSERT_EQ(Found->targets().size(), 1U);
    EXPECT_EQ(Found->targets()[0], Box);

    Resolver.enterScope();
    ASSERT_EQ(Resolver.bind(NameValue, *Variable), BindResult::Inserted);
    EXPECT_EQ(Resolver.bind(NameValue, *Other), BindResult::Conflict);
    EXPECT_EQ(Resolver.bind(NameValue, *First), BindResult::Conflict);
    EXPECT_EQ(Resolver.lookupLocal<Decl *>(NameValue), nullptr);
    EXPECT_EQ(Resolver.definitionScope(*Other), nullptr);
    EXPECT_EQ(Resolver.definitionScope(*First), nullptr);
  }

  // Either target category hides the entire outer name, including mixed overloads, without merging scopes.
  TEST_F(GenericNameResolverTest, ShadowsAcrossTargetCategories)
  {
    const Name F = First->name();
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    auto FunctionValueOwner = Factory.createFunction(F, *Signature);
    Function *FunctionValue = FunctionValueOwner.get();
    auto VariableOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    Value *Variable = VariableOwner.get();
    ASSERT_NE(FunctionValue, nullptr);
    ASSERT_NE(Variable, nullptr);
    ASSERT_EQ(Resolver.bind(F, *First), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(F, *FunctionValue), BindResult::Inserted);
    const auto *OuterValues = Resolver.lookup(F);
    const auto *OuterDefinitions = Resolver.lookup<Decl *>(F);

    Resolver.enterScope();
    ASSERT_EQ(Resolver.bind(F, *Variable), BindResult::Inserted);
    EXPECT_EQ(Resolver.lookup<Decl *>(F), nullptr);
    EXPECT_NE(Resolver.lookup(F), OuterValues);
    Resolver.enterScope();
    EXPECT_EQ(Resolver.lookup<Decl *>(F), nullptr);
    ASSERT_EQ(Resolver.bind(F, *Second), BindResult::Inserted);
    EXPECT_EQ(Resolver.lookup(F), nullptr);
    const auto *Inner = Resolver.lookup<Decl *>(F);
    ASSERT_NE(Inner, nullptr);
    ASSERT_EQ(Inner->targets().size(), 1U);
    EXPECT_EQ(Inner->targets()[0], Second);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookup<Decl *>(F), nullptr);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookup(F), OuterValues);
    EXPECT_EQ(Resolver.lookup<Decl *>(F), OuterDefinitions);
  }

  // Foreign declarations, module definitions and invalid names fail without recording a definition environment.
  TEST_F(GenericNameResolverTest, RejectsInvalidDeclarationBindings)
  {
    SemanticContext Foreign(Compilation);
    IRBuilder ForeignFactory(Foreign.irContext());
    const Name F = First->name();
    Module *ForeignModule = ForeignFactory.createModule(Foreign.namePool().intern("Root"));
    ASSERT_NE(ForeignModule, nullptr);
    ModuleDecl *ForeignRoot = ForeignFactory.createModuleDecl(*ForeignModule, *Parsed.Unit->root());
    ASSERT_NE(ForeignRoot, nullptr);
    auto *ForeignDefinition = ForeignFactory.createFunctionDecl(*ForeignRoot, Foreign.namePool().intern("F"), First->ast());
    auto *ModuleDefinition = Root;
    ASSERT_NE(ForeignDefinition, nullptr);
    ASSERT_NE(ModuleDefinition, nullptr);
    EXPECT_EQ(&First->module(), &Root->module());
    EXPECT_EQ(&Box->module(), &Root->module());
    EXPECT_EQ(&ModuleDefinition->module().context(), &Context.irContext());
    EXPECT_EQ(&ForeignDefinition->module(), ForeignModule);
    EXPECT_EQ(Resolver.bind(F, *ForeignDefinition), BindResult::ForeignDecl);
    EXPECT_EQ(Resolver.bind(F, *ModuleDefinition), BindResult::InvalidDecl);
    EXPECT_EQ(Resolver.bind(Name{}, *First), BindResult::InvalidName);
    Foreign.namePool().intern("Other");
    Foreign.namePool().intern("Third");
    Foreign.namePool().intern("Fourth");
    Foreign.namePool().intern("Fifth");
    const Name OutOfRange = Foreign.namePool().intern("OutOfRange");
    ASSERT_FALSE(Context.namePool().contains(OutOfRange));
    EXPECT_EQ(Resolver.bind(OutOfRange, *First), BindResult::InvalidName);
    EXPECT_EQ(Resolver.lookup<Decl *>(F), nullptr);
    EXPECT_EQ(Resolver.lookupLocal<Decl *>(Name{}), nullptr);
    EXPECT_EQ(Resolver.lookup<Decl *>(OutOfRange), nullptr);
    EXPECT_EQ(Resolver.definitionScope(*First), nullptr);
    EXPECT_EQ(Resolver.definitionScope(*ForeignDefinition), nullptr);
    EXPECT_EQ(Resolver.definitionScope(*ModuleDefinition), nullptr);
  }

  // Generic member lookup stays inside the owner and preserves its scope after exit and through aliases.
  TEST_F(GenericNameResolverTest, ResolvesGenericMembers)
  {
    const Name F = First->name();
    ASSERT_EQ(Resolver.bind(F, *First), BindResult::Inserted);
    auto *Owner = Factory.createModule(Context.namePool().intern("Owner"));
    auto *OtherOwner = Factory.createModule(Context.namePool().intern("OtherOwner"));
    ASSERT_NE(Owner, nullptr);
    ASSERT_NE(OtherOwner, nullptr);
    auto *Members = Resolver.enterScope(*Owner);
    ASSERT_NE(Members, nullptr);
    EXPECT_EQ(Resolver.lookupMember<Decl *>(*Owner, F), nullptr);
    ASSERT_EQ(Resolver.bind(F, *Second), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Box->name(), *Box), BindResult::Inserted);
    const auto *Found = Resolver.lookupLocal<Decl *>(F);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookupMember<Decl *>(*Owner, F), Found);
    EXPECT_EQ(Resolver.lookupMember(*Owner, F), nullptr);
    EXPECT_EQ(Resolver.lookupMember<Decl *>(*OtherOwner, F), nullptr);
    EXPECT_EQ(Resolver.lookupMember<Decl *>(*Owner, Name{}), nullptr);
    const auto *ClassBinding = Resolver.lookupMember<Decl *>(*Owner, Box->name());
    ASSERT_NE(ClassBinding, nullptr);
    EXPECT_FALSE(ClassBinding->isOverloadSet());
    EXPECT_EQ(Resolver.definitionScope(*Second), Members);
    ASSERT_EQ(Resolver.bind(Context.namePool().intern("Alias"), *Second), BindResult::Inserted);
    EXPECT_EQ(Resolver.definitionScope(*Second), Members);
    EXPECT_EQ(Resolver.lookup<Decl *>(F)->targets()[0], First);
  }

  // Definition scopes outlive their original resolver and remain unchanged when another resolver creates an alias.
  TEST_F(GenericNameResolverTest, PreservesDefinitionsAcrossResolvers)
  {
    Scope *DefinitionScope = nullptr;
    const Binding<Decl *> *Saved = nullptr;
    {
      NameResolver Original(Context);
      DefinitionScope = &Original.enterScope();
      ASSERT_EQ(Original.bind(First->name(), *First), BindResult::Inserted);
      Saved = Original.lookup<Decl *>(First->name());
      ASSERT_NE(Saved, nullptr);
    }
    EXPECT_EQ(Context.scopeStore().definitionScope(*First), DefinitionScope);
    EXPECT_EQ(Resolver.definitionScope(*First), DefinitionScope);
    NameResolver Resumed(*DefinitionScope);
    EXPECT_EQ(Resumed.lookupLocal<Decl *>(First->name()), Saved);
    ASSERT_EQ(Resolver.bind(Context.namePool().intern("Alias"), *First), BindResult::Inserted);
    EXPECT_EQ(Resumed.definitionScope(*First), DefinitionScope);
    EXPECT_EQ(Resolver.lookup<Decl *>(First->name()), nullptr);
  }

  // Detaching a module preserves generic bindings; destroying it removes its aliases while preserving another module's overload.
  TEST_F(GenericNameResolverTest, RemovesDestroyedModuleDeclarations)
  {
    Module *OtherOwner = Factory.createModule(Context.namePool().intern("OtherModule"));
    ASSERT_NE(OtherOwner, nullptr);
    ModuleDecl *OtherRoot = Factory.createModuleDecl(*OtherOwner, *Parsed.Unit->root());
    ASSERT_NE(OtherRoot, nullptr);
    FunctionDecl *Survivor = Factory.createFunctionDecl(*OtherRoot, First->name(), First->ast());
    ASSERT_NE(Survivor, nullptr);
    const Name F = First->name();
    const Name Alias = Context.namePool().intern("Alias");
    ASSERT_EQ(Resolver.bind(F, *First), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(F, *Survivor), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Alias, *First), BindResult::Inserted);
    const auto *Saved = Resolver.lookup<Decl *>(F);
    ASSERT_NE(Saved, nullptr);
    auto Detached = Factory.removeModule(Root->module());
    ASSERT_NE(Detached, nullptr);
    EXPECT_EQ(Resolver.lookup<Decl *>(F), Saved);
    EXPECT_EQ(Resolver.definitionScope(*First), &Resolver.rootScope());
    Detached.reset();
    EXPECT_EQ(Resolver.lookup<Decl *>(F), Saved);
    ASSERT_EQ(Saved->targets().size(), 1U);
    EXPECT_EQ(Saved->targets()[0], Survivor);
    EXPECT_EQ(Resolver.lookup<Decl *>(Alias), nullptr);
    EXPECT_EQ(Resolver.definitionScope(*Survivor), &Resolver.rootScope());
    ASSERT_TRUE(Factory.eraseModule(*OtherOwner));
    EXPECT_EQ(Resolver.lookup<Decl *>(F), nullptr);
  }

  // Definition environments and binding addresses survive scope/map growth and are not replaced by inner aliases.
  TEST_F(GenericNameResolverTest, PreservesDefinitionScopeAndBindingIdentity)
  {
    auto &DefinitionScope = Resolver.enterScope();
    ASSERT_EQ(Resolver.bind(First->name(), *First), BindResult::Inserted);
    const auto *Found = Resolver.lookup<Decl *>(First->name());
    Resolver.enterScope();
    ASSERT_EQ(Resolver.bind(Context.namePool().intern("Alias"), *First), BindResult::Inserted);
    EXPECT_EQ(Resolver.definitionScope(*First), &DefinitionScope);
    for (std::size_t Index = 0; Index < 128; ++Index)
    {
      auto *Next = makeFunction(0);
      ASSERT_NE(Next, nullptr);
      ASSERT_EQ(Resolver.bind(Context.namePool().intern("F" + std::to_string(Index)), *Next), BindResult::Inserted);
      Resolver.enterScope();
      ASSERT_TRUE(Resolver.exitScope());
    }
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookup<Decl *>(First->name()), Found);
    ASSERT_TRUE(Resolver.exitScope());
    EXPECT_EQ(Resolver.lookup<Decl *>(First->name()), nullptr);
    EXPECT_EQ(Resolver.definitionScope(*First), &DefinitionScope);
    EXPECT_EQ(Found->targets()[0], First);
  }

  // Snapshots copy both categories at one lexical level while an inner declaration hides all outer candidates of its name.
  TEST_F(GenericNameResolverTest, SnapshotsPreserveMixedBindingsAndCrossCategoryShadowing)
  {
    const Name F = First->name();
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    auto Closed = Factory.createFunction(F, *Signature);
    ASSERT_NE(Closed, nullptr);
    ASSERT_EQ(Resolver.bind(F, *Closed), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(F, *First), BindResult::Inserted);
    Scope *OriginalDefinition = Resolver.definitionScope(*First);
    Scope *MixedSnapshot = Context.scopeStore().snapshotScope(Resolver.currentScope());
    ASSERT_NE(MixedSnapshot, nullptr);
    NameResolver Mixed(*MixedSnapshot);
    ASSERT_NE(Mixed.lookup(F), nullptr);
    ASSERT_NE(Mixed.lookup<Decl *>(F), nullptr);
    EXPECT_EQ(Mixed.lookup(F)->targets()[0], Closed.get());
    EXPECT_EQ(Mixed.lookup<Decl *>(F)->targets()[0], First);
    EXPECT_EQ(Resolver.definitionScope(*First), OriginalDefinition);
    Resolver.enterScope();
    ASSERT_EQ(Resolver.bind(F, *Second), BindResult::Inserted);
    Scope *InnerSnapshot = Context.scopeStore().snapshotScope(Resolver.currentScope());
    ASSERT_NE(InnerSnapshot, nullptr);
    NameResolver Inner(*InnerSnapshot);
    EXPECT_EQ(Inner.lookup(F), nullptr);
    ASSERT_NE(Inner.lookup<Decl *>(F), nullptr);
    ASSERT_EQ(Inner.lookup<Decl *>(F)->targets().size(), 1U);
    EXPECT_EQ(Inner.lookup<Decl *>(F)->targets()[0], Second);
    ASSERT_EQ(Resolver.bind(F, *First), BindResult::Inserted);
    EXPECT_EQ(Resolver.lookup<Decl *>(F)->targets().size(), 2U);
    EXPECT_EQ(Inner.lookup<Decl *>(F)->targets().size(), 1U);
    EXPECT_EQ(Mixed.lookup<Decl *>(F)->targets().size(), 1U);
  }

  // Destroying a declaration owner clears snapshotted aliases while retaining unrelated closed-function candidates.
  TEST_F(GenericNameResolverTest, SnapshotUnregistersDestroyedDeclarations)
  {
    const Name F = First->name();
    const Name Alias = Context.namePool().intern("Alias");
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    auto Closed = Factory.createFunction(F, *Signature);
    ASSERT_NE(Closed, nullptr);
    ASSERT_EQ(Resolver.bind(F, *Closed), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(F, *First), BindResult::Inserted);
    ASSERT_EQ(Resolver.bind(Alias, *First), BindResult::Inserted);
    Scope *Snapshot = Context.scopeStore().snapshotScope(Resolver.currentScope());
    ASSERT_NE(Snapshot, nullptr);
    NameResolver Saved(*Snapshot);
    ASSERT_NE(Saved.lookup<Decl *>(F), nullptr);
    ASSERT_NE(Saved.lookup<Decl *>(Alias), nullptr);
    ASSERT_TRUE(Factory.eraseModule(Root->module()));
    EXPECT_EQ(Saved.lookup<Decl *>(F), nullptr);
    EXPECT_EQ(Saved.lookup<Decl *>(Alias), nullptr);
    ASSERT_NE(Saved.lookup(F), nullptr);
    EXPECT_EQ(Saved.lookup(F)->targets()[0], Closed.get());
  }
} // namespace ink::semantic::test
