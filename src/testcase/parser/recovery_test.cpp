#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ink::parser
{
  namespace
  {
    // Verifies missing punctuation is reported through structured diagnostics at stable real-token or EOF anchors.
    TEST(ParserRecoveryTest, DiagnosesMissingPunctuationAtStableAnchors)
    {
      struct MissingSyntaxCase
      {
          const char *Source;
          const char *Expected;
          std::size_t Anchor;
      };
      const std::vector<MissingSyntaxCase> Cases = {
          {"Value", ";", 5},
          {"call(value;", ")", 10},
          {"let Value: int32 = 1", ";", 20},
      };
      for (const MissingSyntaxCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Source);
        const ParsedFile File = test::parseSource(TestCase.Source);
        EXPECT_FALSE(File.succeeded());
        EXPECT_TRUE(test::hasDiagnostic(File, core::DiagnosticKind::ExpectedToken));
        EXPECT_TRUE(test::hasExpectedDiagnostic(File, TestCase.Expected, TestCase.Anchor));
        EXPECT_TRUE(hasFlag(File.ast().node(File.ast().root()).Flags, AstNodeFlags::HasMissing));
        test::expectAstIntegrity(File);
      }
    }

    // Verifies rejected syntax creates explicit error nodes and parsing resumes at a subsequent declaration.
    TEST(ParserRecoveryTest, PreservesUnexpectedTokensAndContinues)
    {
      const ParsedFile File = test::parseSource("} ; let After: int32 = 1;");
      EXPECT_FALSE(File.succeeded());
      EXPECT_TRUE(test::hasKind(File, AstKind::Error));
      EXPECT_TRUE(test::hasKind(File, AstKind::BindingDeclaration));
      EXPECT_TRUE(hasFlag(File.ast().node(File.ast().root()).Flags, AstNodeFlags::HasError));
      test::expectAstIntegrity(File);
    }

    // Verifies trailing, leading, and repeated commas fail in every ordinary list family while recovery retains a valid AST.
    TEST(ParserRecoveryTest, RejectsMalformedCommaLists)
    {
      const std::vector<std::string> Sources = {
          "f(1,);",
          "f(,1);",
          "f(1,,2);",
          "F::[T,];",
          "F::[,T];",
          "F::[T,,U];",
          "[1,];",
          "(1, 2,);",
          "let (A, B,): T = V;",
          "func f(A: T,) -> void;",
          "func f[T: type,]() -> void;",
          "let F: type = func(T,) -> void;",
          "class C implements T, {}",
          "interface I extends T, {}",
      };
      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const ParsedFile File = test::parseSource(Source);
        EXPECT_FALSE(File.succeeded());
        EXPECT_FALSE(test::testDiagnostics(File).empty());
        test::expectAstIntegrity(File);
      }
    }

    // Verifies EOF in an unfinished construct is incomplete interactively and a complete failed parse in batch mode.
    TEST(ParserInteractiveTest, DistinguishesIncompleteInputFromBatchSyntaxFailure)
    {
      const std::vector<std::string> Sources = {
          "func pending(",
          "func pending() ->",
          "class Pending {",
          "if (Ready",
          "call(value",
          "let Value: T =",
          "comptime",
          "F::[T",
      };
      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const ParsedFile Batch = test::parseSource(Source);
        const ParsedFile Interactive = test::parseSource(Source, ParserOptions{ParseMode::Interactive});
        EXPECT_FALSE(Batch.succeeded());
        EXPECT_FALSE(Interactive.succeeded());
        EXPECT_EQ(Batch.completeness(), ParseCompleteness::Complete);
        EXPECT_EQ(Interactive.completeness(), ParseCompleteness::Incomplete);
        test::expectAstIntegrity(Batch);
        test::expectAstIntegrity(Interactive);
      }
    }

    // Verifies an explicit unexpected token makes an interactive failure definitive instead of requesting more input.
    TEST(ParserInteractiveTest, ExplicitConflictsRemainComplete)
    {
      for (const std::string Source : {"}", "Value++;", "A < B < C;", "let X: T = ;"})
      {
        SCOPED_TRACE(Source);
        const ParsedFile File = test::parseSource(Source, ParserOptions{ParseMode::Interactive});
        EXPECT_FALSE(File.succeeded());
        EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
        test::expectAstIntegrity(File);
      }
    }

    // Verifies successful input remains complete when the parser is used in interactive mode.
    TEST(ParserInteractiveTest, CompleteInputRemainsCompleteInInteractiveMode)
    {
      const ParsedFile File = test::parseSource("func f() -> void { return; }", ParserOptions{ParseMode::Interactive});
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
      test::expectAstIntegrity(File);
    }

    // Verifies recovery diagnoses the complete atomic compound token rather than a partial byte of its spelling.
    TEST(ParserRecoveryTest, RecordsCompoundDiagnosticSpan)
    {
      const ParsedFile File = test::parseSource("A < B <<= C;");
      ASSERT_FALSE(File.succeeded());
      const auto &Diagnostics = test::testDiagnostics(File);
      EXPECT_TRUE(std::any_of(Diagnostics.begin(), Diagnostics.end(), [](const core::Diagnostic &Diagnostic)
                              {
                                return Diagnostic.Span == core::SourceRange{6, 9};
                              }));
      test::expectAstIntegrity(File);
    }

    // Verifies comments beside a missing delimiter do not prevent recovery at the following binding declaration.
    TEST(ParserRecoveryTest, PreservesSynchronizationTrivia)
    {
      const ParsedFile File = test::parseSource("f(Value /* gap */ ;\nlet After: int32 = 1;");
      EXPECT_FALSE(File.succeeded());
      EXPECT_TRUE(test::hasKind(File, AstKind::BindingDeclaration));
      EXPECT_TRUE(test::hasDiagnostic(File, core::DiagnosticKind::ExpectedToken));
      test::expectAstIntegrity(File);
    }

    // Verifies lexical failures are rejected before AST construction.
    TEST(ParserApiTest, RejectsLexicallyFailedTokenBuffers)
    {
      tokenizer::TokenizedBuffer LexedFile = tokenizer::tokenize("@");
      ASSERT_FALSE(LexedFile.succeeded());
      const ParsedFile File = ink::parser::parse(std::move(LexedFile));
      EXPECT_FALSE(File.succeeded());
      EXPECT_TRUE(File.ast().empty());
      EXPECT_EQ(File.ast().root(), InvalidAstNodeId);
    }

    // Verifies one Parser instance accepts independent files without retaining declarations or diagnostics from earlier inputs.
    TEST(ParserApiTest, ParserInstanceIsReusableAcrossFiles)
    {
      core::CompilationContext Compilation;
      core::FrontendContext Context(Compilation);
      const Parser Reusable(Context);
      const ParsedFile First = Reusable.parse(tokenizer::tokenize(Context, "let First: int32 = 1;"));
      const ParsedFile Second = Reusable.parse(tokenizer::tokenize(Context, "func Second() -> void { return; }"));
      ASSERT_TRUE(First.succeeded());
      ASSERT_TRUE(Second.succeeded());
      EXPECT_TRUE(test::hasKind(First, AstKind::BindingDeclaration));
      EXPECT_FALSE(test::hasKind(First, AstKind::FunctionDeclaration));
      EXPECT_TRUE(test::hasKind(Second, AstKind::FunctionDeclaration));
      EXPECT_FALSE(test::hasKind(Second, AstKind::BindingDeclaration));
      test::expectAstIntegrity(First);
      test::expectAstIntegrity(Second);
    }

    // Verifies default filtered tokens retain their registered source identity and are not retokenized to restore trivia.
    TEST(ParserApiTest, AcceptsDefaultFilteredTokensWithoutRestoringTrivia)
    {
      core::CompilationContext Compilation;
      core::FrontendContext Context(Compilation);
      const std::string Source = " /* leading */\rlet Value: int32 = 1; // trailing\n";
      tokenizer::TokenizedBuffer LexedFile = tokenizer::tokenize(Context, Source);
      ASSERT_TRUE(LexedFile.succeeded());
      EXPECT_FALSE(std::any_of(LexedFile.tokens().begin(), LexedFile.tokens().end(), [](const tokenizer::Token &Token)
                               {
                                 return Token.isTrivia();
                               }));
      const core::SourceId Id = LexedFile.sourceId();
      const ParsedFile File = ink::parser::parse(Context, std::move(LexedFile));
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.lexedFile().sourceId(), Id);
      EXPECT_TRUE(File.lexedFile().isRegisteredWith(Compilation.sourceManager()));
      EXPECT_EQ(File.lexedFile().source(), Source);
      EXPECT_FALSE(std::any_of(File.lexedFile().tokens().begin(), File.lexedFile().tokens().end(), [](const tokenizer::Token &Token)
                               {
                                 return Token.isTrivia();
                               }));
      test::expectAstIntegrity(File);
    }

    // Verifies token buffers owned by a different compilation context cannot produce a seemingly valid AST.
    TEST(ParserApiTest, RejectsTokenBufferFromAnotherCompilationContext)
    {
      core::CompilationContext FirstCompilation;
      core::CompilationContext SecondCompilation;
      core::FrontendContext FirstContext(FirstCompilation);
      core::FrontendContext SecondContext(SecondCompilation);
      tokenizer::TokenizedBuffer LexedFile = tokenizer::tokenize(FirstContext, "let Value: int32 = 1;");
      ASSERT_TRUE(LexedFile.succeeded());
      const ParsedFile File = ink::parser::parse(SecondContext, std::move(LexedFile));
      EXPECT_FALSE(File.succeeded());
      EXPECT_TRUE(File.ast().empty());
      EXPECT_EQ(File.ast().root(), InvalidAstNodeId);
    }

    // Verifies parser diagnostics retain registered SourceId and in-range source offsets.
    TEST(ParserApiTest, PublishesDiagnosticsThroughFrontendContext)
    {
      core::CompilationContext Compilation;
      core::FrontendContext Context(Compilation);
      core::CollectingDiagnosticConsumer Diagnostics;
      Compilation.diagnosticEngine().addConsumer(Diagnostics);
      tokenizer::TokenizedBuffer LexedFile = tokenizer::tokenize(Context, "let Value: int32 = ;");
      const core::SourceId Id = LexedFile.sourceId();
      const ParsedFile File = parse(Context, std::move(LexedFile));
      ASSERT_FALSE(File.succeeded());
      ASSERT_FALSE(Diagnostics.diagnostics().empty());
      for (const core::Diagnostic &Diagnostic : Diagnostics.diagnostics())
      {
        EXPECT_EQ(Diagnostic.Source, Id);
        EXPECT_LE(Diagnostic.Span.Start, Diagnostic.Span.End);
        EXPECT_LE(Diagnostic.Span.End, File.lexedFile().source().size());
      }
    }

    // Verifies deterministic recovery, bounded ranges, and valid AST ownership for arbitrary lexically valid token streams.
    TEST(ParserRecoveryTest, AlwaysAdvancesAndPreservesArbitraryTokenStreams)
    {
      constexpr const char *Spellings[] = {
          "Name",
          "Other",
          "_",
          "1",
          "true",
          "null",
          "func",
          "class",
          "interface",
          "enum",
          "let",
          "var",
          "const",
          "if",
          "else",
          "while",
          "for",
          "in",
          "return",
          "comptime",
          "public",
          "private",
          "class_field",
          "enum_field",
          "type",
          "int32",
          "ptr",
          "ref",
          "(",
          ")",
          "[",
          "]",
          "{",
          "}",
          ";",
          ",",
          ".",
          ":",
          "=",
          "+",
          "-",
          "*",
          "&",
          "<",
          ">",
          "::",
          "->",
          ">>=",
          "?",
          "...",
      };
      constexpr std::size_t CaseCount = 128;
      constexpr std::size_t TokensPerCase = 96;
      std::uint32_t State = 0x4B1D5A77U;
      for (std::size_t CaseIndex = 0; CaseIndex < CaseCount; ++CaseIndex)
      {
        std::string Source;
        for (std::size_t TokenIndex = 0; TokenIndex < TokensPerCase; ++TokenIndex)
        {
          State = State * 1664525U + 1013904223U;
          if (!Source.empty())
          {
            Source.push_back(' ');
          }
          Source += Spellings[State % (sizeof(Spellings) / sizeof(Spellings[0]))];
        }
        SCOPED_TRACE(CaseIndex);
        const ParsedFile First = test::parseSource(Source);
        const ParsedFile Second = test::parseSource(Source);
        EXPECT_EQ(First.completeness(), ParseCompleteness::Complete);
        EXPECT_EQ(test::astSnapshot(First), test::astSnapshot(Second));
        EXPECT_TRUE(test::diagnosticsEqual(First, Second));
        test::expectAstIntegrity(First);
      }
    }
  } // namespace
} // namespace ink::parser
