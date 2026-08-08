#include "ink/tokenizer/tokenizer.h"

#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace ink::tokenizer
{
  namespace
  {
    using core::Diagnostic;
    using core::DiagnosticKind;

    bool hasTokenKind(const TokenizedBuffer &Buffer, TokenKind Kind)
    {
      for (const Token &TokenEntry : Buffer.tokens())
      {
        if (TokenEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    bool hasDiagnosticKind(const TokenizedBuffer &Buffer, DiagnosticKind Kind)
    {
      for (const Diagnostic &DiagnosticEntry : Buffer.diagnostics())
      {
        if (DiagnosticEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    bool hasErrorToken(const TokenizedBuffer &Buffer)
    {
      for (const Token &TokenEntry : Buffer.tokens())
      {
        if (TokenEntry.isError())
        {
          return true;
        }
      }
      return false;
    }

    void expectFullFidelity(const TokenizedBuffer &Buffer)
    {
      const std::vector<Token> &Tokens = Buffer.tokens();
      ASSERT_FALSE(Tokens.empty());
      EXPECT_EQ(Tokens.back().Kind, TokenKind::EndOfFile);
      EXPECT_EQ(Tokens.back().Span.Start, Buffer.source().size());
      EXPECT_EQ(Tokens.back().Span.End, Buffer.source().size());
      EXPECT_TRUE(Buffer.raw(Tokens.back()).empty());

      std::size_t NextByte = 0;
      std::size_t EofCount = 0;
      std::string Reconstructed;
      for (const Token &TokenEntry : Tokens)
      {
        EXPECT_LE(TokenEntry.Span.Start, TokenEntry.Span.End);
        EXPECT_LE(TokenEntry.Span.End, Buffer.source().size());
        EXPECT_EQ(TokenEntry.isTrivia(), isTrivia(TokenEntry.Kind));
        EXPECT_EQ(TokenEntry.isError(), isError(TokenEntry.Kind));
        if (TokenEntry.Kind == TokenKind::EndOfFile)
        {
          ++EofCount;
          EXPECT_EQ(&TokenEntry, &Tokens.back());
          continue;
        }
        EXPECT_EQ(TokenEntry.Span.Start, NextByte);
        EXPECT_EQ(Buffer.raw(TokenEntry).size(), TokenEntry.Span.size());
        Reconstructed.append(Buffer.raw(TokenEntry));
        NextByte = TokenEntry.Span.End;
      }
      EXPECT_EQ(EofCount, 1U);
      EXPECT_EQ(NextByte, Buffer.source().size());
      EXPECT_EQ(Reconstructed, Buffer.source());
    }

    // Tests that trivia and significant tokens share one source-ordered token stream.
    TEST(TriviaCommentsTest, TriviaTokensShareTheSingleOrderedTokenList)
    {
      const std::string Source = utf8(u8"\uFEFF \tlet\r\n//comment\n/**/");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 8U);
      const std::vector<TokenKind> Expected = {
          TokenKind::Utf8Bom,
          TokenKind::SpacesAndTabs,
          TokenKind::Keyword,
          TokenKind::LineBreak,
          TokenKind::LineComment,
          TokenKind::LineBreak,
          TokenKind::BlockComment,
          TokenKind::EndOfFile,
      };
      for (std::size_t Index = 0; Index < Expected.size(); ++Index)
      {
        EXPECT_EQ(Buffer.tokens()[Index].Kind, Expected[Index]);
      }
      for (std::size_t Index = 0; Index + 1 < Buffer.tokens().size(); ++Index)
      {
        EXPECT_EQ(Buffer.tokens()[Index].isTrivia(), Index != 2U);
      }
      expectFullFidelity(Buffer);
    }

    // Tests maximal grouping of adjacent ASCII spaces and tabs.
    TEST(TriviaCommentsTest, AdjacentSpacesAndTabsFormOneMaximalTriviaToken)
    {
      const TokenizedBuffer Buffer = tokenize(" \t  \t");

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), " \t  \t");
      EXPECT_TRUE(Buffer.tokens()[0].isTrivia());
      expectFullFidelity(Buffer);
    }

    // Tests that line-comment tokens exclude both LF and CRLF terminators.
    TEST(TriviaCommentsTest, LineCommentExcludesLfOrCrLfTerminator)
    {
      struct LineEndingCase
      {
        const char *Source;
        const char *LineBreak;
      };
      const std::vector<LineEndingCase> Cases = {
          {"// comment\n", "\n"},
          {"// comment\r\n", "\r\n"},
      };

      for (const LineEndingCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.LineBreak);
        const TokenizedBuffer Buffer = tokenize(TestCase.Source);
        ASSERT_TRUE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 3U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "// comment");
        EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::LineBreak);
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), TestCase.LineBreak);
        expectFullFidelity(Buffer);
      }
    }

    // Tests that a line comment can terminate directly at end of file.
    TEST(TriviaCommentsTest, LineCommentMayEndAtEof)
    {
      const TokenizedBuffer Buffer = tokenize("// no final line break");

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "// no final line break");
      expectFullFidelity(Buffer);
    }

    // Tests that Unicode newline lookalikes remain text inside a line comment.
    TEST(TriviaCommentsTest, UnicodeNewlineLookalikesDoNotEndLineComments)
    {
      const std::string Source = utf8(u8"//a\u0085b\u2028c\u2029d\nx");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 4U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), utf8(u8"//a\u0085b\u2028c\u2029d"));
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "\n");
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::Identifier);
      expectFullFidelity(Buffer);
    }

    // Tests that a BOM inside a line comment is preserved as comment text.
    TEST(TriviaCommentsTest, NonInitialByteOrderMarkIsPreservedAsCommentText)
    {
      const std::string Source = utf8(u8"//a\uFEFFb");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
      expectFullFidelity(Buffer);
    }

    // Tests nested block-comment scanning as one token across internal line breaks.
    TEST(TriviaCommentsTest, NestedBlockCommentIsOneTokenIncludingInternalLineBreaks)
    {
      const std::string Source = "/* outer\n/* inner */ \" // still outer */tail";
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::BlockComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "/* outer\n/* inner */ \" // still outer */");
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "tail");
      EXPECT_FALSE(hasTokenKind(Buffer, TokenKind::LineBreak));
      expectFullFidelity(Buffer);
    }

    // Tests acceptance of an empty block comment.
    TEST(TriviaCommentsTest, EmptyBlockCommentIsLegal)
    {
      const TokenizedBuffer Buffer = tokenize("/**/");

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::BlockComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "/**/");
      expectFullFidelity(Buffer);
    }

    // Tests that comment delimiters have meaning only in the active lexical state.
    TEST(TriviaCommentsTest, CommentDelimitersOnlyActInTheActiveLexicalState)
    {
      const std::string Source = "// /* not a block */ \"not a string\"\n\"https://example.com/*text*/\" /* // nested text */";
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 6U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::StringLiteral);
      EXPECT_EQ(Buffer.tokens()[3].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Buffer.tokens()[4].Kind, TokenKind::BlockComment);
      expectFullFidelity(Buffer);
    }

    // Tests that trivia establishes boundaries between adjacent identifiers and symbols.
    TEST(TriviaCommentsTest, TriviaForcesIdentifierAndSymbolBoundaries)
    {
      const std::string Source = "first/* comment */second +/* comment */+";
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      const std::vector<TokenKind> Expected = {
          TokenKind::Identifier,
          TokenKind::BlockComment,
          TokenKind::Identifier,
          TokenKind::SpacesAndTabs,
          TokenKind::Symbol,
          TokenKind::BlockComment,
          TokenKind::Symbol,
          TokenKind::EndOfFile,
      };
      ASSERT_EQ(Buffer.tokens().size(), Expected.size());
      for (std::size_t Index = 0; Index < Expected.size(); ++Index)
      {
        EXPECT_EQ(Buffer.tokens()[Index].Kind, Expected[Index]);
      }
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "first");
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[2]), "second");
      EXPECT_EQ(std::get<char>(Buffer.tokens()[4].Payload), '+');
      EXPECT_EQ(std::get<char>(Buffer.tokens()[6].Payload), '+');
      expectFullFidelity(Buffer);
    }

    // Tests that documentation-like spellings remain ordinary comment tokens.
    TEST(TriviaCommentsTest, DocumentationLikeCommentsRemainOrdinaryComments)
    {
      const std::string Source = "/// line\n//! inner\n/** block */ /*! inner */";
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      const std::vector<TokenKind> Expected = {
          TokenKind::LineComment,
          TokenKind::LineBreak,
          TokenKind::LineComment,
          TokenKind::LineBreak,
          TokenKind::BlockComment,
          TokenKind::SpacesAndTabs,
          TokenKind::BlockComment,
          TokenKind::EndOfFile,
      };
      ASSERT_EQ(Buffer.tokens().size(), Expected.size());
      for (std::size_t Index = 0; Index < Expected.size(); ++Index)
      {
        EXPECT_EQ(Buffer.tokens()[Index].Kind, Expected[Index]);
      }
      expectFullFidelity(Buffer);
    }

    // Tests that a closing block-comment delimiter outside a comment becomes two symbols.
    TEST(TriviaCommentsTest, StarSlashOutsideBlockCommentIsTwoSymbols)
    {
      const TokenizedBuffer Buffer = tokenize("*/");

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Buffer.tokens()[0].Payload), '*');
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Buffer.tokens()[1].Payload), '/');
      expectFullFidelity(Buffer);
    }

    // Tests unterminated nested-comment coverage and detailed nesting diagnostics.
    TEST(TriviaCommentsTest, UnterminatedNestedBlockCommentCoversFromOutermostStartToEof)
    {
      const std::string Source = "/* outer /* inner";
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::UnterminatedBlockComment);
      EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Buffer.tokens()[0].Span.End, Source.size());
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
      EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::UnterminatedBlockComment));
      bool DetailedDiagnosticFound = false;
      for (const Diagnostic &DiagnosticEntry : Buffer.diagnostics())
      {
        if (DiagnosticEntry.Kind == DiagnosticKind::UnterminatedBlockComment && DiagnosticEntry.Message.find("outermost opening byte: 0") != std::string::npos && DiagnosticEntry.Message.find("remaining nesting depth: 2") != std::string::npos && DiagnosticEntry.Message.find("most recent unclosed opening byte: 9") != std::string::npos)
        {
          DetailedDiagnosticFound = true;
        }
      }
      EXPECT_TRUE(DetailedDiagnosticFound);
      expectFullFidelity(Buffer);
    }

    // Tests the combined diagnostics when an over-limit nested block comment also reaches end of file before closing.
    TEST(TriviaCommentsTest, OverLimitUnterminatedBlockCommentReportsBothExactDiagnostics)
    {
      const std::string Source = "/* outer /* inner /* too deep";
      const std::size_t OverLimitOpening = Source.rfind("/*");
      const TokenizedBuffer Buffer = tokenize(Source, TokenizerOptions{2});

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::UnterminatedBlockComment);
      EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Buffer.tokens()[0].Span.End, Source.size());
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::EndOfFile);

      ASSERT_EQ(Buffer.diagnostics().size(), 2U);
      EXPECT_EQ(Buffer.diagnostics()[0].Kind, DiagnosticKind::BlockCommentNestingLimit);
      EXPECT_EQ(Buffer.diagnostics()[0].Span.Start, OverLimitOpening);
      EXPECT_EQ(Buffer.diagnostics()[0].Span.End, OverLimitOpening + 2);
      EXPECT_EQ(Buffer.diagnostics()[0].Message, "block comment nesting limit exceeded");
      EXPECT_EQ(Buffer.diagnostics()[1].Kind, DiagnosticKind::UnterminatedBlockComment);
      EXPECT_EQ(Buffer.diagnostics()[1].Span.Start, 0U);
      EXPECT_EQ(Buffer.diagnostics()[1].Span.End, 2U);
      EXPECT_EQ(Buffer.diagnostics()[1].Message, "block comment is not terminated; outermost opening byte: 0; remaining nesting depth: 3; most recent unclosed opening byte was not retained after the nesting limit was exceeded");
      expectFullFidelity(Buffer);
    }

    // Tests that a zero block-comment depth limit rejects even a closed empty comment and preserves following source text.
    TEST(TriviaCommentsTest, ZeroBlockCommentDepthLimitRejectsTheOutermostComment)
    {
      const std::string Source = "/**/tail";
      const TokenizedBuffer Buffer = tokenize(Source, TokenizerOptions{0});

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "/**/");
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "tail");
      ASSERT_EQ(Buffer.diagnostics().size(), 1U);
      EXPECT_EQ(Buffer.diagnostics().front().Kind, DiagnosticKind::BlockCommentNestingLimit);
      expectFullFidelity(Buffer);
    }

    // Tests that a block comment nested exactly to the configured depth remains valid trivia.
    TEST(TriviaCommentsTest, ExactBlockCommentDepthLimitIsAccepted)
    {
      const std::string Source = "/* outer /* inner */ outer */tail";
      const TokenizedBuffer Buffer = tokenize(Source, TokenizerOptions{2});

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::BlockComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "/* outer /* inner */ outer */");
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "tail");
      EXPECT_TRUE(Buffer.diagnostics().empty());
      expectFullFidelity(Buffer);
    }

    // Tests that exceeding the configured depth still consumes the closed comment and resumes at following source text.
    TEST(TriviaCommentsTest, ConfiguredBlockCommentNestingLimitProducesAnErrorWithoutLosingBytes)
    {
      const std::string Source = "/* one /* two /* three */ two */ one */tail";
      const TokenizedBuffer Buffer = tokenize(Source, TokenizerOptions{2});

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "/* one /* two /* three */ two */ one */");
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "tail");
      ASSERT_EQ(Buffer.diagnostics().size(), 1U);
      EXPECT_EQ(Buffer.diagnostics().front().Kind, DiagnosticKind::BlockCommentNestingLimit);
      EXPECT_TRUE(hasErrorToken(Buffer));
      expectFullFidelity(Buffer);
    }

    // Tests that a backslash cannot continue a physical source line.
    TEST(TriviaCommentsTest, BackslashBeforeLineBreakDoesNotContinueTheLine)
    {
      const TokenizedBuffer Buffer = tokenize("a\\\nb");

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 5U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "\\");
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Buffer.tokens()[3].Kind, TokenKind::Identifier);
      expectFullFidelity(Buffer);
    }
  } // namespace
} // namespace ink::tokenizer
