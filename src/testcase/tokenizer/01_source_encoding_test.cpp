#include "tokenizer_test_support.h"
#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <string_view>
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

    void appendUtf8Scalar(std::string &Result, char32_t Value)
    {
      if (Value <= 0x7F)
      {
        Result.push_back(static_cast<char>(Value));
      }
      else if (Value <= 0x7FF)
      {
        Result.push_back(static_cast<char>(0xC0 | (Value >> 6U)));
        Result.push_back(static_cast<char>(0x80 | (Value & 0x3F)));
      }
      else if (Value <= 0xFFFF)
      {
        Result.push_back(static_cast<char>(0xE0 | (Value >> 12U)));
        Result.push_back(static_cast<char>(0x80 | ((Value >> 6U) & 0x3F)));
        Result.push_back(static_cast<char>(0x80 | (Value & 0x3F)));
      }
      else
      {
        Result.push_back(static_cast<char>(0xF0 | (Value >> 18U)));
        Result.push_back(static_cast<char>(0x80 | ((Value >> 12U) & 0x3F)));
        Result.push_back(static_cast<char>(0x80 | ((Value >> 6U) & 0x3F)));
        Result.push_back(static_cast<char>(0x80 | (Value & 0x3F)));
      }
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
      for (const Diagnostic &DiagnosticEntry : testDiagnostics(Buffer))
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
      ASSERT_TRUE(testDiagnostics(Buffer).empty());
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
      ASSERT_TRUE(testDiagnostics(Buffer).empty());
      ASSERT_EQ(Buffer.tokens().size(), 6U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Utf8Bom);
      EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Buffer.tokens()[0].Span.End, 3U);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), bytes({0xEF, 0xBB, 0xBF}));
      EXPECT_TRUE(Buffer.tokens()[0].isTrivia());
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

    // Tests every byte-length transition and the surrogate-gap boundaries of valid strict UTF-8.
    TEST(SourceEncodingTest, StrictUtf8AcceptsExactScalarBoundaryEncodings)
    {
      struct ValidUtf8BoundaryCase
      {
          const char *Name;
          std::string Encoding;
      };
      const std::vector<ValidUtf8BoundaryCase> Cases = {
          {"lowest two-byte scalar", bytes({0xC2, 0x80})},
          {"highest two-byte scalar", bytes({0xDF, 0xBF})},
          {"lowest three-byte scalar", bytes({0xE0, 0xA0, 0x80})},
          {"last scalar before surrogates", bytes({0xED, 0x9F, 0xBF})},
          {"first scalar after surrogates", bytes({0xEE, 0x80, 0x80})},
          {"highest three-byte scalar", bytes({0xEF, 0xBF, 0xBF})},
          {"lowest four-byte scalar", bytes({0xF0, 0x90, 0x80, 0x80})},
          {"highest unicode scalar", bytes({0xF4, 0x8F, 0xBF, 0xBF})},
      };

      for (const ValidUtf8BoundaryCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const std::string Source = std::string("//") + TestCase.Encoding;
        const TokenizedBuffer Buffer = tokenize(Source);

        ASSERT_TRUE(Buffer.succeeded());
        ASSERT_TRUE(testDiagnostics(Buffer).empty());
        ASSERT_EQ(Buffer.tokens().size(), 2U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
        EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(Buffer.tokens()[0].Span.End, Source.size());
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
        expectFullFidelity(Buffer);
      }
    }

    // Tests that strict UTF-8 decoding accepts every Unicode scalar value allowed in raw comment text.
    TEST(SourceEncodingTest, StrictUtf8AcceptsEveryUnicodeScalarAllowedInRawCommentText)
    {
      std::string Source = "//";
      for (char32_t Value = 0; Value <= 0x10FFFF; ++Value)
      {
        if ((Value <= 0x1F && Value != U'\t') || Value == 0x7F || (Value >= 0xD800 && Value <= 0xDFFF))
        {
          continue;
        }
        appendUtf8Scalar(Source, Value);
      }
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_TRUE(testDiagnostics(Buffer).empty());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::LineComment);
      EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Buffer.tokens()[0].Span.End, Source.size());
      EXPECT_EQ(Buffer.lineStarts(), (std::vector<std::size_t>{0}));
      expectFullFidelity(Buffer);
    }

    // Tests every disallowed UTF-8 leading byte as an isolated byte with an exact recovery span.
    TEST(SourceEncodingTest, StrictUtf8RejectsEveryInvalidLeadingBytePrecisely)
    {
      std::vector<unsigned int> InvalidLeaders;
      for (unsigned int Value = 0x80; Value <= 0xBF; ++Value)
      {
        InvalidLeaders.push_back(Value);
      }
      InvalidLeaders.push_back(0xC0);
      InvalidLeaders.push_back(0xC1);
      for (unsigned int Value = 0xF5; Value <= 0xFF; ++Value)
      {
        InvalidLeaders.push_back(Value);
      }

      for (const unsigned int Leader : InvalidLeaders)
      {
        SCOPED_TRACE(Leader);
        const std::string Source = bytes({Leader});
        const TokenizedBuffer Buffer = tokenize(Source);

        ASSERT_FALSE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 2U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidEncoding);
        EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(Buffer.tokens()[0].Span.End, 1U);
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
        ASSERT_EQ(testDiagnostics(Buffer).size(), 1U);
        EXPECT_EQ(testDiagnostics(Buffer)[0].Kind, DiagnosticKind::InvalidUtf8);
        EXPECT_EQ(testDiagnostics(Buffer)[0].Span.Start, 0U);
        EXPECT_EQ(testDiagnostics(Buffer)[0].Span.End, 1U);
        expectFullFidelity(Buffer);
      }
    }

    // Tests the strict second-byte constraints around overlongs, surrogates, and the Unicode maximum.
    TEST(SourceEncodingTest, StrictUtf8RejectsRestrictedSecondByteBoundariesPrecisely)
    {
      struct InvalidUtf8BoundaryCase
      {
          const char *Name;
          std::string Encoding;
      };
      const std::vector<InvalidUtf8BoundaryCase> Cases = {
          {"three-byte overlong boundary", bytes({0xE0, 0x9F, 0xBF})},
          {"first surrogate", bytes({0xED, 0xA0, 0x80})},
          {"four-byte overlong boundary", bytes({0xF0, 0x8F, 0xBF, 0xBF})},
          {"first scalar above unicode maximum", bytes({0xF4, 0x90, 0x80, 0x80})},
      };

      for (const InvalidUtf8BoundaryCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const TokenizedBuffer Buffer = tokenize(TestCase.Encoding);

        ASSERT_FALSE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 2U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidEncoding);
        EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(Buffer.tokens()[0].Span.End, TestCase.Encoding.size());
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), TestCase.Encoding);
        ASSERT_EQ(testDiagnostics(Buffer).size(), 1U);
        EXPECT_EQ(testDiagnostics(Buffer)[0].Kind, DiagnosticKind::InvalidUtf8);
        EXPECT_EQ(testDiagnostics(Buffer)[0].Span.Start, 0U);
        EXPECT_EQ(testDiagnostics(Buffer)[0].Span.End, TestCase.Encoding.size());
        expectFullFidelity(Buffer);
      }
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
        ASSERT_EQ(testDiagnostics(Buffer).size(), 1U);
        EXPECT_EQ(testDiagnostics(Buffer)[0].Kind, DiagnosticKind::InvalidUtf8);
        EXPECT_EQ(testDiagnostics(Buffer)[0].Span.Start, 0U);
        EXPECT_EQ(testDiagnostics(Buffer)[0].Span.End, TestCase.InvalidPrefix.size());
        expectFullFidelity(Buffer);
      }
    }

    // Tests every truncated UTF-8 prefix both at EOF and before a non-continuation byte with exact recovery.
    TEST(SourceEncodingTest, EveryTruncatedUtf8PrefixHasAnExactRecoverySpan)
    {
      struct TruncatedUtf8Case
      {
          const char *Name;
          std::string Prefix;
      };
      const std::vector<TruncatedUtf8Case> Cases = {
          {"two-byte sequence missing its second byte", bytes({0xC2})},
          {"three-byte sequence missing its second and third bytes", bytes({0xE2})},
          {"three-byte sequence missing its third byte", bytes({0xE2, 0x82})},
          {"four-byte sequence missing its second through fourth bytes", bytes({0xF0})},
          {"four-byte sequence missing its third and fourth bytes", bytes({0xF0, 0x90})},
          {"four-byte sequence missing its fourth byte", bytes({0xF0, 0x90, 0x80})},
      };

      for (const TruncatedUtf8Case &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const TokenizedBuffer EofBuffer = tokenize(TestCase.Prefix);

        ASSERT_FALSE(EofBuffer.succeeded());
        ASSERT_EQ(EofBuffer.tokens().size(), 2U);
        EXPECT_EQ(EofBuffer.tokens()[0].Kind, TokenKind::InvalidEncoding);
        EXPECT_EQ(EofBuffer.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(EofBuffer.tokens()[0].Span.End, TestCase.Prefix.size());
        EXPECT_EQ(EofBuffer.raw(EofBuffer.tokens()[0]), TestCase.Prefix);
        ASSERT_EQ(testDiagnostics(EofBuffer).size(), 1U);
        EXPECT_EQ(testDiagnostics(EofBuffer)[0].Kind, DiagnosticKind::InvalidUtf8);
        EXPECT_EQ(testDiagnostics(EofBuffer)[0].Span.Start, 0U);
        EXPECT_EQ(testDiagnostics(EofBuffer)[0].Span.End, TestCase.Prefix.size());
        expectFullFidelity(EofBuffer);

        const std::string InterruptedSource = TestCase.Prefix + "x";
        const TokenizedBuffer InterruptedBuffer = tokenize(InterruptedSource);

        ASSERT_FALSE(InterruptedBuffer.succeeded());
        ASSERT_EQ(InterruptedBuffer.tokens().size(), 3U);
        EXPECT_EQ(InterruptedBuffer.tokens()[0].Kind, TokenKind::InvalidEncoding);
        EXPECT_EQ(InterruptedBuffer.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(InterruptedBuffer.tokens()[0].Span.End, TestCase.Prefix.size());
        EXPECT_EQ(InterruptedBuffer.raw(InterruptedBuffer.tokens()[0]), TestCase.Prefix);
        EXPECT_EQ(InterruptedBuffer.tokens()[1].Kind, TokenKind::Identifier);
        EXPECT_EQ(InterruptedBuffer.tokens()[1].Span.Start, TestCase.Prefix.size());
        EXPECT_EQ(InterruptedBuffer.tokens()[1].Span.End, InterruptedSource.size());
        EXPECT_EQ(InterruptedBuffer.raw(InterruptedBuffer.tokens()[1]), "x");
        ASSERT_EQ(testDiagnostics(InterruptedBuffer).size(), 1U);
        EXPECT_EQ(testDiagnostics(InterruptedBuffer)[0].Kind, DiagnosticKind::InvalidUtf8);
        EXPECT_EQ(testDiagnostics(InterruptedBuffer)[0].Span.Start, 0U);
        EXPECT_EQ(testDiagnostics(InterruptedBuffer)[0].Span.End, TestCase.Prefix.size());
        expectFullFidelity(InterruptedBuffer);
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

    // Tests rejection of every UTF-16 and UTF-32 byte-order mark byte with exact tokens and diagnostics.
    TEST(SourceEncodingTest, Utf16AndUtf32ByteOrderMarksAreRejected)
    {
      struct RejectedBomCase
      {
          const char *Name;
          std::string Source;
          std::vector<TokenKind> TokenKinds;
          std::vector<DiagnosticKind> DiagnosticKinds;
      };
      const std::vector<RejectedBomCase> Cases = {
          {"utf16 little endian", bytes({0xFF, 0xFE}), {TokenKind::InvalidEncoding, TokenKind::InvalidEncoding}, {DiagnosticKind::InvalidUtf8, DiagnosticKind::InvalidUtf8}},
          {"utf16 big endian", bytes({0xFE, 0xFF}), {TokenKind::InvalidEncoding, TokenKind::InvalidEncoding}, {DiagnosticKind::InvalidUtf8, DiagnosticKind::InvalidUtf8}},
          {"utf32 little endian", bytes({0xFF, 0xFE, 0x00, 0x00}), {TokenKind::InvalidEncoding, TokenKind::InvalidEncoding, TokenKind::InvalidCharacter, TokenKind::InvalidCharacter}, {DiagnosticKind::InvalidUtf8, DiagnosticKind::InvalidUtf8, DiagnosticKind::ForbiddenControlCharacter, DiagnosticKind::ForbiddenControlCharacter}},
          {"utf32 big endian", bytes({0x00, 0x00, 0xFE, 0xFF}), {TokenKind::InvalidCharacter, TokenKind::InvalidCharacter, TokenKind::InvalidEncoding, TokenKind::InvalidEncoding}, {DiagnosticKind::ForbiddenControlCharacter, DiagnosticKind::ForbiddenControlCharacter, DiagnosticKind::InvalidUtf8, DiagnosticKind::InvalidUtf8}},
      };

      for (const RejectedBomCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const TokenizedBuffer Buffer = tokenize(TestCase.Source);

        ASSERT_FALSE(Buffer.succeeded());
        ASSERT_EQ(TestCase.TokenKinds.size(), TestCase.Source.size());
        ASSERT_EQ(TestCase.DiagnosticKinds.size(), TestCase.Source.size());
        ASSERT_EQ(Buffer.tokens().size(), TestCase.Source.size() + 1);
        ASSERT_EQ(testDiagnostics(Buffer).size(), TestCase.Source.size());
        for (std::size_t Index = 0; Index < TestCase.Source.size(); ++Index)
        {
          EXPECT_EQ(Buffer.tokens()[Index].Kind, TestCase.TokenKinds[Index]);
          EXPECT_EQ(Buffer.tokens()[Index].Span.Start, Index);
          EXPECT_EQ(Buffer.tokens()[Index].Span.End, Index + 1);
          EXPECT_EQ(Buffer.raw(Buffer.tokens()[Index]), std::string_view(TestCase.Source).substr(Index, 1));
          EXPECT_EQ(testDiagnostics(Buffer)[Index].Kind, TestCase.DiagnosticKinds[Index]);
          EXPECT_EQ(testDiagnostics(Buffer)[Index].Span.Start, Index);
          EXPECT_EQ(testDiagnostics(Buffer)[Index].Span.End, Index + 1);
        }
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
      EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Buffer.tokens()[0].Span.End, 1U);
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.tokens()[1].Span.Start, 1U);
      EXPECT_EQ(Buffer.tokens()[1].Span.End, 4U);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), utf8(u8"\uFEFF"));
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[2].Span.Start, 4U);
      EXPECT_EQ(Buffer.tokens()[2].Span.End, 5U);
      ASSERT_EQ(testDiagnostics(Buffer).size(), 1U);
      EXPECT_EQ(testDiagnostics(Buffer)[0].Kind, DiagnosticKind::UnexpectedBom);
      EXPECT_EQ(testDiagnostics(Buffer)[0].Span.Start, 1U);
      EXPECT_EQ(testDiagnostics(Buffer)[0].Span.End, 4U);
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
      std::vector<unsigned int> ForbiddenValues;
      for (unsigned int Value = 0; Value <= 0x1F; ++Value)
      {
        if (Value != 0x09 && Value != 0x0A && Value != 0x0D)
        {
          ForbiddenValues.push_back(Value);
        }
      }
      ForbiddenValues.push_back(0x7F);
      for (const unsigned int Value : ForbiddenValues)
      {
        SCOPED_TRACE(Value);
        const TokenizedBuffer Buffer = tokenize(bytes({Value}));

        ASSERT_FALSE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 2U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidCharacter);
        EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(Buffer.tokens()[0].Span.End, 1U);
        ASSERT_EQ(testDiagnostics(Buffer).size(), 1U);
        EXPECT_EQ(testDiagnostics(Buffer)[0].Kind, DiagnosticKind::ForbiddenControlCharacter);
        EXPECT_EQ(testDiagnostics(Buffer)[0].Span.Start, 0U);
        EXPECT_EQ(testDiagnostics(Buffer)[0].Span.End, 1U);
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
      ASSERT_EQ(testDiagnostics(LoneCarriageReturn).size(), 1U);
      EXPECT_EQ(testDiagnostics(LoneCarriageReturn)[0].Kind, DiagnosticKind::LoneCarriageReturn);
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
