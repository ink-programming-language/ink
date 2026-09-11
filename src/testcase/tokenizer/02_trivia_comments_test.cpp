#include "grammar_test_support.h"

namespace ink::tokenizer
{
  namespace
  {
    using namespace grammar_test;
    using core::DiagnosticKind;

    // Verifies the default stream discards leading, internal and trailing trivia while retaining source offsets.
    TEST(TriviaCommentsTest, DefaultStreamDiscardsEveryTriviaKind)
    {
      const TokenizedBuffer File = tokenize(" \t\r\n/* outer /* inner */ end */ let// line\rvalue \n");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"let", "value"});
      EXPECT_GT(File.tokens()[0].Span.Start, 0U);
      EXPECT_GT(File.tokens()[1].Span.Start, File.tokens()[0].Span.End);
    }

    // Verifies explicit full-fidelity mode partitions whitespace, comments and syntax without losing bytes.
    TEST(TriviaCommentsTest, PreservedTriviaSharesTheOrderedTokenList)
    {
      const TokenizedBuffer File = tokenize(" \t\r\n/* c */a// d\rb", preservingTrivia());
      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 8U);
      EXPECT_EQ(File.tokens()[0].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(File.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(File.tokens()[2].Kind, TokenKind::BlockComment);
      EXPECT_EQ(File.tokens()[4].Kind, TokenKind::LineComment);
      EXPECT_EQ(File.tokens()[5].Kind, TokenKind::LineBreak);
      expectStream(File, true);
    }

    // Verifies each CR or LF terminates a line comment, and CRLF forms one following line break.
    TEST(TriviaCommentsTest, LineCommentsExcludeTheirPhysicalTerminator)
    {
      for (const std::string &Ending : {std::string("\r"), std::string("\n"), std::string("\r\n")})
      {
        const TokenizedBuffer File = tokenize("// text" + Ending + "after", preservingTrivia());
        ASSERT_TRUE(File.succeeded());
        ASSERT_EQ(File.tokens().size(), 4U);
        EXPECT_EQ(File.raw(File.tokens()[0]), "// text");
        EXPECT_EQ(File.raw(File.tokens()[1]), Ending);
        EXPECT_EQ(File.lineStarts(), (std::vector<std::size_t>{0, 7 + Ending.size()}));
        expectStream(File, true);
      }
    }

    // Verifies a line comment may end at EOF without an implicit newline or an unterminated-comment diagnostic.
    TEST(TriviaCommentsTest, LineCommentMayEndAtEof)
    {
      const TokenizedBuffer File = tokenize("// no ending", preservingTrivia());
      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2U);
      EXPECT_EQ(File.tokens()[0].Kind, TokenKind::LineComment);
      expectStream(File, true);
    }

    // Verifies comment contents include NUL, BOM, control characters and Unicode line separator scalars.
    TEST(TriviaCommentsTest, ArbitraryScalarsRemainCommentText)
    {
      const std::string Body = bytes({0, 1, 0x7F}) + utf8(u8"\uFEFF\u2028\u2029");
      for (const auto &Delimiters : {std::pair<std::string, std::string>{"//", ""}, {"/*", "*/"}})
      {
        const TokenizedBuffer File = tokenize(Delimiters.first + Body + Delimiters.second, preservingTrivia());
        EXPECT_TRUE(File.succeeded());
        EXPECT_EQ(File.lineStarts(), (std::vector<std::size_t>{0}));
        expectStream(File, true);
      }
    }

    // Verifies nested block comments are one lexical token and do not reinterpret embedded quotes.
    TEST(TriviaCommentsTest, NestedBlockCommentIsOneToken)
    {
      const std::string Source = "/* \" /* nested */ // outer */after";
      const TokenizedBuffer File = tokenize(Source, preservingTrivia());
      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 3U);
      EXPECT_EQ(File.tokens()[0].Kind, TokenKind::BlockComment);
      EXPECT_EQ(File.raw(File.tokens()[1]), "after");
      expectStream(File, true);
    }

    // Verifies zero means unlimited comment nesting and deep input does not require recursive scanner calls.
    TEST(TriviaCommentsTest, DefaultCommentDepthAcceptsDeepNesting)
    {
      std::string Source;
      for (std::size_t Index = 0; Index < 12000; ++Index)
      {
        Source += "/*";
      }
      for (std::size_t Index = 0; Index < 12000; ++Index)
      {
        Source += "*/";
      }
      const TokenizedBuffer File = tokenize(Source, preservingTrivia());
      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2U);
      expectStream(File, true);
    }

    // Verifies configured comment depth is inclusive and exceeding it reports failure without losing the suffix.
    TEST(TriviaCommentsTest, ConfiguredDepthLimitRecoversAfterClosingComment)
    {
      const TokenizedBuffer Exact = tokenize("/* /* */ */", preservingTrivia(2));
      const TokenizedBuffer Exceeded = tokenize("/* /* /* */ */ */ after", preservingTrivia(2));
      EXPECT_TRUE(Exact.succeeded());
      EXPECT_FALSE(Exceeded.succeeded());
      EXPECT_TRUE(hasDiagnostic(Exceeded, DiagnosticKind::BlockCommentNestingLimit));
      EXPECT_EQ(Exceeded.raw(Exceeded.tokens()[Exceeded.tokens().size() - 2]), "after");
      expectStream(Exact, true);
      expectStream(Exceeded, true);
    }

    // Verifies unterminated nesting spans the outer opening to EOF and exposes a useful lexical diagnostic.
    TEST(TriviaCommentsTest, UnterminatedNestedCommentRetainsTheWholeErrorSpan)
    {
      const std::string Source = "/* outer /* inner";
      const TokenizedBuffer File = tokenize(Source);
      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2U);
      EXPECT_EQ(File.tokens()[0].Kind, TokenKind::UnterminatedBlockComment);
      EXPECT_EQ(File.tokens()[0].Span, (core::SourceRange{0, Source.size()}));
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnterminatedBlockComment));
      expectStream(File);
    }

    // Verifies depth exhaustion and missing closing delimiters remain separate actionable diagnostics.
    TEST(TriviaCommentsTest, DepthLimitDoesNotHideUnterminatedComment)
    {
      const TokenizedBuffer File = tokenize("/* /* unclosed", preservingTrivia(1));
      EXPECT_FALSE(File.succeeded());
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::BlockCommentNestingLimit));
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnterminatedBlockComment));
      expectStream(File, true);
    }

    // Verifies comments separate tokens and cannot synthesize a longer compound operator across trivia.
    TEST(TriviaCommentsTest, CommentBoundariesPreventOperatorJoining)
    {
      const TokenizedBuffer File = tokenize("a/*gap*/b < /*gap*/ = / /*gap*/ = : //gap\n :");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"a", "b", "<", "=", "/", "=", ":", ":"});
    }

    // Verifies closing-comment punctuation outside a comment is ordinary multiplication and division syntax.
    TEST(TriviaCommentsTest, StrayCommentCloserIsTwoSymbols)
    {
      const TokenizedBuffer File = tokenize("*/");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"*", "/"});
    }
  } // namespace
} // namespace ink::tokenizer