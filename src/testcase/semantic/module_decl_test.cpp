#include "ink/ir/ir_builder.h"
#include "ink/semantic/context.h"
#include "ink/parser/parser.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

namespace ink::semantic::test
{
  using namespace ink::ir;

  static_assert(std::is_same_v<decltype(std::declval<Decl &>().children()), const std::vector<std::unique_ptr<Decl>> &>);

  // The module owns one root, whose nested children retain their owners, parents and AST-based classification.
  TEST(SemanticModuleDeclTest, ModuleOwnsIndependentDeclarationTree)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "class Box[T: type] { func Make[U: type](): U; };"));
    ASSERT_TRUE(Parsed.succeeded());
    const auto *ClassAST = static_cast<const parser::ClassDecl *>(static_cast<const parser::DeclStmt *>(Parsed.Unit->root()->statements()[0])->declaration());
    const auto *FunctionAST = static_cast<const parser::FunctionDecl *>(static_cast<const parser::DeclStmt *>(ClassAST->body()->statements()[0])->declaration());
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context.irContext());
    Module *Owner = Factory.createModule(Context.namePool().intern("Example"));
    ASSERT_NE(Owner, nullptr);
    EXPECT_EQ(Owner->declarationRoot(), nullptr);
    ModuleDecl *Root = Factory.createModuleDecl(*Owner, *Parsed.Unit->root());
    ASSERT_NE(Root, nullptr);
    EXPECT_TRUE(Root->children().empty());
    ClassDecl *Class = Factory.createClassDecl(*Root, Context.namePool().intern(ClassAST->name().Text), *ClassAST);
    ASSERT_NE(Class, nullptr);
    EXPECT_TRUE(Class->children().empty());
    FunctionDecl *Function = Factory.createFunctionDecl(*Class, Context.namePool().intern(FunctionAST->name().Text), *FunctionAST);
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Owner->declarationRoot(), Root);
    EXPECT_EQ(Root->name(), Owner->name());
    EXPECT_EQ(Context.namePool().text(Root->name()), "Example");
    EXPECT_EQ(Root->parent(), nullptr);
    EXPECT_EQ(Class->parent(), Root);
    EXPECT_EQ(Function->parent(), Class);
    EXPECT_EQ(&Root->module(), Owner);
    EXPECT_EQ(&Class->module(), Owner);
    EXPECT_EQ(&Function->module(), Owner);
    EXPECT_TRUE(Function->children().empty());
    EXPECT_TRUE(Owner->entryBlock().values().empty());

    const Module &ModuleView = *Owner;
    EXPECT_EQ(ModuleView.declarationRoot(), Root);
    const Decl &Scope = *Root;
    EXPECT_EQ(Scope.parent(), nullptr);
    EXPECT_EQ(&Scope.module(), Owner);
    EXPECT_EQ(&Scope.ast(), Parsed.Unit->root());
    EXPECT_TRUE(ModuleDecl::classof(&Scope));
    EXPECT_FALSE(ClassDecl::classof(&Scope));
    EXPECT_FALSE(FunctionDecl::classof(&Scope));
    EXPECT_FALSE(ModuleDecl::classof(Class));
    EXPECT_FALSE(ModuleDecl::classof(Function));
    EXPECT_FALSE(ModuleDecl::classof(nullptr));
    ASSERT_EQ(Scope.children().size(), 1U);
    const Decl *ClassChild = Scope.children()[0].get();
    EXPECT_EQ(ClassChild, Class);
    EXPECT_TRUE(ClassDecl::classof(ClassChild));
    EXPECT_EQ(ClassChild->parent(), Root);
    ASSERT_EQ(ClassChild->children().size(), 1U);
    const Decl *FunctionChild = ClassChild->children()[0].get();
    EXPECT_EQ(FunctionChild, Function);
    EXPECT_TRUE(FunctionDecl::classof(FunctionChild));
  }

  // Duplicate roots and foreign module/parent arguments are rejected without changing either declaration tree.
  TEST(SemanticModuleDeclTest, RejectsDuplicateRootsAndForeignOwners)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "func Make[T: type](): T; class Box[T: type];"));
    ASSERT_TRUE(Parsed.succeeded());
    const auto Statements = Parsed.Unit->root()->statements();
    const auto *FunctionAST = static_cast<const parser::FunctionDecl *>(static_cast<const parser::DeclStmt *>(Statements[0])->declaration());
    const auto *ClassAST = static_cast<const parser::ClassDecl *>(static_cast<const parser::DeclStmt *>(Statements[1])->declaration());
    SemanticContext Context(Compilation);
    SemanticContext Foreign(Compilation);
    IRBuilder Factory(Context.irContext());
    IRBuilder ForeignFactory(Foreign.irContext());
    Module *Owner = Factory.createModule(Context.namePool().intern("Local"));
    Module *ForeignOwner = ForeignFactory.createModule(Foreign.namePool().intern("Foreign"));
    ASSERT_NE(Owner, nullptr);
    ASSERT_NE(ForeignOwner, nullptr);
    EXPECT_EQ(Factory.createModuleDecl(*ForeignOwner, *Parsed.Unit->root()), nullptr);
    EXPECT_EQ(ForeignOwner->declarationRoot(), nullptr);
    ModuleDecl *Root = Factory.createModuleDecl(*Owner, *Parsed.Unit->root());
    ModuleDecl *ForeignRoot = ForeignFactory.createModuleDecl(*ForeignOwner, *Parsed.Unit->root());
    ASSERT_NE(Root, nullptr);
    ASSERT_NE(ForeignRoot, nullptr);
    EXPECT_EQ(Factory.createModuleDecl(*Owner, *Parsed.Unit->root()), nullptr);
    EXPECT_EQ(Owner->declarationRoot(), Root);
    EXPECT_EQ(Factory.createFunctionDecl(*ForeignRoot, Context.namePool().intern("Make"), *FunctionAST), nullptr);
    EXPECT_EQ(Factory.createClassDecl(*ForeignRoot, Context.namePool().intern("Box"), *ClassAST), nullptr);
    EXPECT_EQ(ForeignFactory.createFunctionDecl(*Root, Foreign.namePool().intern("Make"), *FunctionAST), nullptr);
    EXPECT_EQ(ForeignFactory.createClassDecl(*Root, Foreign.namePool().intern("Box"), *ClassAST), nullptr);
    EXPECT_TRUE(Root->children().empty());
    EXPECT_TRUE(ForeignRoot->children().empty());
  }

  // Declarations survive independent IR deletion and follow their module through detachment, nesting and restoration.
  TEST(SemanticModuleDeclTest, DeclarationTreeFollowsModuleOwnership)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "func Make[T: type](): T;"));
    ASSERT_TRUE(Parsed.succeeded());
    const auto *FunctionAST = static_cast<const parser::FunctionDecl *>(static_cast<const parser::DeclStmt *>(Parsed.Unit->root()->statements()[0])->declaration());
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context.irContext());
    Module *Owner = Factory.createModule(Context.namePool().intern("Example"));
    ASSERT_NE(Owner, nullptr);
    ModuleDecl *Root = Factory.createModuleDecl(*Owner, *Parsed.Unit->root());
    ASSERT_NE(Root, nullptr);
    const Name FunctionName = Context.namePool().intern("Make");
    FunctionDecl *Definition = Factory.createFunctionDecl(*Root, FunctionName, *FunctionAST);
    ASSERT_NE(Definition, nullptr);
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>());
    ASSERT_NE(Signature, nullptr);
    auto ClosedOwner = Factory.createFunction(FunctionName, *Signature);
    Function *Closed = ClosedOwner.get();
    ASSERT_NE(Closed, nullptr);
    ASSERT_TRUE(Factory.appendValue(Owner->entryBlock(), std::move(ClosedOwner)));
    ASSERT_TRUE(Factory.eraseValue(Owner->entryBlock(), *Closed));
    ASSERT_EQ(Root->children().size(), 1U);
    EXPECT_EQ(Root->children()[0].get(), Definition);

    auto Detached = Factory.removeModule(*Owner);
    ASSERT_EQ(Detached.get(), Owner);
    EXPECT_TRUE(Context.modules().empty());
    EXPECT_EQ(Detached->declarationRoot(), Root);
    EXPECT_EQ(&Definition->module(), Owner);
    FunctionDecl *Second = Factory.createFunctionDecl(*Root, FunctionName, *FunctionAST);
    ASSERT_NE(Second, nullptr);
    {
      NameResolver Resolver(Context);
      EXPECT_EQ(Resolver.bind(FunctionName, *Definition), NameResolver::BindResult::Inserted);
      EXPECT_EQ(Resolver.bind(FunctionName, *Second), NameResolver::BindResult::Inserted);
      EXPECT_NE(Resolver.lookup<Decl *>(FunctionName), nullptr);
    }

    Module *Container = Factory.createModule(Context.namePool().intern("Container"));
    ASSERT_NE(Container, nullptr);
    ASSERT_TRUE(Factory.appendValue(Container->entryBlock(), std::move(Detached)));
    EXPECT_EQ(Container->declarationRoot(), nullptr);
    EXPECT_EQ(Owner->outer(), &Container->entryBlock());
    EXPECT_EQ(Owner->declarationRoot(), Root);
    EXPECT_EQ(Root->parent(), nullptr);
    EXPECT_EQ(Definition->parent(), Root);
    EXPECT_EQ(&Definition->module(), Owner);
    auto Restored = Factory.removeValue(Container->entryBlock(), *Owner);
    ASSERT_EQ(Restored.get(), Owner);
    ASSERT_TRUE(Factory.appendModule(std::move(Restored)));
    EXPECT_EQ(Owner->outer(), nullptr);
    EXPECT_EQ(Owner->declarationRoot(), Root);
    ASSERT_EQ(Root->children().size(), 2U);
    EXPECT_EQ(Root->children()[1].get(), Second);
    ASSERT_TRUE(Factory.eraseModule(*Owner));
    ASSERT_EQ(Context.modules().size(), 1U);
    EXPECT_EQ(Context.modules()[0].get(), Container);
  }
} // namespace ink::semantic::test
