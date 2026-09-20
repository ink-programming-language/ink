#include "parser_test_support.h"
namespace ink::parser::test
{
  // Type positions reject ungrouped binary operators while allowing complete expressions in parentheses.
  TEST_F(ParserTest, TypeGrammar)
  {
    EXPECT_FALSE(read("var x: A + B;").succeeded());
    const auto Result = read("var x: (A + B); var y: comptime *T::[U][n]; var z: do { yield T; };");
    ASSERT_TRUE(Result.succeeded());
    EXPECT_TRUE(isa<ParenExpr>(cast<VarDecl>(declaration(Result))->type()->expression()));
    EXPECT_TRUE(isa<ComptimeExpr>(cast<VarDecl>(declaration(Result, 1))->type()->expression()));
    EXPECT_TRUE(isa<BlockExpr>(cast<VarDecl>(declaration(Result, 2))->type()->expression()));
  }

  // Function declarations preserve generic/default/variadic parameters and distinguish forward declarations.
  TEST_F(ParserTest, FunctionDeclarations)
  {
    const auto Result = read("func f[T: type](x: T = 1, ys: T...): T { return x; } func g(): int;");
    ASSERT_TRUE(Result.succeeded());
    const auto *Function = cast<FunctionDecl>(declaration(Result));
    EXPECT_EQ(Function->name().Text, "f");
    EXPECT_EQ(Function->genericParameters().size(), 1U);
    ASSERT_EQ(Function->parameters().size(), 2U);
    EXPECT_NE(Function->parameters()[0].defaultValue(), nullptr);
    EXPECT_TRUE(Function->parameters()[1].variadic());
    EXPECT_EQ(Function->bodyKind(), FunctionBodyKind::Definition);
    EXPECT_TRUE(isa<ReturnStmt>(Function->body()->statements()[0]));
    const auto *Forward = cast<FunctionDecl>(declaration(Result, 1));
    EXPECT_EQ(Forward->bodyKind(), FunctionBodyKind::DeclarationOnly);
    EXPECT_EQ(Forward->body(), nullptr);
  }

  // Lambda-only header features commit to lambdas even when their required block is missing.
  TEST_F(ParserTest, LambdaAndFunctionTypeDisambiguation)
  {
    const auto Result = read("func(x: T = 1): T { return x; }; func[T: type](x: T) {}; func(T, named: U...): R()[n];");
    ASSERT_TRUE(Result.succeeded());
    EXPECT_TRUE(isa<LambdaExpr>(expression(Result)));
    EXPECT_EQ(cast<LambdaExpr>(expression(Result, 1))->genericParameters().size(), 1U);
    const auto *Type = cast<FunctionTypeExpr>(expression(Result, 2));
    EXPECT_FALSE(Type->parameters()[0].name());
    EXPECT_TRUE(Type->parameters()[1].variadic());
    EXPECT_TRUE(isa<IndexExpr>(Type->returnType()->expression()));
    const auto Missing = read("var f = func(x: T = 1): T; var kept = 2;");
    EXPECT_FALSE(Missing.succeeded());
    EXPECT_TRUE(cast<LambdaExpr>(cast<VarDecl>(declaration(Missing))->initializer())->body()->synthetic());
    EXPECT_EQ(Missing.Unit->root()->statements().size(), 2U);
    EXPECT_FALSE(read("func(T) {};").succeeded());
    EXPECT_FALSE(read("var t: func(x: T = 1): R;").succeeded());
    EXPECT_FALSE(read("var t: func[T: type](): R;").succeeded());
  }

  // Only var with a plain name can omit its initializer; const and destructuring require one.
  TEST_F(ParserTest, VariableForms)
  {
    const auto Result = read("var a; var b: T; const c = 1; var (x,) = v; var [head, tail...] = xs;");
    ASSERT_TRUE(Result.succeeded());
    EXPECT_EQ(cast<VarDecl>(declaration(Result))->form(), VarDeclForm::Uninitialized);
    EXPECT_TRUE(cast<VarDecl>(declaration(Result, 2))->constant());
    EXPECT_TRUE(isa<TupleBindingPattern>(cast<VarDecl>(declaration(Result, 3))->binding()));
    const auto *Array = cast<ArrayBindingPattern>(cast<VarDecl>(declaration(Result, 4))->binding());
    ASSERT_TRUE(Array->rest());
    EXPECT_EQ(Array->rest()->Name.Text, "tail");
    for (const auto *Source : {"const x;", "var _;", "var (x) = v;", "var () = v;", "var [x,] = v;", "var [xs..., x] = v;", "var 1 = v;", "var a | b = v;"})
    {
      SCOPED_TRACE(Source);
      EXPECT_FALSE(read(Source).succeeded());
    }
  }

