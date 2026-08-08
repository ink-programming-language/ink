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

    // Tests that an empty source produces only the mandatory end-of-file token.
    TEST(SourceEncodingTest, EmptyFileHasExactlyOneEofToken)
    {
      const TokenizedBuffer Buffer = tokenize("");

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_TRUE(Buffer.diagnostics().empty());
      ASSERT_EQ(Buffer.tokens().size(), 1U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::EndOfFile);
      expectFullFidelity(Buffer);
    }

    // Tests initial UTF-8 BOM handling and byte-based spans for multibyte source text.
    TEST(SourceEncodingTest, InitialBomAndMultibyteCharactersUseOriginalByteSpans)
    {
      const std::string Source = utf8(u8"\uFEFF\u7528\u6237\r\n\tname");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 6U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Utf8Bom);
      EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Buffer.tokens()[0].Span.End, 3U);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[1].Span.Start, 3U);
      EXPECT_EQ(Buffer.tokens()[1].Span.End, 9U);
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Buffer.tokens()[2].Span.Start, 9U);
      EXPECT_EQ(Buffer.tokens()[2].Span.End, 11U);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[2]), "\r\n");
      EXPECT_EQ(Buffer.tokens()[3].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Buffer.tokens()[3].Span.Start, 11U);
      EXPECT_EQ(Buffer.tokens()[3].Span.End, 12U);
      EXPECT_EQ(Buffer.tokens()[4].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[4].Span.Start, 12U);
      EXPECT_EQ(Buffer.tokens()[4].Span.End, 16U);
      expectFullFidelity(Buffer);
    }

    // Tests acceptance of valid UTF-8 scalars through the maximum Unicode scalar value.
    TEST(SourceEncodingTest, StrictUtf8AcceptsValidScalarsIncludingMaximumScalar)
    {
      std::string Source = "//";
      Source.append(utf8(u8"\u00E9\u4E2D\U0001F600"));
      Source.append(bytes({0xF4, 0x8F, 0xBF, 0xBF}));
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
      expectFullFidelity(Buffer);
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
          {"four byte overlong encoding", bytes({0xF0, 0x8F, 0xBF, 0xBF})},
          {"surrogate", bytes({0xED, 0xA0, 0x80})},
          {"above unicode maximum", bytes({0xF4, 0x90, 0x80, 0x80})},
          {"invalid four byte leader", bytes({0xF5, 0x80, 0x80, 0x80})},
          {"utf16 little endian bom", bytes({0xFF, 0xFE})},
      };

      for (const InvalidUtf8Case &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const TokenizedBuffer Buffer = tokenize(TestCase.Bytes);
        EXPECT_FALSE(Buffer.succeeded());
        EXPECT_TRUE(hasTokenKind(Buffer, TokenKind::InvalidEncoding));
        EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::InvalidUtf8));
        expectFullFidelity(Buffer);
      }
    }

    // Tests that malformed third and fourth bytes in four-byte UTF-8 sequences do not consume a following ASCII identifier.
    TEST(SourceEncodingTest, FourByteUtf8ErrorsPreserveFollowingAsciiIdentifier)
    {
      struct InvalidUtf8RecoveryCase
      {
        const char *Name;
        std::string InvalidPrefix;
        std::string FollowingIdentifier;
      };
      const std::vector<InvalidUtf8RecoveryCase> Cases = {
          {"four byte overlong encoding", bytes({0xF0, 0x8F, 0xBF, 0xBF}), "x"},
          {"invalid third byte", bytes({0xF0, 0x90}), "xy"},
          {"invalid fourth byte", bytes({0xF0, 0x90, 0x80}), "x"},
      };

      for (const InvalidUtf8RecoveryCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const std::string Source = TestCase.InvalidPrefix + TestCase.FollowingIdentifier;
        const TokenizedBuffer Buffer = tokenize(Source);

        ASSERT_FALSE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 3U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidEncoding);
        EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(Buffer.tokens()[0].Span.End, TestCase.InvalidPrefix.size());
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), TestCase.InvalidPrefix);
        EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
        EXPECT_EQ(Buffer.tokens()[1].Span.Start, TestCase.InvalidPrefix.size());
        EXPECT_EQ(Buffer.tokens()[1].Span.End, Source.size());
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), TestCase.FollowingIdentifier);
        ASSERT_EQ(Buffer.diagnostics().size(), 1U);
        EXPECT_EQ(Buffer.diagnostics()[0].Kind, DiagnosticKind::InvalidUtf8);
        EXPECT_EQ(Buffer.diagnostics()[0].Span.Start, 0U);
        EXPECT_EQ(Buffer.diagnostics()[0].Span.End, TestCase.InvalidPrefix.size());
        expectFullFidelity(Buffer);
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
        const TokenizedBuffer Buffer = tokenize(Source);

        ASSERT_FALSE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 3U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidEncoding);
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), InvalidPrefix);
        EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "x");
        expectFullFidelity(Buffer);
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
        const TokenizedBuffer Buffer = tokenize(Source);
        EXPECT_FALSE(Buffer.succeeded());
        EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::InvalidUtf8));
        expectFullFidelity(Buffer);
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
        const TokenizedBuffer Buffer = tokenize(Source);
        EXPECT_FALSE(Buffer.succeeded());
        EXPECT_TRUE(hasTokenKind(Buffer, TokenKind::InvalidEncoding) || hasTokenKind(Buffer, TokenKind::InvalidCharacter));
        expectFullFidelity(Buffer);
      }
    }

    // Tests that a noninitial UTF-8 BOM is diagnosed instead of treated as whitespace.
    TEST(SourceEncodingTest, BomOutsideTheInitialPositionIsAnErrorAndNotWhitespace)
    {
      const std::string Source = utf8(u8"a\uFEFFb");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 4U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), utf8(u8"\uFEFF"));
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::UnexpectedBom));
      expectFullFidelity(Buffer);
    }

    // Tests LF and CRLF tokenization while preserving their distinct raw spellings.
    TEST(SourceEncodingTest, LfAndCrLfAreSingleLogicalLineBreakTokensWithDistinctRawBytes)
    {
      const std::string Source = "a \t\tb\nc\r\nd";
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 8U);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), " \t\t");
      EXPECT_EQ(Buffer.tokens()[3].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[3]), "\n");
      EXPECT_EQ(Buffer.tokens()[5].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[5]), "\r\n");
      expectFullFidelity(Buffer);
    }

    // Tests logical line-number lookup using original UTF-8 byte offsets.
    TEST(SourceEncodingTest, LogicalLineStartsRetainOriginalByteOffsets)
    {
      const std::string Source = utf8(u8"用户\r\nname\nlast");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      EXPECT_EQ(Buffer.lineStarts(), (std::vector<std::size_t>{0, 8, 13}));
      EXPECT_EQ(Buffer.lineNumber(0), 1U);
      EXPECT_EQ(Buffer.lineNumber(6), 1U);
      EXPECT_EQ(Buffer.lineNumber(8), 2U);
      EXPECT_EQ(Buffer.lineNumber(13), 3U);
      EXPECT_EQ(Buffer.lineNumber(Source.size()), 3U);
      expectFullFidelity(Buffer);
    }

    // Tests diagnosis of a lone carriage return and continued tokenization afterward.
    TEST(SourceEncodingTest, LoneCarriageReturnIsAnErrorAndRecoveryContinues)
    {
      const TokenizedBuffer Buffer = tokenize("a\rb");

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 4U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "\r");
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::LoneCarriageReturn));
      expectFullFidelity(Buffer);
    }

    // Tests the allowed C0 source characters and the forbidden control boundaries through DELETE.
    TEST(SourceEncodingTest, C0AndDeleteBoundariesFollowTheSourceCharacterPolicy)
    {
      const std::vector<unsigned int> ForbiddenValues = {
          0x00,
          0x08,
          0x0B,
          0x0C,
          0x0E,
          0x1F,
          0x7F,
      };
      for (const unsigned int Value : ForbiddenValues)
      {
        SCOPED_TRACE(Value);
        const TokenizedBuffer Buffer = tokenize(bytes({Value}));

        ASSERT_FALSE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 2U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidCharacter);
        EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(Buffer.tokens()[0].Span.End, 1U);
        ASSERT_EQ(Buffer.diagnostics().size(), 1U);
        EXPECT_EQ(Buffer.diagnostics()[0].Kind, DiagnosticKind::ForbiddenControlCharacter);
        EXPECT_EQ(Buffer.diagnostics()[0].Span.Start, 0U);
        EXPECT_EQ(Buffer.diagnostics()[0].Span.End, 1U);
        expectFullFidelity(Buffer);
      }

      const TokenizedBuffer Allowed = tokenize("\t\n\r\n ~");
      ASSERT_TRUE(Allowed.succeeded());
      ASSERT_EQ(Allowed.tokens().size(), 6U);
      EXPECT_EQ(Allowed.tokens()[0].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Allowed.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Allowed.raw(Allowed.tokens()[1]), "\n");
      EXPECT_EQ(Allowed.tokens()[2].Kind, TokenKind::LineBreak);
      EXPECT_EQ(Allowed.raw(Allowed.tokens()[2]), "\r\n");
      EXPECT_EQ(Allowed.tokens()[3].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Allowed.tokens()[4].Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Allowed.tokens()[4].Payload), '~');
      expectFullFidelity(Allowed);

      const TokenizedBuffer LoneCarriageReturn = tokenize("\r");
      ASSERT_FALSE(LoneCarriageReturn.succeeded());
      ASSERT_EQ(LoneCarriageReturn.diagnostics().size(), 1U);
      EXPECT_EQ(LoneCarriageReturn.diagnostics()[0].Kind, DiagnosticKind::LoneCarriageReturn);
      EXPECT_FALSE(hasDiagnosticKind(LoneCarriageReturn, DiagnosticKind::ForbiddenControlCharacter));
      expectFullFidelity(LoneCarriageReturn);
    }

    // Tests rejection of non-ASCII whitespace and Unicode line-separator lookalikes.
    TEST(SourceEncodingTest, UnicodeWhitespaceAndUnicodeLineSeparatorsAreNotCodeWhitespaceOrLineBreaks)
    {
      const std::vector<std::string> Cases = {
          utf8(u8"a\u0085b"),
          utf8(u8"a\u00A0b"),
          utf8(u8"a\u1680b"),
          utf8(u8"a\u2000b"),
          utf8(u8"a\u2003b"),
          utf8(u8"a\u200Ab"),
          utf8(u8"a\u2028b"),
          utf8(u8"a\u2029b"),
          utf8(u8"a\u202Fb"),
          utf8(u8"a\u205Fb"),
          utf8(u8"a\u3000b"),
      };

      for (const std::string &Source : Cases)
      {
        SCOPED_TRACE(Source);
        const TokenizedBuffer Buffer = tokenize(Source);
        EXPECT_FALSE(Buffer.succeeded());
        EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::NonAsciiWhitespace));
        EXPECT_FALSE(hasTokenKind(Buffer, TokenKind::LineBreak));
        expectFullFidelity(Buffer);
      }
    }

    // Tests that scalar values immediately outside the fixed Unicode whitespace ranges are not classified as source whitespace.
    TEST(SourceEncodingTest, AdjacentUnicodeWhitespaceNonMembersUseTheirOwnLexicalClassification)
    {
      const std::vector<std::string> Cases = {
          utf8(u8"a\u167Fb"),
          utf8(u8"a\u1681b"),
          utf8(u8"a\u1FFFb"),
          utf8(u8"a\u200Bb"),
          utf8(u8"a\u202Eb"),
          utf8(u8"a\u2030b"),
          utf8(u8"a\u205Eb"),
          utf8(u8"a\u2060b"),
      };

      for (const std::string &Source : Cases)
      {
        SCOPED_TRACE(Source);
        const TokenizedBuffer Buffer = tokenize(Source);
        EXPECT_FALSE(hasDiagnosticKind(Buffer, DiagnosticKind::NonAsciiWhitespace));
        expectFullFidelity(Buffer);
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
        const TokenizedBuffer Buffer = tokenize(Source);
        EXPECT_FALSE(Buffer.succeeded());
        EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::ForbiddenControlCharacter));
        expectFullFidelity(Buffer);
      }
    }

    // Tests that tokenization preserves the original source bytes without global normalization.
    TEST(SourceEncodingTest, TokenizerDoesNotNormalizeTheWholeSource)
    {
      const std::string Source = utf8(u8"//\u00E9 e\u0301");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
      EXPECT_EQ(Buffer.source(), Source);
      expectFullFidelity(Buffer);
    }

    // Tests recovery when a valid Unicode scalar cannot begin any token.
    TEST(SourceEncodingTest, AUnicodeScalarThatCannotStartAnyTokenProducesAnErrorToken)
    {
      const std::string Source = utf8(u8"\U0001F600value");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), utf8(u8"\U0001F600"));
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "value");
      EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::InvalidCharacter));
      expectFullFidelity(Buffer);
    }
  } // namespace
} // namespace ink::tokenizer
