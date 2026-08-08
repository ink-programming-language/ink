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

    bool hasTokenKind(const LexedFile &Lexed, TokenKind Kind)
    {
      for (const Token &TokenEntry : Lexed.tokens())
      {
        if (TokenEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    bool hasDiagnosticKind(const LexedFile &Lexed, DiagnosticKind Kind)
    {
      for (const Diagnostic &DiagnosticEntry : Lexed.diagnostics())
      {
        if (DiagnosticEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    bool hasErrorToken(const LexedFile &Lexed)
    {
      for (const Token &TokenEntry : Lexed.tokens())
      {
        if (TokenEntry.isError())
        {
          return true;
        }
      }
      return false;
    }

    void expectFullFidelity(const LexedFile &Lexed)
    {
      const std::vector<Token> &Tokens = Lexed.tokens();
      ASSERT_FALSE(Tokens.empty());
      EXPECT_EQ(Tokens.back().Kind, TokenKind::EndOfFile);
      EXPECT_EQ(Tokens.back().Span.Start, Lexed.source().size());
      EXPECT_EQ(Tokens.back().Span.End, Lexed.source().size());
      EXPECT_TRUE(Lexed.raw(Tokens.back()).empty());

      std::size_t NextByte = 0;
      std::size_t EofCount = 0;
      std::string Reconstructed;
      for (const Token &TokenEntry : Tokens)
      {
        EXPECT_LE(TokenEntry.Span.Start, TokenEntry.Span.End);
        EXPECT_LE(TokenEntry.Span.End, Lexed.source().size());
        EXPECT_EQ(TokenEntry.isTrivia(), isTrivia(TokenEntry.Kind));
        EXPECT_EQ(TokenEntry.isError(), isError(TokenEntry.Kind));
        if (TokenEntry.Kind == TokenKind::EndOfFile)
        {
          ++EofCount;
          EXPECT_EQ(&TokenEntry, &Tokens.back());
          continue;
        }
        EXPECT_EQ(TokenEntry.Span.Start, NextByte);
        EXPECT_EQ(Lexed.raw(TokenEntry).size(), TokenEntry.Span.size());
        Reconstructed.append(Lexed.raw(TokenEntry));
        NextByte = TokenEntry.Span.End;
      }
      EXPECT_EQ(EofCount, 1U);
      EXPECT_EQ(NextByte, Lexed.source().size());
      EXPECT_EQ(Reconstructed, Lexed.source());
    }

    // Tests that trivia and significant tokens share one source-ordered token stream.
    TEST(TriviaCommentsTest, TriviaTokensShareTheSingleOrderedTokenList)
    {
      const std::string Source = utf8(u8"\uFEFF \tlet\r\n//comment\n/**/");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 8U);
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
        EXPECT_EQ(Lexed.tokens()[Index].Kind, Expected[Index]);
      }
      for (std::size_t Index = 0; Index + 1 < Lexed.tokens().size(); ++Index)
      {
        EXPECT_EQ(Lexed.tokens()[Index].isTrivia(), Index != 2U);
      }
      expectFullFidelity(Lexed);
    }

    // Tests maximal grouping of adjacent ASCII spaces and tabs.
    TEST(TriviaCommentsTest, AdjacentSpacesAndTabsFormOneMaximalTriviaToken)
    {
      const LexedFile Lexed = tokenize(" \t  \t");

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 2U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), " \t  \t");
      EXPECT_TRUE(Lexed.tokens()[0].isTrivia());
      expectFullFidelity(Lexed);
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
        const LexedFile Lexed = tokenize(TestCase.Source);
        ASSERT_TRUE(Lexed.succeeded());
        ASSERT_EQ(Lexed.tokens().size(), 3U);
        EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::LineComment);
        EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), "// comment");
        EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::LineBreak);
        EXPECT_EQ(Lexed.raw(Lexed.tokens()[1]), TestCase.LineBreak);
        expectFullFidelity(Lexed);
      }
    }

    // Tests that a line comment can terminate directly at end of file.
    TEST(TriviaCommentsTest, LineCommentMayEndAtEof)
    {
      const LexedFile Lexed = tokenize("// no final line break");

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 2U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), "// no final line break");
      expectFullFidelity(Lexed);
    }

    // Tests that Unicode newline lookalikes remain text inside a line comment.
    TEST(TriviaCommentsTest, UnicodeNewlineLookalikesDoNotEndLineComments)
    {
      const std::string Source = utf8(u8"//a\u0085b\u2028c\u2029d\nx");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 4U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), utf8(u8"//a\u0085b\u2028c\u2029d"));
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[1]), "\n");
      EXPECT_EQ(Lexed.tokens()[2].Kind, TokenKind::Identifier);
      expectFullFidelity(Lexed);
    }

    // Tests that a BOM inside a line comment is preserved as comment text.
    TEST(TriviaCommentsTest, NonInitialByteOrderMarkIsPreservedAsCommentText)
    {
      const std::string Source = utf8(u8"//a\uFEFFb");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 2U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), Source);
      expectFullFidelity(Lexed);
    }

    // Tests nested block-comment scanning as one token across internal line breaks.
    TEST(TriviaCommentsTest, NestedBlockCommentIsOneTokenIncludingInternalLineBreaks)
    {
      const std::string Source = "/* outer\n/* inner */ \" // still outer */tail";
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 3U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::BlockComment);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), "/* outer\n/* inner */ \" // still outer */");
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[1]), "tail");
      EXPECT_FALSE(hasTokenKind(Lexed, TokenKind::LineBreak));
      expectFullFidelity(Lexed);
    }

    // Tests acceptance of an empty block comment.
    TEST(TriviaCommentsTest, EmptyBlockCommentIsLegal)
    {
      const LexedFile Lexed = tokenize("/**/");

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 2U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::BlockComment);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), "/**/");
      expectFullFidelity(Lexed);
    }

    // Tests that comment delimiters have meaning only in the active lexical state.
    TEST(TriviaCommentsTest, CommentDelimitersOnlyActInTheActiveLexicalState)
    {
      const std::string Source = "// /* not a block */ \"not a string\"\n\"https://example.com/*text*/\" /* // nested text */";
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 6U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Lexed.tokens()[2].Kind, TokenKind::StringLiteral);
      EXPECT_EQ(Lexed.tokens()[3].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Lexed.tokens()[4].Kind, TokenKind::BlockComment);
      expectFullFidelity(Lexed);
    }

    // Tests that trivia establishes boundaries between adjacent identifiers and symbols.
    TEST(TriviaCommentsTest, TriviaForcesIdentifierAndSymbolBoundaries)
    {
      const std::string Source = "first/* comment */second +/* comment */+";
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
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
      ASSERT_EQ(Lexed.tokens().size(), Expected.size());
      for (std::size_t Index = 0; Index < Expected.size(); ++Index)
      {
        EXPECT_EQ(Lexed.tokens()[Index].Kind, Expected[Index]);
      }
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), "first");
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[2]), "second");
      EXPECT_EQ(std::get<char>(Lexed.tokens()[4].Payload), '+');
      EXPECT_EQ(std::get<char>(Lexed.tokens()[6].Payload), '+');
      expectFullFidelity(Lexed);
    }

    // Tests that documentation-like spellings remain ordinary comment tokens.
    TEST(TriviaCommentsTest, DocumentationLikeCommentsRemainOrdinaryComments)
    {
      const std::string Source = "/// line\n//! inner\n/** block */ /*! inner */";
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
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
      ASSERT_EQ(Lexed.tokens().size(), Expected.size());
      for (std::size_t Index = 0; Index < Expected.size(); ++Index)
      {
        EXPECT_EQ(Lexed.tokens()[Index].Kind, Expected[Index]);
      }
      expectFullFidelity(Lexed);
    }

    // Tests that a closing block-comment delimiter outside a comment becomes two symbols.
    TEST(TriviaCommentsTest, StarSlashOutsideBlockCommentIsTwoSymbols)
    {
      const LexedFile Lexed = tokenize("*/");

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 3U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Lexed.tokens()[0].Payload), '*');
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Lexed.tokens()[1].Payload), '/');
      expectFullFidelity(Lexed);
    }

    // Tests unterminated nested-comment coverage and detailed nesting diagnostics.
    TEST(TriviaCommentsTest, UnterminatedNestedBlockCommentCoversFromOutermostStartToEof)
    {
      const std::string Source = "/* outer /* inner";
      const LexedFile Lexed = tokenize(Source);

      ASSERT_FALSE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 2U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::UnterminatedBlockComment);
      EXPECT_EQ(Lexed.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Lexed.tokens()[0].Span.End, Source.size());
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), Source);
      EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::UnterminatedBlockComment));
      bool DetailedDiagnosticFound = false;
      for (const Diagnostic &DiagnosticEntry : Lexed.diagnostics())
      {
        if (DiagnosticEntry.Kind == DiagnosticKind::UnterminatedBlockComment && DiagnosticEntry.Message.find("outermost opening byte: 0") != std::string::npos && DiagnosticEntry.Message.find("remaining nesting depth: 2") != std::string::npos && DiagnosticEntry.Message.find("most recent unclosed opening byte: 9") != std::string::npos)
        {
          DetailedDiagnosticFound = true;
        }
      }
      EXPECT_TRUE(DetailedDiagnosticFound);
      expectFullFidelity(Lexed);
    }

    // Tests configured block-comment depth limits while preserving full source fidelity.
    TEST(TriviaCommentsTest, ConfiguredBlockCommentNestingLimitProducesAnErrorWithoutLosingBytes)
    {
      const std::string Source = "/* one /* two /* three */ two */ one */tail";
      const LexedFile Lexed = tokenize(Source, TokenizerOptions{2});

      EXPECT_FALSE(Lexed.succeeded());
      EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::BlockCommentNestingLimit));
      EXPECT_TRUE(hasErrorToken(Lexed));
      expectFullFidelity(Lexed);
    }

    // Tests that a backslash cannot continue a physical source line.
    TEST(TriviaCommentsTest, BackslashBeforeLineBreakDoesNotContinueTheLine)
    {
      const LexedFile Lexed = tokenize("a\\\nb");

      ASSERT_FALSE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 5U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[1]), "\\");
      EXPECT_EQ(Lexed.tokens()[2].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Lexed.tokens()[3].Kind, TokenKind::Identifier);
      expectFullFidelity(Lexed);
    }
  } // namespace
} // namespace ink::tokenizer