  // Field tails retain none, typed, initializer-only and payload forms without accepting parameter defaults.
  TEST_F(ParserTest, FieldForms)
  {
    const auto Result = read("field A; field B: T = 1; field C = 2; field D(x: T, y: U) = 3;");
    ASSERT_TRUE(Result.succeeded());
    EXPECT_EQ(cast<FieldDecl>(declaration(Result))->tail(), FieldTailKind::None);
    EXPECT_EQ(cast<FieldDecl>(declaration(Result, 1))->tail(), FieldTailKind::Typed);
    EXPECT_EQ(cast<FieldDecl>(declaration(Result, 2))->tail(), FieldTailKind::InitializerOnly);
    const auto *Payload = cast<FieldDecl>(declaration(Result, 3));
    EXPECT_EQ(Payload->tail(), FieldTailKind::Payload);
    EXPECT_EQ(Payload->payload().size(), 2U);
    for (const auto *Source : {"field A();", "field A(x: T = 1);", "field A(x: T...);"})
    {
      EXPECT_FALSE(read(Source).succeeded()) << Source;
    }
  }

  // Class forward declarations and complete aggregate bodies preserve bases, implements and ordered comptime members.
  TEST_F(ParserTest, AggregateDeclarations)
  {
    const auto Result = read("class Forward[T: type]; class C[T: type]: Base, implements I { field x: T; comptime if (ready) { field y; } func f(): T; }; enum E { field A; }; interface I { func f(): T; };");
    ASSERT_TRUE(Result.succeeded());
    EXPECT_EQ(cast<ClassDecl>(declaration(Result))->form(), AggregateForm::Forward);
    const auto *Class = cast<ClassDecl>(declaration(Result, 1));
    ASSERT_EQ(Class->bases().size(), 2U);
    EXPECT_FALSE(Class->bases()[0].implements());
    EXPECT_TRUE(Class->bases()[1].implements());
    EXPECT_EQ(Class->body()->statements().size(), 3U);
    EXPECT_TRUE(isa<ComptimeStmt>(Class->body()->statements()[1]));
    EXPECT_TRUE(isa<EnumDecl>(declaration(Result, 2)));
    EXPECT_TRUE(isa<InterfaceDecl>(declaration(Result, 3)));
    EXPECT_FALSE(read("enum E;").succeeded());
    EXPECT_FALSE(read("interface I;").succeeded());
  }

  // Attribute suffixes and qualified paths are retained, while array/lambda expressions are not mistaken for declarations.
  TEST_F(ParserTest, AttributesAndArrays)
  {
    const auto Result = read("[ns.flag, call(x = 1, ys...), value = do { yield 3; }] var x = 1; [a,b]; [a]; func(x: T) {};");
    ASSERT_TRUE(Result.succeeded());
    const auto Attributes = cast<VarDecl>(declaration(Result))->attributes();
    ASSERT_EQ(Attributes.size(), 3U);
    EXPECT_EQ(Attributes[0].path().size(), 2U);
    EXPECT_EQ(Attributes[0].suffix(), AttributeSuffix::None);
    EXPECT_EQ(Attributes[1].suffix(), AttributeSuffix::Arguments);
    EXPECT_EQ(Attributes[2].suffix(), AttributeSuffix::Value);
    EXPECT_TRUE(isa<ArrayExpr>(expression(Result, 1)));
    const auto Prefix = read("[a] func(x: T) {};");
    EXPECT_FALSE(Prefix.succeeded());
    EXPECT_TRUE(isa<SimpleStmt>(Prefix.Unit->root()->statements()[0]));
  }

  // Missing attribute brackets recover as declarations and long attribute prefixes are not artificially capped.
  TEST_F(ParserTest, AttributeRecoveryAndLongPrefix)
  {
    const auto Missing = read("[tag var x = ; var kept = 2;");
    EXPECT_FALSE(Missing.succeeded());
    ASSERT_GE(Missing.Unit->root()->statements().size(), 2U);
    EXPECT_TRUE(isa<VarDecl>(declaration(Missing)));
    std::string Source = "[tag(";
    for (int Index = 0; Index < 300; ++Index)
    {
      Source += Index == 0 ? "x" : ",x";
    }
    Source += ")] var x = 1;";
    EXPECT_TRUE(read(Source).succeeded());
  }

  // Each list family enforces its own emptiness, singleton and trailing-comma rules.
  TEST_F(ParserTest, DeclarationListRules)
  {
    for (const auto *Source : {"func f[](): T;", "func f[T: type,](): T;", "func f(x: T,): T;", "func f(x: T = 1...): T;", "func f(T): T;", "func(T,): T;", "[a,] var x;", "[] var x;"})
    {
      SCOPED_TRACE(Source);
      EXPECT_FALSE(read(Source).succeeded());
    }
  }
} // namespace ink::parser::test
