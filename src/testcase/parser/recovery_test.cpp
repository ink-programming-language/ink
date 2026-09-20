#include "parser_test_support.h"
#include <random>
namespace ink::parser::test
{
  // The design document's three independent errors retain both variables and both if branches with three diagnostics.
  TEST_F(ParserTest, DesignRecoveryExample)
  {
    const std::string Source = "func test(): int { var a = 1\n var b = foo(2, , 4); if (b > 0 { return b; } else { return 0; } }";
    const auto Result = read(Source);
    EXPECT_EQ(Result.Status, ParseStatus::Completed);
    ASSERT_TRUE(Result.HasSyntaxErrors);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 3U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, core::DiagnosticKind::ParserExpectedToken);
    EXPECT_EQ(Diagnostics.diagnostics()[1].Kind, core::DiagnosticKind::ParserExpectedExpression);
    EXPECT_EQ(Diagnostics.diagnostics()[2].Kind, core::DiagnosticKind::ParserExpectedToken);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Span, SourceRange::fromByteOffsets(Source.find("var b"), Source.find("var b")));
    const auto *Function = cast<FunctionDecl>(declaration(Result));
    const auto Body = Function->body()->statements();
    ASSERT_EQ(Body.size(), 3U);
    const auto *Variable = cast<VarDecl>(cast<DeclStmt>(Body[1])->declaration());
    const auto Arguments = cast<CallExpr>(Variable->initializer())->arguments();
    ASSERT_EQ(Arguments.size(), 3U);
    EXPECT_TRUE(isa<MissingExpr>(Arguments[1].value()));
    const auto *If = cast<IfStmt>(Body[2]);
    ASSERT_NE(If->elseBranch(), nullptr);
    EXPECT_TRUE(isa<ReturnStmt>(cast<BlockStmt>(If->elseBranch())->statements()[0]));
    ASSERT_EQ(Result.Unit->recoveryInfo().Entries.size(), 2U);
    EXPECT_EQ(Result.Unit->recoveryInfo().Entries[0].Token.Status, ExpectStatus::Inserted);
  }

  // Missing punctuation or operands in nested lists never consume an independent following declaration.
  TEST_F(ParserTest, PreserveFollowingDeclarations)
  {
    for (const auto *Prefix : {"var x = ", "var x = foo(1, ", "var x = [1, ", "var x = foo::[T, ", "var x = (a + ", "func f(): T ", "[tag var x = ", "var x = foo(1 + ", "var x = match(a) { _ => "})
    {
      const std::string Source = std::string(Prefix) + "var kept = 42;";
      SCOPED_TRACE(Source);
      const auto Result = read(Source);
      EXPECT_TRUE(Result.HasSyntaxErrors);
      bool Found = false;
      ASTWalker{}.walk(Result.Unit->root(), [&](const ASTNodeBase *Node)
                       {
                         if (auto *Name = dyn_cast<NameBindingPattern>(Node))
                         {
                           Found = Found || Name->name().Text == "kept";
                         }
                         return WalkAction::Continue;
                       });
      EXPECT_TRUE(Found);
    }
  }

  // Recovery protects else and switch labels even when nested delimiters and statement operands are missing.
  TEST_F(ParserTest, PreserveElseAndSwitchLabels)
  {
    const auto Result = read("if (x) { var a = f(1; else { return 0; } switch (x) { case 1: f( default: return; case 2: break; }");
    EXPECT_TRUE(Result.HasSyntaxErrors);
    ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
    const auto *If = cast<IfStmt>(Result.Unit->root()->statements()[0]);
    EXPECT_NE(If->elseBranch(), nullptr);
    const auto *Switch = cast<SwitchStmt>(Result.Unit->root()->statements()[1]);
    ASSERT_EQ(Switch->clauses().size(), 3U);
    EXPECT_TRUE(Switch->clauses()[1].isDefault());
  }

  // Single-token deletion is recorded distinctly from insertion, and unmatched file terminators make progress.
  TEST_F(ParserTest, DeletedTokenAndFileTerminators)
  {
    const auto Result = read("var x = 1 @;");
    EXPECT_FALSE(Result.succeeded());
    const auto Deleted = read("var x = 1 !; var kept = 2;");
    EXPECT_FALSE(Deleted.succeeded());
    ASSERT_EQ(Deleted.Unit->recoveryInfo().Entries.size(), 1U);
    EXPECT_EQ(Deleted.Unit->recoveryInfo().Entries[0].Token.Status, ExpectStatus::Recovered);
    EXPECT_TRUE(Deleted.Unit->recoveryInfo().Entries[0].Skipped.has_value());
    const auto Stray = read("} ) ] ; var kept = 1;");
    EXPECT_TRUE(Stray.HasSyntaxErrors);
    EXPECT_EQ(Stray.Unit->root()->statements().size(), 5U);
  }

  // Missing match arrows and missing list separators insert punctuation while retaining subsequent elements.
  TEST_F(ParserTest, MatchArrowAndListSeparatorRecovery)
  {
    const auto Result = read("match(x) { A 1, B => 2 }; foo(1 2, 3);");
    EXPECT_TRUE(Result.HasSyntaxErrors);
    EXPECT_EQ(cast<MatchExpr>(expression(Result))->arms().size(), 2U);
    EXPECT_EQ(cast<CallExpr>(expression(Result, 1))->arguments().size(), 3U);
  }

  // Completed describes scanning, errors belong to this parse call, and lexical failures are not diagnosed again.
  TEST_F(ParserTest, StatusAndDiagnosticIsolation)
  {
    auto Invalid = tokenizer::tokenize(Frontend, "@");
    const auto Count = Diagnostics.diagnostics().size();
    const auto Lexical = parse(Frontend, std::move(Invalid));
    EXPECT_FALSE(Lexical.succeeded());
    EXPECT_FALSE(Lexical.HasSyntaxErrors);
    EXPECT_EQ(Diagnostics.diagnostics().size(), Count);
    const auto Valid = parse(Frontend, tokenizer::tokenize(Frontend, "var x = 1;"));
    EXPECT_TRUE(Valid.succeeded());
    EXPECT_FALSE(Valid.HasSyntaxErrors);
    const auto Syntax = read("var x = ;");
    EXPECT_EQ(Syntax.Status, ParseStatus::Completed);
    EXPECT_TRUE(Syntax.HasSyntaxErrors);
  }

  // Nesting, total work, allocation and cancellation each produce a valid explicitly incomplete result.
  TEST_F(ParserTest, ResourceLimits)
  {
    ParseLimits Limits;
    Limits.MaxNestingDepth = 12;
    EXPECT_EQ(read(std::string(1000, '(') + "x;", Limits).Status, ParseStatus::LimitExceeded);
    Limits = {};
    Limits.MaxWork = 20;
    EXPECT_EQ(read("for ([x,y,z]; a; b) { var x = 1; }", Limits).Status, ParseStatus::LimitExceeded);
    Limits = {};
    Limits.MaxAllocationBytes = 128;
    EXPECT_EQ(read("a + b + c + d + e + f;", Limits).Status, ParseStatus::LimitExceeded);
    Limits = {};
    Limits.IsCancelled = []
    {
      return true;
    };
    const auto Cancelled = read("var x = 1;", Limits);
    EXPECT_EQ(Cancelled.Status, ParseStatus::Cancelled);
    EXPECT_FALSE(Cancelled.succeeded());
  }

  // Diagnostic suppression does not hide the parse's error status and still permits subsequent progress.
  TEST_F(ParserTest, DiagnosticBudget)
  {
    ParseLimits Limits;
    Limits.MaxDiagnostics = 2;
    const auto Result = read("; ; ; ; var kept = 1;", Limits);
    EXPECT_EQ(Result.Status, ParseStatus::Completed);
    EXPECT_TRUE(Result.HasSyntaxErrors);
    EXPECT_EQ(Diagnostics.diagnostics().size(), 2U);
    EXPECT_EQ(Result.Unit->root()->statements().size(), 5U);
    Limits.MaxDiagnostics = 0;
    EXPECT_TRUE(read(";", Limits).HasSyntaxErrors);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Long left-associative and assignment chains parse, walk and destroy using bounded native stack space.
  TEST_F(ParserTest, LongChains)
  {
    for (const auto *Operator : {" + x", " = x"})
    {
      std::string Source = "x";
      for (int Index = 0; Index < 12000; ++Index)
      {
        Source += Operator;
      }
      Source += ';';
      const auto Result = read(Source);
      ASSERT_TRUE(Result.succeeded());
      std::size_t Count = 0;
      ASTWalker{}.walk(Result.Unit->root(), [&](const ASTNodeBase *)
                       {
                         ++Count;
                         return WalkAction::Continue;
                       });
      EXPECT_GT(Count, 24000U);
    }
  }

  // Every token-boundary truncation, deletion and inserted delimiter terminates with a verifiable recovery tree.
  TEST_F(ParserTest, TokenBoundaryMutations)
  {
    const std::string Source = "[tag(x = 1)] func f[T: type](a: T = 2): T { for ([x, rest...] in xs) { var y = match(x) { A(v) if (v > 0) => v, _ => do { yield 0; } }; } return a; } var kept = 1;";
    const auto Lexed = tokenizer::tokenize(Frontend, Source);
    ASSERT_TRUE(Lexed.succeeded());
    ParseLimits Limits;
    Limits.MaxWork = 100000;
    for (const auto &Token : Lexed.tokens())
    {
      const std::size_t Begin = Token.Span.getBegin().getByteOffset();
      const std::size_t End = Token.Span.getEnd().getByteOffset();
      SCOPED_TRACE(Begin);
      EXPECT_EQ(read(Source.substr(0, Begin), Limits).Status, ParseStatus::Completed);
      EXPECT_EQ(read(Source.substr(0, Begin) + Source.substr(End), Limits).Status, ParseStatus::Completed);
      for (const auto *Inserted : {",", ")", "}", ";"})
      {
        EXPECT_EQ(read(Source.substr(0, Begin) + Inserted + Source.substr(Begin), Limits).Status, ParseStatus::Completed);
      }
    }
  }

  // Truncated field payloads keep their required parameter placeholder without consuming the next declaration.
  TEST_F(ParserTest, TruncatedFieldPayload)
  {
    const auto Result = read("field A( var kept = 1;");
    EXPECT_TRUE(Result.HasSyntaxErrors);
    ASSERT_EQ(Result.Unit->root()->statements().size(), 2U);
    const auto *Field = cast<FieldDecl>(declaration(Result));
    ASSERT_EQ(Field->payload().size(), 1U);
    EXPECT_TRUE(isa<MissingExpr>(Field->payload()[0].type()->expression()));
    EXPECT_EQ(cast<NameBindingPattern>(cast<VarDecl>(declaration(Result, 1))->binding())->name().Text, "kept");
  }

  // Damaged attributes retain their children within the recovered declaration's range.
  TEST_F(ParserTest, DamagedAttributeDeclarationRange)
  {
    const auto Result = read("[tag(x = 1)] } func f(): T;");
    EXPECT_TRUE(Result.HasSyntaxErrors);
    EXPECT_TRUE(verifyAST(Result.Unit->root(), Result.Unit->input().lexedFile().source().size()));
  }

  // Deterministic malformed token streams terminate within budgets without null required children or invalid ranges.
  TEST_F(ParserTest, DeterministicRandomInputs)
  {
    const std::vector<std::string> Tokens = {
        "var",
        "func",
        "if",
        "else",
        "for",
        "in",
        "switch",
        "case",
        "default",
        "match",
        "do",
        "comptime",
        "field",
        "const",
        "return",
        "yield",
        "x",
        "1",
        "(",
        ")",
        "[",
        "]",
        "{",
        "}",
        ",",
        ";",
        ":",
        "=",
        "=>",
        "...",
        "|",
        "+",
        "?",
        "::",
        "import",
        "from",
        "_",
    };
    std::mt19937 Random(0x1A2B3C);
    ParseLimits Limits;
    Limits.MaxWork = 50000;
    Limits.MaxNestingDepth = 48;
    for (unsigned Iteration = 0; Iteration < 600; ++Iteration)
    {
      std::string Source;
      for (unsigned Index = 0, Count = Random() % 80; Index < Count; ++Index)
      {
        Source += Tokens[Random() % Tokens.size()] + ' ';
      }
      SCOPED_TRACE(Source);
      const auto Result = read(Source, Limits);
      EXPECT_NE(Result.Status, ParseStatus::Cancelled);
    }
  }
} // namespace ink::parser::test
