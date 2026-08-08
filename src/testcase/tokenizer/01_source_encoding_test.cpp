#include "ink/tokenizer/tokenizer.h"
#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <vector>

namespace ink::tokenizer
{
  namespace
  {
    using core::Diagnostic;
    using core::DiagnosticKind;

    std::string bytes(std::initializer_list<unsigned int> Values)
    {
      std::string Result;
      Result.reserve(Values.size());
      for (const unsigned int Value : Values)
      {
        Result.push_back(static_cast<char>(Value));
      }
      return Result;
    }

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

    // Tests that an empty source produces only the mandatory end-of-file token.
    TEST(SourceEncodingTest, EmptyFileHasExactlyOneEofToken)
    {
      const LexedFile Lexed = tokenize("");

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_TRUE(Lexed.diagnostics().empty());
      ASSERT_EQ(Lexed.tokens().size(), 1U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::EndOfFile);
      expectFullFidelity(Lexed);
    }

    // Tests initial UTF-8 BOM handling and byte-based spans for multibyte source text.
    TEST(SourceEncodingTest, InitialBomAndMultibyteCharactersUseOriginalByteSpans)
    {
      const std::string Source = utf8(u8"\uFEFF\u7528\u6237\r\n\tname");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 6U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::Utf8Bom);
      EXPECT_EQ(Lexed.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Lexed.tokens()[0].Span.End, 3U);
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.tokens()[1].Span.Start, 3U);
      EXPECT_EQ(Lexed.tokens()[1].Span.End, 9U);
      EXPECT_EQ(Lexed.tokens()[2].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Lexed.tokens()[2].Span.Start, 9U);
      EXPECT_EQ(Lexed.tokens()[2].Span.End, 11U);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[2]), "\r\n");
      EXPECT_EQ(Lexed.tokens()[3].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Lexed.tokens()[3].Span.Start, 11U);
      EXPECT_EQ(Lexed.tokens()[3].Span.End, 12U);
      EXPECT_EQ(Lexed.tokens()[4].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.tokens()[4].Span.Start, 12U);
      EXPECT_EQ(Lexed.tokens()[4].Span.End, 16U);
      expectFullFidelity(Lexed);
    }

    // Tests acceptance of valid UTF-8 scalars through the maximum Unicode scalar value.
    TEST(SourceEncodingTest, StrictUtf8AcceptsValidScalarsIncludingMaximumScalar)
    {
      std::string Source = "//";
      Source.append(utf8(u8"\u00E9\u4E2D\U0001F600"));
      Source.append(bytes({0xF4, 0x8F, 0xBF, 0xBF}));
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 2U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), Source);
      expectFullFidelity(Lexed);
    }

    // Tests rejection and recovery for every major class of malformed UTF-8 sequence.
    TEST(SourceEncodingTest, StrictUtf8RejectsEveryInvalidSequenceClass)
    {
      struct InvalidUtf8Case
      {
        const char *Name;
        std::string Bytes;
      };
      const std::vector<InvalidUtf8Case> Cases = {
          {"invalid leading byte", bytes({0x80})},
          {"invalid continuation byte", bytes({0xC2, 0x41})},
          {"truncated sequence", bytes({0xE2, 0x82})},
          {"two byte overlong encoding", bytes({0xC0, 0x80})},
          {"three byte overlong encoding", bytes({0xE0, 0x80, 0x80})},
          {"surrogate", bytes({0xED, 0xA0, 0x80})},
          {"above unicode maximum", bytes({0xF4, 0x90, 0x80, 0x80})},
          {"invalid four byte leader", bytes({0xF5, 0x80, 0x80, 0x80})},
          {"utf16 little endian bom", bytes({0xFF, 0xFE})},
      };

      for (const InvalidUtf8Case &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const LexedFile Lexed = tokenize(TestCase.Bytes);
        EXPECT_FALSE(Lexed.succeeded());
        EXPECT_TRUE(hasTokenKind(Lexed, TokenKind::InvalidEncoding));
        EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::InvalidUtf8));
        expectFullFidelity(Lexed);
      }
    }

    // Tests that malformed UTF-8 recovery preserves the following valid token.
    TEST(SourceEncodingTest, InvalidUtf8RecoveryDoesNotConsumeFollowingToken)
    {
      const std::vector<std::string> InvalidPrefixes = {
          bytes({0xE2}),
          bytes({0xE2, 0x82}),
          bytes({0xF0, 0x80}),
      };
      for (const std::string &InvalidPrefix : InvalidPrefixes)
      {
        std::string Source = InvalidPrefix;
        Source.push_back('x');
        const LexedFile Lexed = tokenize(Source);

        ASSERT_FALSE(Lexed.succeeded());
        ASSERT_EQ(Lexed.tokens().size(), 3U);
        EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::InvalidEncoding);
        EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), InvalidPrefix);
        EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::Identifier);
        EXPECT_EQ(Lexed.raw(Lexed.tokens()[1]), "x");
        expectFullFidelity(Lexed);
      }
    }

    // Tests strict UTF-8 validation while scanning comments and all literal states.
    TEST(SourceEncodingTest, StrictUtf8ValidationAlsoAppliesInsideCommentsAndLiterals)
    {
      const std::string InvalidByte = bytes({0x80});
      const std::vector<std::string> Cases = {
          std::string("//") + InvalidByte,
          std::string("/*") + InvalidByte,
          std::string("\"") + InvalidByte + "\"",
          std::string("r\"") + InvalidByte + "\"",
          std::string("'\\u{") + InvalidByte + "}'",
          std::string("\"\"\"\n") + InvalidByte,
          std::string("\"\"\"inline\n") + InvalidByte,
      };

      for (const std::string &Source : Cases)
      {
        SCOPED_TRACE(Source.size());
        const LexedFile Lexed = tokenize(Source);
        EXPECT_FALSE(Lexed.succeeded());
        EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::InvalidUtf8));
        expectFullFidelity(Lexed);
      }
    }

    // Tests rejection of UTF-16 and UTF-32 byte-order marks.
    TEST(SourceEncodingTest, Utf16AndUtf32ByteOrderMarksAreRejected)
    {
      const std::vector<std::string> Cases = {
          bytes({0xFF, 0xFE}),
          bytes({0xFE, 0xFF}),
          bytes({0xFF, 0xFE, 0x00, 0x00}),
          bytes({0x00, 0x00, 0xFE, 0xFF}),
      };

      for (const std::string &Source : Cases)
      {
        SCOPED_TRACE(Source.size());
        const LexedFile Lexed = tokenize(Source);
        EXPECT_FALSE(Lexed.succeeded());
        EXPECT_TRUE(hasTokenKind(Lexed, TokenKind::InvalidEncoding) || hasTokenKind(Lexed, TokenKind::InvalidCharacter));
        expectFullFidelity(Lexed);
      }
    }

    // Tests that a noninitial UTF-8 BOM is diagnosed instead of treated as whitespace.
    TEST(SourceEncodingTest, BomOutsideTheInitialPositionIsAnErrorAndNotWhitespace)
    {
      const std::string Source = utf8(u8"a\uFEFFb");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_FALSE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 4U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[1]), utf8(u8"\uFEFF"));
      EXPECT_EQ(Lexed.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::UnexpectedBom));
      expectFullFidelity(Lexed);
    }

    // Tests LF and CRLF tokenization while preserving their distinct raw spellings.
    TEST(SourceEncodingTest, LfAndCrLfAreSingleLogicalLineBreakTokensWithDistinctRawBytes)
    {
      const std::string Source = "a \t\tb\nc\r\nd";
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 8U);
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[1]), " \t\t");
      EXPECT_EQ(Lexed.tokens()[3].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[3]), "\n");
      EXPECT_EQ(Lexed.tokens()[5].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[5]), "\r\n");
      expectFullFidelity(Lexed);
    }

    // Tests logical line-number lookup using original UTF-8 byte offsets.
    TEST(SourceEncodingTest, LogicalLineStartsRetainOriginalByteOffsets)
    {
      const std::string Source = utf8(u8"用户\r\nname\nlast");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      EXPECT_EQ(Lexed.lineStarts(), (std::vector<std::size_t>{0, 8, 13}));
      EXPECT_EQ(Lexed.lineNumber(0), 1U);
      EXPECT_EQ(Lexed.lineNumber(6), 1U);
      EXPECT_EQ(Lexed.lineNumber(8), 2U);
      EXPECT_EQ(Lexed.lineNumber(13), 3U);
      EXPECT_EQ(Lexed.lineNumber(Source.size()), 3U);
      expectFullFidelity(Lexed);
    }

    // Tests diagnosis of a lone carriage return and continued tokenization afterward.
    TEST(SourceEncodingTest, LoneCarriageReturnIsAnErrorAndRecoveryContinues)
    {
      const LexedFile Lexed = tokenize("a\rb");

      ASSERT_FALSE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 4U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[1]), "\r");
      EXPECT_EQ(Lexed.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::LoneCarriageReturn));
      expectFullFidelity(Lexed);
    }

    // Tests rejection of non-ASCII whitespace and Unicode line-separator lookalikes.
    TEST(SourceEncodingTest, UnicodeWhitespaceAndUnicodeLineSeparatorsAreNotCodeWhitespaceOrLineBreaks)
    {
      const std::vector<std::string> Cases = {
          utf8(u8"a\u0085b"),
          utf8(u8"a\u00A0b"),
          utf8(u8"a\u2003b"),
          utf8(u8"a\u2028b"),
          utf8(u8"a\u2029b"),
          utf8(u8"a\u3000b"),
      };

      for (const std::string &Source : Cases)
      {
        SCOPED_TRACE(Source);
        const LexedFile Lexed = tokenize(Source);
        EXPECT_FALSE(Lexed.succeeded());
        EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::NonAsciiWhitespace));
        EXPECT_FALSE(hasTokenKind(Lexed, TokenKind::LineBreak));
        expectFullFidelity(Lexed);
      }
    }

    // Tests forbidden raw control diagnostics in comments, strings, and unterminated states.
    TEST(SourceEncodingTest, RawControlsAreForbiddenEvenInsideCommentsAndLiterals)
    {
      std::string NulInComment = "//before";
      NulInComment.push_back('\0');
      NulInComment.append("after");
      std::string EscapeInString = "\"before";
      EscapeInString.push_back(static_cast<char>(0x1B));
      EscapeInString.append("after\"");
      std::string DelInBlockComment = "/*before";
      DelInBlockComment.push_back(static_cast<char>(0x7F));
      DelInBlockComment.append("after*/");
      std::string NulInUnterminatedBlock = "/*before";
      NulInUnterminatedBlock.push_back('\0');
      std::string NulInUnterminatedMultiline = "\"\"\"\n  before";
      NulInUnterminatedMultiline.push_back('\0');
      std::string NulAfterEscape = "\"\\";
      NulAfterEscape.push_back('\0');
      NulAfterEscape.push_back('"');
      std::string NulInUnicodeEscape = "\"\\u{";
      NulInUnicodeEscape.push_back('\0');
      NulInUnicodeEscape.append("}\"");
      const std::vector<std::string> Cases = {
          NulInComment,
          EscapeInString,
          DelInBlockComment,
          NulInUnterminatedBlock,
          NulInUnterminatedMultiline,
          NulAfterEscape,
          NulInUnicodeEscape,
      };

      for (const std::string &Source : Cases)
      {
        SCOPED_TRACE(Source.size());
        const LexedFile Lexed = tokenize(Source);
        EXPECT_FALSE(Lexed.succeeded());
        EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::ForbiddenControlCharacter));
        expectFullFidelity(Lexed);
      }
    }

    // Tests that tokenization preserves the original source bytes without global normalization.
    TEST(SourceEncodingTest, TokenizerDoesNotNormalizeTheWholeSource)
    {
      const std::string Source = utf8(u8"//\u00E9 e\u0301");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 2U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), Source);
      EXPECT_EQ(Lexed.source(), Source);
      expectFullFidelity(Lexed);
    }

    // Tests recovery when a valid Unicode scalar cannot begin any token.
    TEST(SourceEncodingTest, AUnicodeScalarThatCannotStartAnyTokenProducesAnErrorToken)
    {
      const std::string Source = utf8(u8"\U0001F600value");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_FALSE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 3U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), utf8(u8"\U0001F600"));
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[1]), "value");
      EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::InvalidCharacter));
      expectFullFidelity(Lexed);
    }
  } // namespace
} // namespace ink::tokenizer
