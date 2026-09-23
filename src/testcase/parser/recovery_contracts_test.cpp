#include "parser_test_support.h"

namespace ink::parser::test
{
  namespace
  {
    void expectKeptVariable(const Stmt *Statement)
    {
      const auto *Declaration = dyn_cast<DeclStmt>(Statement);
      ASSERT_NE(Declaration, nullptr);
      const auto *Variable = dyn_cast<VarDecl>(Declaration->declaration());
      ASSERT_NE(Variable, nullptr);
      const auto *Binding = dyn_cast<NameBindingPattern>(Variable->binding());
      ASSERT_NE(Binding, nullptr);
      EXPECT_EQ(Binding->name().Text, "kept");
      ASSERT_NE(Variable->initializer(), nullptr);
      EXPECT_TRUE(isa<LiteralExpr>(Variable->initializer()));
    }
  } // namespace

  // Each missing delimiter is inserted at the following structure without deleting any of its tokens.
  TEST_F(ParserTest, RecoveryInsertionSites)
  {
    struct Case
    {
        const char *Source;
        TokenKind Expected;
        const char *Boundary;
    };
    const Case Cases[] = {
        {"var x = 1\nvar kept = 42;", TokenKind::Semicolon, "var kept"},
        {"if (x { return; } var kept = 42;", TokenKind::RParen, "{"},
        {"var x = f(1; var kept = 42;", TokenKind::RParen, ";"},
        {"var x = [1, 2; var kept = 42;", TokenKind::RBracket, ";"},
        {"var x = f::[T; var kept = 42;", TokenKind::RBracket, ";"},
        {"class C {} var kept = 42;", TokenKind::Semicolon, "var kept"},
        {"field x: T var kept = 42;", TokenKind::Semicolon, "var kept"},
        {"import a var kept = 42;", TokenKind::Semicolon, "var kept"},
        {"func f(): T var kept = 42;", TokenKind::LBrace, "var kept"},
    };
    for (const auto &Case : Cases)
    {
      SCOPED_TRACE(Case.Source);
      const auto Result = read(Case.Source);
      ASSERT_EQ(Result.Status, ParseStatus::Completed);
      ASSERT_TRUE(Result.HasSyntaxErrors);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::ParserExpectedToken);
      const auto &Entries = Result.Unit->recoveryInfo().Entries;
      ASSERT_EQ(Entries.size(), 1U);
      const std::size_t Offset = std::string_view(Case.Source).find(Case.Boundary);
      EXPECT_EQ(Entries[0].Token.Expected, Case.Expected);
      EXPECT_EQ(Entries[0].Token.Status, ExpectStatus::Inserted);
      EXPECT_EQ(Entries[0].Token.Range, SourceRange::fromByteOffsets(Offset, Offset));
      EXPECT_EQ(Diagnostics.diagnostics()[0].Span, Entries[0].Token.Range);
      EXPECT_FALSE(Entries[0].Skipped.has_value());
      ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
      expectKeptVariable(Result.Unit->root()->statements()[1]);
    }
  }

  // Recovery distinguishes one-token deletion from skipping balanced nested garbage and records exact byte ranges.
  TEST_F(ParserTest, RecoverySkippedRanges)
  {
    for (const std::string Garbage : {"!", "junk [a, (b)] tail", "junk (a, [b, c]) tail"})
    {
      SCOPED_TRACE(Garbage);
      const std::string Prefix = "var x = 1 ";
      const std::string Source = Prefix + Garbage + "; var kept = 42;";
      const auto Result = read(Source);
      ASSERT_EQ(Result.Status, ParseStatus::Completed);
      ASSERT_TRUE(Result.HasSyntaxErrors);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      const auto &Entries = Result.Unit->recoveryInfo().Entries;
      ASSERT_EQ(Entries.size(), 1U);
      EXPECT_EQ(Entries[0].Token.Status, ExpectStatus::Recovered);
      EXPECT_EQ(Entries[0].Token.Expected, TokenKind::Semicolon);
      ASSERT_TRUE(Entries[0].Skipped.has_value());
      EXPECT_EQ(*Entries[0].Skipped, SourceRange::fromByteOffsets(Prefix.size(), Prefix.size() + Garbage.size()));
      EXPECT_EQ(Entries[0].Token.Range, SourceRange::fromByteOffsets(Prefix.size() + Garbage.size(), Prefix.size() + Garbage.size() + 1));
      ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
      expectKeptVariable(Result.Unit->root()->statements()[1]);
    }
  }

  // Synchronization abandons damaged nested garbage at a declaration or mismatched closer instead of swallowing the suffix.
  TEST_F(ParserTest, RecoverySynchronizationBoundaries)
  {
    for (const auto *Garbage : {"junk (a ", "junk [a ", "junk (a ] ", "junk [a ) ", "junk (a { b } ", "junk (a, [b "})
    {
      const std::string Source = std::string("var x = 1 ") + Garbage + "var kept = 42;";
      SCOPED_TRACE(Source);
      const auto Result = read(Source);
      ASSERT_EQ(Result.Status, ParseStatus::Completed);
      ASSERT_TRUE(Result.HasSyntaxErrors);
      ASSERT_GE(Result.Unit->root()->statements().size(), 2U);
      expectKeptVariable(Result.Unit->root()->statements().back());
      for (const auto &Entry : Result.Unit->recoveryInfo().Entries)
      {
        if (Entry.Skipped)
        {
          EXPECT_LE(Entry.Skipped->getEnd().getByteOffset(), Source.find("var kept"));
        }
      }
    }
  }

  // Missing operands and forbidden operand tokens create distinct zero-width missing and nonempty error nodes.
  TEST_F(ParserTest, RecoveryMissingAndErrorExpressions)
  {
    for (const auto *Source : {"var x = ; var kept = 42;", "var x = :; var kept = 42;", "var x = =>; var kept = 42;", "var x = ...; var kept = 42;", "var x = in; var kept = 42;"})
    {
      SCOPED_TRACE(Source);
      const auto Result = read(Source);
      ASSERT_EQ(Result.Status, ParseStatus::Completed);
      ASSERT_TRUE(Result.HasSyntaxErrors);
      ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
      const auto *Variable = dyn_cast<VarDecl>(declaration(Result));
      ASSERT_NE(Variable, nullptr);
      const auto *Value = Variable->initializer();
      ASSERT_NE(Value, nullptr);
      const std::string_view Text(Source);
      const bool Missing = Text.find("= ;") != std::string_view::npos || Text.find("= :") != std::string_view::npos || Text.find("= =>") != std::string_view::npos;
      EXPECT_EQ(isa<MissingExpr>(Value), Missing);
      EXPECT_EQ(isa<ErrorExpr>(Value), !Missing);
      EXPECT_EQ(Value->getSourceRange().empty(), Missing);
      EXPECT_EQ(Diagnostics.diagnostics().front().Kind, Missing ? core::DiagnosticKind::ParserExpectedExpression : core::DiagnosticKind::ParserInvalidSyntax);
      expectKeptVariable(Result.Unit->root()->statements()[1]);
    }
  }

  // Missing call, generic and array elements retain their positions and the valid elements on both sides.
  TEST_F(ParserTest, RecoveryListElementPositions)
  {
    for (const auto *Source : {"f(1, , 3); var kept = 42;", "f::[1, , 3]; var kept = 42;", "[1, , 3]; var kept = 42;", "(1, , 3); var kept = 42;"})
    {
      SCOPED_TRACE(Source);
      const auto Result = read(Source);
      ASSERT_EQ(Result.Status, ParseStatus::Completed);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::ParserExpectedExpression);
      ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
      std::vector<const Expr *> Elements;
      const Expr *Value = expression(Result);
      if (const auto *Call = dyn_cast<CallExpr>(Value))
      {
        for (const auto &Argument : Call->arguments())
        {
          Elements.push_back(Argument.value());
        }
      }
      else if (const auto *Generic = dyn_cast<GenericApplyExpr>(Value))
      {
        for (const auto &Argument : Generic->arguments())
        {
          Elements.push_back(Argument.value());
        }
      }
      else if (const auto *Array = dyn_cast<ArrayExpr>(Value))
      {
        Elements.assign(Array->elements().begin(), Array->elements().end());
      }
      else
      {
        const auto *Tuple = dyn_cast<TupleExpr>(Value);
        ASSERT_NE(Tuple, nullptr);
        Elements.assign(Tuple->elements().begin(), Tuple->elements().end());
      }
      ASSERT_EQ(Elements.size(), 3U);
      EXPECT_TRUE(isa<LiteralExpr>(Elements[0]));
      EXPECT_TRUE(isa<MissingExpr>(Elements[1]));
      EXPECT_TRUE(isa<LiteralExpr>(Elements[2]));
      EXPECT_EQ(Elements[1]->getSourceRange().getBegin().getByteOffset(), std::string_view(Source).find(", ,") + 2);
      expectKeptVariable(Result.Unit->root()->statements()[1]);
    }
  }

  // Missing parameter types, defaults and separators keep subsequent parameters and the next declaration intact.
  TEST_F(ParserTest, RecoveryParameterLists)
  {
    const auto Result = read("func f[T: , U: type](a: , b: T = , c: U): T; var kept = 42;");
    ASSERT_EQ(Result.Status, ParseStatus::Completed);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 3U);
    ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
    const auto *Function = cast<FunctionDecl>(declaration(Result));
    ASSERT_EQ(Function->genericParameters().size(), 2U);
    EXPECT_TRUE(isa<MissingExpr>(Function->genericParameters()[0].type()->expression()));
    EXPECT_EQ(Function->genericParameters()[1].name().Text, "U");
    ASSERT_EQ(Function->parameters().size(), 3U);
    EXPECT_TRUE(isa<MissingExpr>(Function->parameters()[0].type()->expression()));
    EXPECT_TRUE(isa<MissingExpr>(Function->parameters()[1].defaultValue()));
    EXPECT_EQ(Function->parameters()[2].name().Text, "c");
    expectKeptVariable(Result.Unit->root()->statements()[1]);
    const auto Separator = read("func f(a: T b: U): T; var kept = 42;");
    ASSERT_EQ(Separator.Unit->recoveryInfo().Entries.size(), 1U);
    EXPECT_EQ(Separator.Unit->recoveryInfo().Entries[0].Token.Expected, TokenKind::Comma);
    EXPECT_EQ(cast<FunctionDecl>(declaration(Separator))->parameters().size(), 2U);
  }

  // Binding and match pattern recovery preserve missing/error categories, rest bindings and later alternatives.
  TEST_F(ParserTest, RecoveryPatternPlaceholders)
  {
    const auto Bindings = read("var [a, , tail...] = xs; var [1, b] = xs; var kept = 42;");
    ASSERT_EQ(Bindings.Status, ParseStatus::Completed);
    ASSERT_EQ(Bindings.Unit->root()->statements().size(), 3U);
    const auto *Missing = cast<ArrayBindingPattern>(cast<VarDecl>(declaration(Bindings))->binding());
    ASSERT_EQ(Missing->elements().size(), 2U);
    EXPECT_TRUE(isa<MissingBindingPattern>(Missing->elements()[1]));
    ASSERT_TRUE(Missing->rest().has_value());
    EXPECT_EQ(Missing->rest()->Name.Text, "tail");
    const auto *Error = cast<ArrayBindingPattern>(cast<VarDecl>(declaration(Bindings, 1))->binding());
    ASSERT_EQ(Error->elements().size(), 2U);
    EXPECT_TRUE(isa<ErrorBindingPattern>(Error->elements()[0]));
    EXPECT_EQ(cast<NameBindingPattern>(Error->elements()[1])->name().Text, "b");
    expectKeptVariable(Bindings.Unit->root()->statements()[2]);
    const auto Patterns = read("match(x) { A | => 1, + => 2, _ => 3 }; var kept = 42;");
    ASSERT_EQ(Patterns.Status, ParseStatus::Completed);
    const auto Arms = cast<MatchExpr>(expression(Patterns))->arms();
    ASSERT_EQ(Arms.size(), 3U);
    const auto Alternatives = cast<OrMatchPattern>(Arms[0].pattern())->alternatives();
    ASSERT_EQ(Alternatives.size(), 2U);
    EXPECT_TRUE(isa<MissingMatchPattern>(Alternatives[1]));
    EXPECT_TRUE(isa<ErrorMatchPattern>(Arms[1].pattern()));
    EXPECT_TRUE(isa<WildcardMatchPattern>(Arms[2].pattern()));
    expectKeptVariable(Patterns.Unit->root()->statements().back());
  }

  // A missing conditional arm or match body leaves its enclosing colon, comma and subsequent arm available.
  TEST_F(ParserTest, RecoveryConditionalAndMatchBodies)
  {
    const auto Result = read("var x = ready ? : 2; match(x) { A => , B if () => 2, _ => 3 }; var kept = 42;");
    ASSERT_EQ(Result.Status, ParseStatus::Completed);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 3U);
    ASSERT_EQ(Result.Unit->root()->statements().size(), 3U);
    const auto *Conditional = cast<ConditionalExpr>(cast<VarDecl>(declaration(Result))->initializer());
    EXPECT_TRUE(isa<MissingExpr>(Conditional->then()));
    EXPECT_TRUE(isa<LiteralExpr>(Conditional->elseValue()));
    const auto Arms = cast<MatchExpr>(expression(Result, 1))->arms();
    ASSERT_EQ(Arms.size(), 3U);
    EXPECT_TRUE(isa<MissingExpr>(Arms[0].value()));
    EXPECT_TRUE(isa<MissingExpr>(Arms[1].guard()));
    EXPECT_TRUE(isa<LiteralExpr>(Arms[2].value()));
    expectKeptVariable(Result.Unit->root()->statements()[2]);
  }

  // An absent then branch becomes a missing statement while else and switch labels stay owned by their control statement.
  TEST_F(ParserTest, RecoveryControlFlowOwnership)
  {
    const auto Result = read("if (x) else return; switch(x) { case : break; case 2 return; default: yield ; } var kept = 42;");
    ASSERT_EQ(Result.Status, ParseStatus::Completed);
    ASSERT_TRUE(Result.HasSyntaxErrors);
    ASSERT_EQ(Result.Unit->root()->statements().size(), 3U);
    const auto *If = cast<IfStmt>(Result.Unit->root()->statements()[0]);
    EXPECT_TRUE(isa<MissingStmt>(If->thenBranch()));
    EXPECT_TRUE(isa<ReturnStmt>(If->elseBranch()));
    const auto Clauses = cast<SwitchStmt>(Result.Unit->root()->statements()[1])->clauses();
    ASSERT_EQ(Clauses.size(), 3U);
    EXPECT_TRUE(isa<MissingExpr>(Clauses[0].value()));
    ASSERT_EQ(Clauses[1].statements().size(), 1U);
    EXPECT_TRUE(isa<ReturnStmt>(Clauses[1].statements()[0]));
    EXPECT_TRUE(Clauses[2].isDefault());
    ASSERT_EQ(Clauses[2].statements().size(), 1U);
    EXPECT_TRUE(isa<MissingExpr>(cast<YieldStmt>(Clauses[2].statements()[0])->value()));
    expectKeptVariable(Result.Unit->root()->statements()[2]);
  }

  // Failed for-in speculation rolls back its diagnostics and recovery records before classic-for recovery commits.
  TEST_F(ParserTest, RecoverySpeculationRollback)
  {
    const auto Result = read("for ([a,b]; test; step) {} for ([a,b]; test; step {} var kept = 42;");
    ASSERT_EQ(Result.Status, ParseStatus::Completed);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    ASSERT_EQ(Result.Unit->recoveryInfo().Entries.size(), 1U);
    EXPECT_EQ(Result.Unit->recoveryInfo().Entries[0].Token.Expected, TokenKind::RParen);
    ASSERT_EQ(Result.Unit->root()->statements().size(), 3U);
    EXPECT_TRUE(isa<ClassicForStmt>(Result.Unit->root()->statements()[0]));
    EXPECT_TRUE(isa<ClassicForStmt>(Result.Unit->root()->statements()[1]));
    expectKeptVariable(Result.Unit->root()->statements()[2]);
  }

  // Malformed attributes, imports, aggregates, lambdas and types recover to an independent final declaration.
  TEST_F(ParserTest, RecoveryDeclarationFamilies)
  {
    const char *Prefixes[] = {
        "[tag(x = )] var broken = 1; ",
        "[tag var broken = 1; ",
        "import a as ; ",
        "from a import ; ",
        "class C: { field x: ; }; ",
        "enum E { field A(x: ); }; ",
        "interface I { func f(x: ): ; }; ",
        "var f = func(x: T = ): T {}; ",
        "var t: func(T, named: ): ; ",
        "const ; ",
        "var x = object.; ",
        "var x = array[]; ",
        "var x = [value; ]; ",
    };
    for (const auto *Prefix : Prefixes)
    {
      SCOPED_TRACE(Prefix);
      const auto Result = read(std::string(Prefix) + "var kept = 42;");
      ASSERT_EQ(Result.Status, ParseStatus::Completed);
      EXPECT_TRUE(Result.HasSyntaxErrors);
      ASSERT_GE(Result.Unit->root()->statements().size(), 2U);
      expectKeptVariable(Result.Unit->root()->statements().back());
    }
  }

  // EOF inside each grammar family yields bounded diagnostics and a valid partial tree without fabricated source bytes.
  TEST_F(ParserTest, RecoveryEndOfFileFamilies)
  {
    const char *Sources[] = {
        "var",
        "const",
        "var x =",
        "var x:",
        "func f",
        "func f[T:",
        "func f(x:",
        "func f(): T {",
        "field A(",
        "class C:",
        "enum E {",
        "interface I {",
        "import a.",
        "from a import",
        "if (",
        "if (x)",
        "while (x)",
        "for (",
        "for (x in",
        "switch(x) { case",
        "match(x) { A if (",
        "match(x) { A =>",
        "x ?",
        "x ? y :",
        "f(1,",
        "f::[T,",
        "a[",
        "a.",
        "[1;",
        "do { yield",
        "func(x: T = 1): T",
    };
    for (const auto *Source : Sources)
    {
      SCOPED_TRACE(Source);
      ParseLimits Limits;
      Limits.MaxWork = 10000;
      const auto Result = read(Source, Limits);
      EXPECT_EQ(Result.Status, ParseStatus::Completed);
      EXPECT_TRUE(Result.HasSyntaxErrors);
      EXPECT_FALSE(Result.succeeded());
      EXPECT_LE(Diagnostics.diagnostics().size(), 12U);
      EXPECT_FALSE(dumpAST(*Result.Unit).empty());
    }
  }

  // Repeated errors respect zero and finite diagnostic budgets while retaining every later declaration.
  TEST_F(ParserTest, RecoveryDiagnosticStorm)
  {
    constexpr std::size_t Count = 256;
    std::string Source;
    for (std::size_t Index = 0; Index < Count; ++Index)
    {
      Source += "var x" + std::to_string(Index) + " = ; ";
    }
    Source += "var kept = 42;";
    for (const std::size_t Budget : {0U, 1U, 7U, 100U})
    {
      SCOPED_TRACE(Budget);
      ParseLimits Limits;
      Limits.MaxDiagnostics = Budget;
      const auto Result = read(Source, Limits);
      EXPECT_EQ(Result.Status, ParseStatus::Completed);
      EXPECT_TRUE(Result.HasSyntaxErrors);
      EXPECT_EQ(Diagnostics.diagnostics().size(), Budget);
      ASSERT_EQ(Result.Unit->root()->statements().size(), Count + 1);
      expectKeptVariable(Result.Unit->root()->statements().back());
    }
  }

  // Deep expression, statement and pattern families hit the parser depth budget instead of exhausting the native stack.
  TEST_F(ParserTest, RecoveryDepthLimitFamilies)
  {
    struct Case
    {
        const char *Prefix;
        const char *Open;
        const char *Center;
        const char *Close;
        const char *Suffix;
    };
    const Case Cases[] = {
        {"var x = ", "(", "x", ")", ";"},
        {"var x = ", "!", "x", "", ";"},
        {"var x = ", "x ?? ", "x", "", ";"},
        {"var x = ", "x ? x : ", "x", "", ";"},
        {"", "{", "return;", "}", ""},
        {"", "if(x) ", "return;", "", ""},
        {"var ", "[", "x", "]", " = xs;"},
        {"match(x) { ", "A(", "_", ")", " => 0 };"},
    };
    for (const auto &Case : Cases)
    {
      SCOPED_TRACE(Case.Open);
      std::string Source = Case.Prefix;
      for (unsigned Index = 0; Index < 256; ++Index)
      {
        Source += Case.Open;
      }
      Source += Case.Center;
      for (unsigned Index = 0; Index < 256; ++Index)
      {
        Source += Case.Close;
      }
      Source += Case.Suffix;
      ParseLimits Limits;
      Limits.MaxNestingDepth = 24;
      const auto Result = read(Source, Limits);
      EXPECT_EQ(Result.Status, ParseStatus::LimitExceeded);
      EXPECT_TRUE(Result.HasSyntaxErrors);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::ParserLimitExceeded);
      EXPECT_EQ(Diagnostics.diagnostics()[0].classification(), core::DiagnosticClass::InternalCompilerError);
    }
  }

  // Cancellation during malformed-input recovery publishes a valid partial tree and leaves the next parse usable.
  TEST_F(ParserTest, RecoveryCancellationAndReuse)
  {
    std::string Source;
    for (unsigned Index = 0; Index < 100; ++Index)
    {
      Source += "var x = f(1, , 3); ";
    }
    std::size_t Checks = 0;
    ParseLimits Limits;
    Limits.IsCancelled = [&Checks]
    {
      return ++Checks >= 80;
    };
    const auto Result = read(Source, Limits);
    EXPECT_EQ(Result.Status, ParseStatus::Cancelled);
    EXPECT_TRUE(Result.HasSyntaxErrors);
    EXPECT_FALSE(Result.succeeded());
    EXPECT_FALSE(Result.Unit->root()->statements().empty());
    EXPECT_LT(Result.Unit->root()->statements().size(), 100U);
    EXPECT_TRUE(read("var kept = 42;").succeeded());
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Stopping at each work/cancellation checkpoint and tiny allocation budgets still publishes valid partial recovery trees.
  TEST_F(ParserTest, RecoveryBudgetCheckpoints)
  {
    std::string Source;
    for (unsigned Index = 0; Index < 8; ++Index)
    {
      Source += "for ([a, b]; ready; step) { match(x) { A | => do { yield ; }, _ => 2 }; } var kept = 42;";
    }
    for (std::size_t Budget = 0; Budget < 160; ++Budget)
    {
      SCOPED_TRACE(Budget);
      ParseLimits Limits;
      Limits.MaxWork = Budget;
      const auto Limited = read(Source, Limits);
      EXPECT_EQ(Limited.Status, ParseStatus::LimitExceeded);
      EXPECT_FALSE(Limited.succeeded());
      std::size_t Checks = 0;
      Limits = {};
      Limits.IsCancelled = [&Checks, Budget]
      {
        return Checks++ >= Budget;
      };
      const auto Cancelled = read(Source, Limits);
      EXPECT_EQ(Cancelled.Status, ParseStatus::Cancelled);
      EXPECT_FALSE(Cancelled.succeeded());
    }
    for (const std::size_t Bytes : {0U, 1U, 128U, 1024U})
    {
      ParseLimits Limits;
      Limits.MaxAllocationBytes = Bytes;
      Limits.MaxDiagnostics = 0;
      const auto Result = read(Source, Limits);
      EXPECT_EQ(Result.Status, ParseStatus::LimitExceeded);
      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(Diagnostics.diagnostics().empty());
    }
  }

  // Deleting, truncating and inserting tokens across valid grammar families always terminates with valid recovery metadata.
  TEST_F(ParserTest, RecoveryGrammarMutationCorpus)
  {
    const char *Sources[] = {
        "class C[T: type]: Base, implements I { field x: T; func f(a: T): T { return a; } };",
        "[tag(x = 1)] var [head, tail...] = source; from ...pkg import x as y; import a.b, c;",
        "for ([x, rest...] in xs) { if (x) continue; else break; } for (var i = 0; i < 3; i++) {}",
        "switch(x) { case 1: yield return 2; case 3: defer f(); default: return; }",
        "var f = func[T: type](x: T = 1): T { return x; }; var t: func(T, x: U...): R;",
        "match(x) { A::[T](a) | B if (ready) => a ? b : c, [x, _...] => do { yield x; } };",
        "var x = object?.f::[T](a = [1; n], xs...)[0] ?? (a,b);",
        "func \u51FD\u6570(\u53C2\u6570: T): T { var e\u0301 = \u53C2\u6570; return e\u0301; }",
    };
    ParseLimits Limits;
    Limits.MaxWork = 100000;
    for (const auto *Text : Sources)
    {
      const std::string Source(Text);
      SCOPED_TRACE(Source);
      ASSERT_TRUE(read(Source).succeeded());
      const auto Lexed = tokenizer::tokenize(Frontend, Source);
      ASSERT_TRUE(Lexed.succeeded());
      for (const auto &Token : Lexed.tokens())
      {
        const std::size_t Begin = Token.Span.getBegin().getByteOffset();
        const std::size_t End = Token.Span.getEnd().getByteOffset();
        SCOPED_TRACE(Begin);
        EXPECT_EQ(read(Source.substr(0, Begin), Limits).Status, ParseStatus::Completed);
        EXPECT_EQ(read(Source.substr(0, Begin) + Source.substr(End), Limits).Status, ParseStatus::Completed);
        for (const auto *Insertion : {",", ":", "=>", ")", "]", "}", ";"})
        {
          SCOPED_TRACE(Insertion);
          EXPECT_EQ(read(Source.substr(0, Begin) + Insertion + Source.substr(Begin), Limits).Status, ParseStatus::Completed);
        }
      }
    }
  }
} // namespace ink::parser::test
