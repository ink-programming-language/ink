#include "ink/semantic/context.h"
#include "ink/parser/parser.h"

#include <gtest/gtest.h>

namespace ink::semantic::test
{
  // Module/class child lists retain the original declarations and their AST-based classification.
  TEST(SemanticModuleDeclTest, ChildSequencePreservesDeclarationIdentityAndKind)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "class Box[T: type] { func Make[U: type](): U; };"));
    ASSERT_TRUE(Parsed.succeeded());
    const auto *ClassAST = static_cast<const parser::ClassDecl *>(static_cast<const parser::DeclStmt *>(Parsed.Unit->root()->statements()[0])->declaration());
    const auto *FunctionAST = static_cast<const parser::FunctionDecl *>(static_cast<const parser::DeclStmt *>(ClassAST->body()->statements()[0])->declaration());
    SemanticContext Context(Compilation);
    ModuleDecl *Module = Context.createModuleDecl(Context.namePool().intern("Example"), *Parsed.Unit->root());
    ClassDecl *Class = Context.createClassDecl(Context.namePool().intern(ClassAST->name().Text), *ClassAST);
    FunctionDecl *Function = Context.createFunctionDecl(Context.namePool().intern(FunctionAST->name().Text), *FunctionAST);
    ASSERT_NE(Module, nullptr);
    ASSERT_NE(Class, nullptr);
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Context.namePool().text(Module->name()), "Example");
    EXPECT_EQ(&Module->ast(), Parsed.Unit->root());
    EXPECT_TRUE(Module->child().empty());
    EXPECT_TRUE(Class->child().empty());
    EXPECT_TRUE(Function->child().empty());
    Decl &ModuleBase = *Module;
    ModuleBase.child().push_back(Class);
    Class->child().push_back(Function);

    const Decl &Scope = *Module;
    EXPECT_EQ(&Scope.ast(), Parsed.Unit->root());
    EXPECT_TRUE(ModuleDecl::classof(&Scope));
    EXPECT_FALSE(ClassDecl::classof(&Scope));
    EXPECT_FALSE(FunctionDecl::classof(&Scope));
    EXPECT_FALSE(ModuleDecl::classof(Class));
    EXPECT_FALSE(ModuleDecl::classof(Function));
    EXPECT_FALSE(ModuleDecl::classof(nullptr));
    ASSERT_EQ(Scope.child().size(), 1U);
    const Decl *ClassChild = Scope.child()[0];
    EXPECT_EQ(ClassChild, Class);
    EXPECT_TRUE(ClassDecl::classof(ClassChild));
    ASSERT_EQ(ClassChild->child().size(), 1U);
    const Decl *FunctionChild = ClassChild->child()[0];
    EXPECT_EQ(FunctionChild, Function);
    EXPECT_TRUE(FunctionDecl::classof(FunctionChild));
  }
} // namespace ink::semantic::test
