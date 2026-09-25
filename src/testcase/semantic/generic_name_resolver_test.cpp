#include "ink/semantic/name_resolve/name_resolver.h"

#include "ink/parser/parser.h"
#include "ink/semantic/context.h"

#include <gtest/gtest.h>

#include <string>
#include <type_traits>

namespace ink::semantic::test
{
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
            Resolver(Context)
      {
      }

      void SetUp() override
      {
        ASSERT_TRUE(Parsed.succeeded());
        ASSERT_EQ(Parsed.Unit->root()->statements().size(), 4U);
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
        return Context.createFunctionDecl(Context.namePool().intern(AST.name().Text), AST);
      }

      ClassDecl *makeClass(std::size_t Index)
      {
        const auto *Statement = static_cast<const parser::DeclStmt *>(Parsed.Unit->root()->statements()[Index]);
        const auto &AST = static_cast<const parser::ClassDecl &>(*Statement->declaration());
        return Context.createClassDecl(Context.namePool().intern(AST.name().Text), AST);
      }

      using BindResult = NameResolver::BindResult;
      core::CompilationContext Compilation;
      core::FrontendContext Frontend;
      parser::ParseResult Parsed;
      SemanticContext Context;
      NameResolver Resolver;
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
    const auto *Signature = Context.getFunctionType(Context.getVoidType());
    ASSERT_NE(Signature, nullptr);
    Function *FunctionValue = Context.createFunction(First->name(), *Signature);
    Value *Variable = Context.createAllocaInstruction(Context.getBoolType());
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
    Value *Variable = Context.createAllocaInstruction(Context.getBoolType());
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
    const auto *Signature = Context.getFunctionType(Context.getVoidType());
    ASSERT_NE(Signature, nullptr);
    Function *FunctionValue = Context.createFunction(F, *Signature);
    Value *Variable = Context.createAllocaInstruction(Context.getBoolType());
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
    const Name F = First->name();
    auto *ForeignDefinition = Foreign.createFunctionDecl(Foreign.namePool().intern("F"), First->ast());
    auto *ModuleDefinition = Context.createModuleDecl(Context.namePool().intern("Module"), *Parsed.Unit->root());
    ASSERT_NE(ForeignDefinition, nullptr);
    ASSERT_NE(ModuleDefinition, nullptr);
    EXPECT_TRUE(Context.owns(*First));
    EXPECT_TRUE(Context.owns(*Box));
    EXPECT_TRUE(Context.owns(*ModuleDefinition));
    EXPECT_FALSE(Context.owns(*ForeignDefinition));
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
    auto *Owner = Context.createModule(Context.namePool().intern("Owner"));
    auto *OtherOwner = Context.createModule(Context.namePool().intern("OtherOwner"));
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
} // namespace ink::semantic::test
