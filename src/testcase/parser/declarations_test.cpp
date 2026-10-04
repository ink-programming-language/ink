#include "parser_test_support.h"
namespace ink::parser::test
{
  // Explicit public/private visibility is retained separately from the default on declarations and nested definitions.
  TEST_F(ParserTest, DeclarationVisibilityModifiers)
  {
    const auto Result = read("func plain(): i32 { return 0; } public func exposed(): i32 { private func local(): i32 { return 1; } return local(); } private func hidden(): i32 { return 2; } public var value = 3; private class Hidden; public import \"C\" func native(): i32;");
    ASSERT_TRUE(Result.succeeded());
    EXPECT_EQ(declaration(Result, 0)->visibility(), DeclarationVisibility::Default);
    EXPECT_EQ(declaration(Result, 1)->visibility(), DeclarationVisibility::Public);
    EXPECT_EQ(declaration(Result, 2)->visibility(), DeclarationVisibility::Private);
    EXPECT_EQ(declaration(Result, 3)->visibility(), DeclarationVisibility::Public);
    EXPECT_EQ(declaration(Result, 4)->visibility(), DeclarationVisibility::Private);
    EXPECT_EQ(declaration(Result, 5)->visibility(), DeclarationVisibility::Public);
    const auto *Outer = cast<FunctionDecl>(declaration(Result, 1));
    const auto *Local = cast<DeclStmt>(Outer->body()->statements()[0])->declaration();
    EXPECT_EQ(Local->visibility(), DeclarationVisibility::Private);
    EXPECT_NE(dumpAST(*Result.Unit).find("Visibility=Public"), std::string::npos);
    EXPECT_NE(dumpAST(*Result.Unit).find("Visibility=Private"), std::string::npos);
  }

  // Visibility composes with attribute groups and comptime prefixes without losing declaration or statement metadata.
  TEST_F(ParserTest, VisibilityAttributesAndComptime)
  {
    const char *Sources[] = {
        "comptime [tag] public func f(): i32 { return 1; }",
        "comptime public [tag] func f(): i32 { return 1; }",
        "public comptime [tag] func f(): i32 { return 1; }",
        "public [tag] comptime func f(): i32 { return 1; }",
        "[tag] public comptime func f(): i32 { return 1; }",
    };
    for (const char *Source : Sources)
    {
      SCOPED_TRACE(Source);
      const auto Result = read(Source);
      ASSERT_TRUE(Result.succeeded());
      const auto *Statement = cast<DeclStmt>(Result.Unit->root()->statements()[0]);
      const auto *Function = cast<FunctionDecl>(Statement->declaration());
      EXPECT_EQ(Function->visibility(), DeclarationVisibility::Public);
      EXPECT_EQ(Function->attributes().size(), 1U);
      EXPECT_TRUE(Function->isComptime());
      EXPECT_TRUE(Statement->isComptime());
      EXPECT_EQ(Statement->getSourceRange(), SourceRange::fromByteOffsets(0, std::string_view(Source).size()));
    }
  }

