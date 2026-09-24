#include "ink/semantic/model/context.h"
#include "ink/parser/parser.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace ink::semantic::test
{
  template <typename T>
  concept HasResolvedType = requires(const T &Object) {
    Object.type();
  };

  template <typename T>
  concept HasContext = requires(const T &Object) {
    Object.context();
  };

  template <typename T>
  concept HasInitializer = requires(const T &Object) {
    Object.initializer();
  };

  static_assert(std::is_base_of_v<Decl, FunctionDecl>);
  static_assert(std::is_base_of_v<Decl, ClassDecl>);
  static_assert(!std::is_base_of_v<Value, Decl>);
  static_assert(!std::is_copy_constructible_v<FunctionDecl>);
  static_assert(!std::is_move_constructible_v<ClassDecl>);
  static_assert(!HasResolvedType<FunctionDecl> && !HasResolvedType<ClassDecl>);
  static_assert(!HasContext<Decl> && !HasInitializer<Decl>);
  static_assert(sizeof(FunctionDecl) == sizeof(Decl));
  static_assert(sizeof(ClassDecl) == sizeof(Decl));

  // A generic function keeps only its pooled name and the original AST, including its unresolved signature and body.
  TEST(SemanticFunctionDeclTest, GenericFunctionRetainsNameAndBorrowedAST)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "func Identity[T: type](Value: T): T { return Value; }"));
    ASSERT_TRUE(Parsed.succeeded());
    const auto *Statement = static_cast<const parser::DeclStmt *>(Parsed.Unit->root()->statements()[0]);
    ASSERT_TRUE(parser::FunctionDecl::classof(Statement->declaration()));
    const auto *AST = static_cast<const parser::FunctionDecl *>(Statement->declaration());
    SemanticContext Context(Compilation);
    const Name NameValue = Context.namePool().intern(AST->name().Text);
    const FunctionDecl *Definition = Context.createFunctionDecl(NameValue, *AST);
    ASSERT_NE(Definition, nullptr);
    const Decl *Base = Definition;
    EXPECT_EQ(Context.namePool().text(Definition->name()), "Identity");
    EXPECT_EQ(&Definition->ast(), AST);
    EXPECT_EQ(&Base->ast(), AST);
    EXPECT_EQ(Definition->ast().genericParameters().size(), 1U);
    EXPECT_EQ(Definition->ast().parameters().size(), 1U);
    EXPECT_EQ(Definition->ast().returnType(), AST->returnType());
    EXPECT_EQ(Definition->ast().body(), AST->body());
    EXPECT_EQ(Definition->ast().getSourceRange(), AST->getSourceRange());
    EXPECT_TRUE(FunctionDecl::classof(Base));
    EXPECT_FALSE(ClassDecl::classof(Base));
    EXPECT_FALSE(FunctionDecl::classof(nullptr));
  }

  // A generic class retains its borrowed AST while separate closed class types carry instance identity.
  TEST(SemanticClassDeclTest, GenericClassIsSeparateFromResolvedTypes)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "class Box[T: type] { field Item: T; };"));
    ASSERT_TRUE(Parsed.succeeded());
    const auto *Statement = static_cast<const parser::DeclStmt *>(Parsed.Unit->root()->statements()[0]);
    ASSERT_TRUE(parser::ClassDecl::classof(Statement->declaration()));
    const auto *AST = static_cast<const parser::ClassDecl *>(Statement->declaration());
    SemanticContext Context(Compilation);
    const Name NameValue = Context.namePool().intern(AST->name().Text);
    const ClassDecl *Definition = Context.createClassDecl(NameValue, *AST);
    ASSERT_NE(Definition, nullptr);
    const ClassType *First = Context.createClassType(NameValue);
    const ClassType *Second = Context.createClassType(NameValue);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_EQ(Definition->name(), NameValue);
    EXPECT_EQ(&Definition->ast(), AST);
    EXPECT_EQ(Definition->ast().genericParameters().size(), 1U);
    EXPECT_EQ(Definition->ast().body(), AST->body());
    EXPECT_TRUE(ClassDecl::classof(Definition));
    EXPECT_FALSE(FunctionDecl::classof(Definition));
    EXPECT_FALSE(ClassDecl::classof(nullptr));
    static_assert(!std::is_base_of_v<Decl, ClassType>);
  }

  // Invalid names cannot register declarations; ordinary function/class ASTs cannot register generic definitions.
  TEST(SemanticFunctionDeclTest, FactoriesRejectOrdinaryDefinitionsAndInvalidNames)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "func Plain(): int32; class PlainClass; func Generic[T: type](): T; class GenericClass[T: type];"));
    ASSERT_TRUE(Parsed.succeeded());
    ASSERT_EQ(Parsed.Unit->root()->statements().size(), 4U);
    const auto Statements = Parsed.Unit->root()->statements();
    const auto *Plain = static_cast<const parser::FunctionDecl *>(static_cast<const parser::DeclStmt *>(Statements[0])->declaration());
    const auto *PlainClass = static_cast<const parser::ClassDecl *>(static_cast<const parser::DeclStmt *>(Statements[1])->declaration());
    const auto *Generic = static_cast<const parser::FunctionDecl *>(static_cast<const parser::DeclStmt *>(Statements[2])->declaration());
    const auto *GenericClass = static_cast<const parser::ClassDecl *>(static_cast<const parser::DeclStmt *>(Statements[3])->declaration());
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const Name OutOfRange = Other.namePool().intern("Foreign");
    EXPECT_EQ(Context.createModuleDecl(OutOfRange, *Parsed.Unit->root()), nullptr);
    EXPECT_EQ(Context.createFunctionDecl(OutOfRange, *Generic), nullptr);
    EXPECT_EQ(Context.createClassDecl(OutOfRange, *GenericClass), nullptr);
    EXPECT_EQ(Context.createFunctionDecl(Name{}, *Generic), nullptr);
    EXPECT_EQ(Context.createClassDecl(Name{}, *GenericClass), nullptr);
    EXPECT_EQ(Context.createModuleDecl(Name{}, *Parsed.Unit->root()), nullptr);
    EXPECT_EQ(Context.createFunctionDecl(Context.namePool().intern(Plain->name().Text), *Plain), nullptr);
    EXPECT_EQ(Context.createClassDecl(Context.namePool().intern(PlainClass->name().Text), *PlainClass), nullptr);
    EXPECT_NE(Context.createFunctionDecl(Context.namePool().intern(Generic->name().Text), *Generic), nullptr);
    EXPECT_NE(Context.createClassDecl(Context.namePool().intern(GenericClass->name().Text), *GenericClass), nullptr);
  }

  // Repeated registration and storage growth retain definition identity and borrowed ASTs without publishing instance state.
  TEST(SemanticFunctionDeclTest, DefinitionsSurviveGrowthAndDoNotAcquireInstanceState)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "func Make[T: type](): T; class Box[T: type];"));
    ASSERT_TRUE(Parsed.succeeded());
    const auto Statements = Parsed.Unit->root()->statements();
    const auto *FunctionAST = static_cast<const parser::FunctionDecl *>(static_cast<const parser::DeclStmt *>(Statements[0])->declaration());
    const auto *ClassAST = static_cast<const parser::ClassDecl *>(static_cast<const parser::DeclStmt *>(Statements[1])->declaration());
    SemanticContext Context(Compilation);
    const Name FunctionName = Context.namePool().intern(FunctionAST->name().Text);
    const Name ClassName = Context.namePool().intern(ClassAST->name().Text);
    const FunctionDecl *Definition = Context.createFunctionDecl(FunctionName, *FunctionAST);
    const ClassDecl *ClassDefinition = Context.createClassDecl(ClassName, *ClassAST);
    ModuleDecl *Module = Context.createModuleDecl(Context.namePool().intern("Example"), *Parsed.Unit->root());
    ASSERT_NE(Definition, nullptr);
    ASSERT_NE(ClassDefinition, nullptr);
    ASSERT_NE(Module, nullptr);
    Module->child().push_back(Definition);
    Module->child().push_back(ClassDefinition);
    const FunctionType *Signature = Context.getFunctionType(Context.getBoolType());
    ASSERT_NE(Signature, nullptr);
    const Function *Closed = Context.createFunction(FunctionName, *Signature);
    ASSERT_NE(Closed, nullptr);
    ASSERT_NE(Context.createCallInstruction(*Closed), nullptr);
    for (unsigned Index = 0; Index < 1024; ++Index)
    {
      const FunctionDecl *Next = Context.createFunctionDecl(FunctionName, *FunctionAST);
      const ClassDecl *NextClass = Context.createClassDecl(ClassName, *ClassAST);
      const ModuleDecl *NextModule = Context.createModuleDecl(Module->name(), *Parsed.Unit->root());
      ASSERT_NE(Next, nullptr);
      ASSERT_NE(NextClass, nullptr);
      ASSERT_NE(NextModule, nullptr);
      EXPECT_NE(NextModule, Module);
      EXPECT_TRUE(NextModule->child().empty());
      EXPECT_NE(Next, Definition);
      EXPECT_NE(NextClass, ClassDefinition);
    }
    EXPECT_EQ(Definition->name(), FunctionName);
    EXPECT_EQ(&Definition->ast(), FunctionAST);
    EXPECT_EQ(&ClassDefinition->ast(), ClassAST);
    ASSERT_EQ(Module->child().size(), 2U);
    EXPECT_EQ(Module->child()[0], Definition);
    EXPECT_EQ(Module->child()[1], ClassDefinition);
    EXPECT_EQ(&Closed->type(), Signature);
    EXPECT_EQ(FunctionAST->genericParameters().size(), 1U);
  }
} // namespace ink::semantic::test