  // Duplicate and contradictory visibility keywords diagnose the later modifier while preserving both surrounding functions.
  TEST_F(ParserTest, RejectsDuplicateAndConflictingVisibility)
  {
    struct Case
    {
        const char *Modifiers;
        core::DiagnosticKind Diagnostic;
        DeclarationVisibility First;
    };
    const Case Cases[] = {
        {"public public", core::DiagnosticKind::ParserDuplicateVisibilityModifier, DeclarationVisibility::Public},
        {"private private", core::DiagnosticKind::ParserDuplicateVisibilityModifier, DeclarationVisibility::Private},
        {"public private", core::DiagnosticKind::ParserConflictingVisibilityModifiers, DeclarationVisibility::Public},
        {"private public", core::DiagnosticKind::ParserConflictingVisibilityModifiers, DeclarationVisibility::Private},
        {"public [tag] private", core::DiagnosticKind::ParserConflictingVisibilityModifiers, DeclarationVisibility::Public},
    };
    for (const auto &Case : Cases)
    {
      SCOPED_TRACE(Case.Modifiers);
      const std::string Source = std::string(Case.Modifiers) + " func broken(): i32 { return 0; } public func kept(): i32 { return 1; }";
      const auto Result = read(Source);
      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, Case.Diagnostic);
      ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
      EXPECT_EQ(declaration(Result)->visibility(), Case.First);
      EXPECT_EQ(cast<FunctionDecl>(declaration(Result, 1))->name().Text, "kept");
      EXPECT_EQ(declaration(Result, 1)->visibility(), DeclarationVisibility::Public);
    }
  }

  // Declaration visibility participates in structural verification even for manually constructed or corrupted trees.
  TEST_F(ParserTest, VerifierRejectsInvalidDeclarationVisibility)
  {
    auto Result = read("private func f(): i32 { return 0; }");
    ASSERT_TRUE(Result.succeeded());
    auto *Declaration = cast<DeclStmt>(Result.Unit->root()->statements()[0])->declaration();
    Declaration->setVisibility(static_cast<DeclarationVisibility>(99));
    EXPECT_FALSE(verifyAST(Result.Unit->root(), Result.Unit->input().lexedFile().source().size()));
  }

  // Type positions reject ungrouped binary operators while allowing complete expressions in parentheses.
  TEST_F(ParserTest, TypeGrammar)
  {
    EXPECT_FALSE(read("var x: A + B;").succeeded());
    const auto Result = read("var x: (A + B); var y: comptime *T::[U][n]; var z: do { yield T; };");
    ASSERT_TRUE(Result.succeeded());
    EXPECT_TRUE(isa<ParenExpr>(cast<VarDecl>(declaration(Result))->type()->expression()));
    const auto *ComptimeType = cast<VarDecl>(declaration(Result, 1))->type();
    ASSERT_TRUE(isa<UnaryExpr>(ComptimeType->expression()));
    EXPECT_TRUE(ComptimeType->expression()->isComptime());
    EXPECT_FALSE(ComptimeType->isComptime());
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
    EXPECT_EQ(Function->linkage(), nullptr);
    EXPECT_EQ(Forward->linkage(), nullptr);
    EXPECT_EQ(Function->nativeSymbolKind(), core::FunctionBinding::Local);
    EXPECT_EQ(Forward->nativeSymbolKind(), core::FunctionBinding::Local);
  }

  // Native import preserves arbitrary decoded linkage names and the literal's original spelling and range.
  TEST_F(ParserTest, NativeImportLinkageNames)
  {
    struct Case
    {
        const char *Spelling;
        std::string_view Decoded;
    };
    const Case Cases[] = {
        {"\"C\"", "C"},
        {"\"C++\"", "C++"},
        {"\"vendor.custom\"", "vendor.custom"},
        {"\"\\x43\"", "C"},
        {"r\"custom\\name\"", "custom\\name"},
        {"\"\"\"custom\nname\"\"\"", "custom\nname"},
        {"\"\"", ""},
        {"\"C\\0other\"", std::string_view("C\0other", 7)},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Spelling);
      const std::string Source = std::string("import ") + Entry.Spelling + " func f(msg: *u8): i32;";
      const auto Result = read(Source);
      ASSERT_TRUE(Result.succeeded());
      const auto *Function = cast<FunctionDecl>(declaration(Result));
      ASSERT_TRUE(isa<LiteralExpr>(Function->linkage()));
      EXPECT_EQ(Function->nativeSymbolKind(), core::FunctionBinding::Import);
      const auto *Linkage = cast<LiteralExpr>(Function->linkage());
      EXPECT_EQ(Linkage->literalKind(), TokenKind::StringLiteral);
      EXPECT_EQ(Result.Unit->input().spelling(Linkage->token()), Entry.Spelling);
      EXPECT_EQ(std::get<tokenizer::StringInfo>(Result.Unit->input().token(Linkage->token()).Payload).Decoded, Entry.Decoded);
      EXPECT_EQ(Linkage->getSourceRange().getBegin().getByteOffset(), 7U);
      EXPECT_EQ(Function->getSourceRange(), SourceRange::fromByteOffsets(0, Source.size()));
      EXPECT_EQ(Function->bodyKind(), FunctionBodyKind::DeclarationOnly);
      EXPECT_EQ(Function->body(), nullptr);
      EXPECT_EQ(Function->parameters().size(), 1U);
    }
  }

  // Attributes and comptime prefixes retain export function definitions, and walkers visit linkage before the signature.
  TEST_F(ParserTest, NativeExportDefinitionsAndTraversal)
  {
    const auto Result = read("comptime [tag] export \"other\" func f[T: type](x: T): T { return x; }");
    ASSERT_TRUE(Result.succeeded());
    const auto *Statement = cast<DeclStmt>(Result.Unit->root()->statements()[0]);
    const auto *Function = cast<FunctionDecl>(Statement->declaration());
    EXPECT_TRUE(Statement->isComptime());
    EXPECT_TRUE(Function->isComptime());
    EXPECT_EQ(Function->attributes().size(), 1U);
    EXPECT_EQ(Function->genericParameters().size(), 1U);
    EXPECT_EQ(Function->nativeSymbolKind(), core::FunctionBinding::Export);
    EXPECT_EQ(Function->bodyKind(), FunctionBodyKind::Definition);
    EXPECT_NE(Function->body(), nullptr);
    std::vector<const ASTNodeBase *> Visited;
    ASTWalker{}.walk(Function, [&](const ASTNodeBase *Node)
    {
      Visited.push_back(Node);
      return WalkAction::Continue;
    });
    ASSERT_GE(Visited.size(), 3U);
    EXPECT_EQ(Visited[1], Function->linkage());
    EXPECT_EQ(Visited[2], Function->genericParameters()[0].type());
    EXPECT_NE(dumpAST(*Result.Unit).find("Linkage=LiteralExpr@"), std::string::npos);
    EXPECT_NE(dumpAST(*Result.Unit).find("NativeSymbolKind=Export"), std::string::npos);
  }

  // Native import/export remain separate from Ink module imports, declaration visibility and standalone ABI attributes.
  TEST_F(ParserTest, NativeSymbolsAndModuleImports)
  {
    const auto Result = read("import math; from math import add; private import \"C\" func abs(Value: i32): i32; public export \"C\" func sum(A: i32, B: i32): i32 { return A + B; } [abi(\"C\")] private func callback(Value: i32): i32 { return Value; } private export \"C\" func hidden(): void {}");
    ASSERT_TRUE(Result.succeeded());
    ASSERT_EQ(Result.Unit->root()->statements().size(), 6U);
    EXPECT_TRUE(isa<DirectImportStmt>(Result.Unit->root()->statements()[0]));
    EXPECT_TRUE(isa<FromImportStmt>(Result.Unit->root()->statements()[1]));
    const auto *Imported = cast<FunctionDecl>(declaration(Result, 2));
    EXPECT_EQ(Imported->nativeSymbolKind(), core::FunctionBinding::Import);
    EXPECT_EQ(Imported->visibility(), DeclarationVisibility::Private);
    const auto *Exported = cast<FunctionDecl>(declaration(Result, 3));
    EXPECT_EQ(Exported->nativeSymbolKind(), core::FunctionBinding::Export);
    EXPECT_EQ(Exported->visibility(), DeclarationVisibility::Public);
    const auto *Callback = cast<FunctionDecl>(declaration(Result, 4));
    EXPECT_EQ(Callback->nativeSymbolKind(), core::FunctionBinding::Local);
    EXPECT_EQ(Callback->linkage(), nullptr);
    ASSERT_EQ(Callback->attributes().size(), 1U);
    EXPECT_EQ(Callback->attributes()[0].path()[0].Text, "abi");
    const auto *Hidden = cast<FunctionDecl>(declaration(Result, 5));
    EXPECT_EQ(Hidden->nativeSymbolKind(), core::FunctionBinding::Export);
    EXPECT_EQ(Hidden->visibility(), DeclarationVisibility::Private);
  }

  // Removed extern syntax produces diagnostics while its spelling remains available as an ordinary identifier.
  TEST_F(ParserTest, RemovedExternSyntax)
  {
    EXPECT_FALSE(read("extern \"C\" func legacy(): void;").succeeded());
    EXPECT_TRUE(read("func extern(): i32 { return 1; }").succeeded());
  }

  // Malformed import headers report errors and preserve a subsequent complete function declaration.
  TEST_F(ParserTest, NativeImportHeaderRecovery)
  {
    const char *Sources[] = {
        "import func broken(): void;",
        "import 123 func broken(): void;",
        "import \"C\" broken(): void;",
        "import \"C\" func broken();",
        "import \"C\" \"other\" func broken(): void;",
        "import \"C\";",
        "import;",
        "import \"C\" var broken;",
    };
    for (const char *Source : Sources)
    {
      SCOPED_TRACE(Source);
      const auto Result = read(std::string(Source) + " import \"kept\" func kept(): void;");
      ASSERT_FALSE(Result.succeeded());
      ASSERT_GE(Result.Unit->root()->statements().size(), 2U);
      const auto *Last = cast<DeclStmt>(Result.Unit->root()->statements().back());
      ASSERT_TRUE(isa<FunctionDecl>(Last->declaration()));
      EXPECT_EQ(cast<FunctionDecl>(Last->declaration())->name().Text, "kept");
    }
    const auto Missing = read("import func f(): void;");
    ASSERT_FALSE(Missing.succeeded());
    EXPECT_TRUE(isa<MissingExpr>(cast<FunctionDecl>(declaration(Missing))->linkage()));
    EXPECT_FALSE(read("import").succeeded());
    EXPECT_FALSE(read("import \"C\"").succeeded());
  }

  // Export recovery keeps a later export and records a missing ABI string as a missing expression.
  TEST_F(ParserTest, NativeExportHeaderRecovery)
  {
    const char *Sources[] = {
        "export func broken(): void {}",
        "export 123 func broken(): void {}",
        "export \"C\" broken(): void {}",
        "export \"C\";",
        "export;",
        "export \"C\" var broken;",
    };
    for (const char *Source : Sources)
    {
      SCOPED_TRACE(Source);
      const auto Result = read(std::string(Source) + " public export \"C\" func kept(): void {}");
      ASSERT_FALSE(Result.succeeded());
      const auto *Last = cast<DeclStmt>(Result.Unit->root()->statements().back());
      ASSERT_TRUE(isa<FunctionDecl>(Last->declaration()));
      const auto *Function = cast<FunctionDecl>(Last->declaration());
      EXPECT_EQ(Function->name().Text, "kept");
      EXPECT_EQ(Function->nativeSymbolKind(), core::FunctionBinding::Export);
    }
    const auto Missing = read("export func f(): void {}");
    ASSERT_FALSE(Missing.succeeded());
    const auto *Function = cast<FunctionDecl>(declaration(Missing));
    EXPECT_EQ(Function->nativeSymbolKind(), core::FunctionBinding::Export);
    EXPECT_TRUE(isa<MissingExpr>(Function->linkage()));
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

  // Declaration modifiers mark both ordinary declaration layers while initializer-only prefixes leave both layers runtime.
  TEST_F(ParserTest, ComptimeDeclarationFlags)
  {
    struct Case
    {
        const char *Source;
        ASTKind Kind;
    };
    const Case Cases[] = {
        {"comptime var x = 1;", ASTKind::VarDecl},
        {"comptime const x = 1;", ASTKind::VarDecl},
        {"comptime [tag] field X: T;", ASTKind::FieldDecl},
        {"comptime func f(): T;", ASTKind::FunctionDecl},
        {"comptime class C;", ASTKind::ClassDecl},
        {"comptime enum E {};", ASTKind::EnumDecl},
        {"comptime interface I {};", ASTKind::InterfaceDecl},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      const auto Result = read(Entry.Source);
      ASSERT_TRUE(Result.succeeded());
      ASSERT_EQ(Result.Unit->root()->statements().size(), 1U);
      const auto *Statement = cast<DeclStmt>(Result.Unit->root()->statements()[0]);
      const auto *Declaration = Statement->declaration();
      EXPECT_EQ(Declaration->getKind(), Entry.Kind);
      EXPECT_TRUE(Statement->isComptime());
      EXPECT_TRUE(Declaration->isComptime());
      const auto Range = SourceRange::fromByteOffsets(0, std::string_view(Entry.Source).size());
      EXPECT_EQ(Statement->getSourceRange(), Range);
      EXPECT_EQ(Declaration->getSourceRange(), Range);
    }
    const auto Result = read("var x = comptime 1;");
    ASSERT_TRUE(Result.succeeded());
    EXPECT_FALSE(Result.Unit->root()->statements()[0]->isComptime());
    const auto *Variable = cast<VarDecl>(declaration(Result));
    EXPECT_FALSE(Variable->isComptime());
    EXPECT_TRUE(Variable->initializer()->isComptime());
    EXPECT_FALSE(Variable->binding()->isComptime());
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
    EXPECT_TRUE(isa<IfStmt>(Class->body()->statements()[1]));
    EXPECT_TRUE(Class->body()->statements()[1]->isComptime());
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
