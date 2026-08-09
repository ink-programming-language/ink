#include "ink/tokenizer/tokenizer.h"
#include "tokenizer_test_support.h"

#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace ink::tokenizer
{
  namespace
  {
    using core::Diagnostic;
    using core::DiagnosticKind;
    using core::SourceRange;

    struct ValidScalarCase
    {
      std::string Spelling;
      char32_t Value;
    };

    struct InvalidScalarCase
    {
      std::string Spelling;
      DiagnosticKind Diagnostic;
    };

    bool hasDiagnostic(const TokenizedBuffer &Result, DiagnosticKind Kind)
    {
      return std::any_of(Result.diagnostics().begin(), Result.diagnostics().end(), [Kind](const Diagnostic &Diagnostic)
                         {
                           return Diagnostic.Kind == Kind;
                         });
    }

    // Verifies that a scalar literal accepts exactly one directly encoded Unicode scalar value.
    TEST(ScalarLiteralsTest, AcceptsExactlyOneDirectUnicodeScalar)
    {
      const std::vector<ValidScalarCase> Cases = {
          {"'A'", U'A'},
          {utf8(u8"'\u00E9'"), U'\u00E9'},
          {utf8(u8"'\u4E2D'"), U'\u4E2D'},
          {utf8(u8"'\uD7FF'"), U'\uD7FF'},
          {utf8(u8"'\uE000'"), U'\uE000'},
          {utf8(u8"'\U0001F600'"), U'\U0001F600'},
      };

      for (const ValidScalarCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(TestSourceFileId, Test.Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        const Token &Token = Result.tokens().front();
        EXPECT_EQ(Token.Kind, TokenKind::ScalarLiteral);
        EXPECT_EQ(Token.Span, (SourceRange{0, Test.Spelling.size()}));
        EXPECT_EQ(Result.raw(Token), Test.Spelling);
        ASSERT_TRUE(std::holds_alternative<char32_t>(Token.Payload));
        EXPECT_EQ(std::get<char32_t>(Token.Payload), Test.Value);
      }
    }

    // Verifies that NEL, LINE SEPARATOR, and PARAGRAPH SEPARATOR are scalar contents rather than logical source line breaks.
    TEST(ScalarLiteralsTest, UnicodeSeparatorsRemainScalarValuesWithoutCreatingLines)
    {
      const std::vector<ValidScalarCase> Cases = {
          {std::string("'\xC2\x85'", 4), static_cast<char32_t>(0x0085)},
          {utf8(u8"'\u2028'"), U'\u2028'},
          {utf8(u8"'\u2029'"), U'\u2029'},
      };

      for (const ValidScalarCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(TestSourceFileId, Test.Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        EXPECT_EQ(Result.tokens().front().Kind, TokenKind::ScalarLiteral);
        EXPECT_EQ(Result.raw(Result.tokens().front()), Test.Spelling);
        ASSERT_TRUE(std::holds_alternative<char32_t>(Result.tokens().front().Payload));
        EXPECT_EQ(std::get<char32_t>(Result.tokens().front().Payload), Test.Value);
        EXPECT_EQ(Result.lineStarts(), (std::vector<std::size_t>{0}));
        EXPECT_EQ(Result.lineNumber(Test.Spelling.size()), 1U);
      }
    }

    // Verifies decoding of every supported single-character escape in scalar literals.
    TEST(ScalarLiteralsTest, DecodesEverySimpleEscape)
    {
      const std::vector<ValidScalarCase> Cases = {
          {"'\\\\'", U'\\'},
          {"'\\''", U'\''},
          {"'\\\"'", U'\"'},
          {"'\\0'", U'\0'},
          {"'\\n'", U'\n'},
          {"'\\r'", U'\r'},
          {"'\\t'", U'\t'},
      };

      for (const ValidScalarCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(TestSourceFileId, Test.Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        const Token &Token = Result.tokens().front();
        EXPECT_EQ(Token.Kind, TokenKind::ScalarLiteral);
        EXPECT_EQ(Result.raw(Token), Test.Spelling);
        ASSERT_TRUE(std::holds_alternative<char32_t>(Token.Payload));
        EXPECT_EQ(std::get<char32_t>(Token.Payload), Test.Value);
      }
    }

    // Verifies fixed-width byte escapes and braced Unicode escapes across their valid ranges.
    TEST(ScalarLiteralsTest, DecodesFixedWidthHexAndBracedUnicodeEscapes)
    {
      const std::vector<ValidScalarCase> Cases = {
          {"'\\x00'", U'\0'},
          {"'\\x1B'", U'\x1B'},
          {"'\\x7F'", U'\x7F'},
          {"'\\xFF'", U'\u00FF'},
          {"'\\u{0}'", U'\0'},
          {"'\\u{41}'", U'A'},
          {"'\\u{4E2D}'", U'\u4E2D'},
          {"'\\u{D7FF}'", U'\uD7FF'},
          {"'\\u{E000}'", U'\uE000'},
          {"'\\u{1F600}'", U'\U0001F600'},
          {"'\\u{10FFFF}'", U'\U0010FFFF'},
          {"'\\u{AD}'", static_cast<char32_t>(0x00AD)},
          {"'\\u{200B}'", U'\u200B'},
          {"'\\u{202E}'", U'\u202E'},
          {"'\\u{FEFF}'", U'\uFEFF'},
          {"'\\u{E0100}'", static_cast<char32_t>(0xE0100)},
      };

      for (const ValidScalarCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(TestSourceFileId, Test.Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        const Token &Token = Result.tokens().front();
        EXPECT_EQ(Token.Kind, TokenKind::ScalarLiteral);
        EXPECT_EQ(Result.raw(Token), Test.Spelling);
        ASSERT_TRUE(std::holds_alternative<char32_t>(Token.Payload));
        EXPECT_EQ(std::get<char32_t>(Token.Payload), Test.Value);
      }
    }

    // Verifies rejection of empty literals and spellings containing more than one scalar value.
    TEST(ScalarLiteralsTest, RejectsEmptyMultipleAndMultiScalarGraphemeSpellings)
    {
      const std::vector<InvalidScalarCase> Cases = {
          {"''", DiagnosticKind::EmptyScalarLiteral},
          {"'ab'", DiagnosticKind::MultipleScalarValues},
          {utf8(u8"'e\u0301'"), DiagnosticKind::MultipleScalarValues},
          {utf8(u8"'\U0001F1E8\U0001F1F3'"), DiagnosticKind::MultipleScalarValues},
      };

      for (const InvalidScalarCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(TestSourceFileId, Test.Spelling);
        ASSERT_FALSE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
        EXPECT_EQ(Result.raw(Result.tokens().front()), Test.Spelling);
        EXPECT_TRUE(hasDiagnostic(Result, Test.Diagnostic));
      }
    }

    // Verifies that an empty scalar is distinguished from a nonempty scalar whose decoding or escape validation fails.
    TEST(ScalarLiteralsTest, ReportsOnlyTheRootCauseForClosedNonemptyInvalidScalars)
    {
      struct RootDiagnosticCase
      {
        std::string Spelling;
        DiagnosticKind Kind;
        SourceRange Span;
      };

      std::string InvalidUtf8 = "'";
      InvalidUtf8.push_back(static_cast<char>(0x80));
      InvalidUtf8.push_back('\'');
      const std::vector<RootDiagnosticCase> Cases = {
          {"'\\q'", DiagnosticKind::UnknownEscape, {1, 3}},
          {"'\\u{D800}'", DiagnosticKind::InvalidUnicodeScalar, {1, 9}},
          {InvalidUtf8, DiagnosticKind::InvalidUtf8, {1, 2}},
      };

      for (const RootDiagnosticCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(TestSourceFileId, Test.Spelling);
        ASSERT_FALSE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
        EXPECT_EQ(Result.raw(Result.tokens().front()), Test.Spelling);
        ASSERT_EQ(Result.diagnostics().size(), 1U);
        EXPECT_EQ(Result.diagnostics().front().Kind, Test.Kind);
        EXPECT_EQ(Result.diagnostics().front().Span, Test.Span);
        EXPECT_FALSE(hasDiagnostic(Result, DiagnosticKind::EmptyScalarLiteral));
      }

      const TokenizedBuffer Empty = tokenize(TestSourceFileId, "''");
      ASSERT_FALSE(Empty.succeeded());
      ASSERT_EQ(Empty.diagnostics().size(), 1U);
      EXPECT_EQ(Empty.diagnostics().front().Kind, DiagnosticKind::EmptyScalarLiteral);
      EXPECT_EQ(Empty.diagnostics().front().Span, (SourceRange{0, 2}));
    }

    // Verifies that a large but reasonable scalar body is rejected as multiple values without truncation or extra diagnostics.
    TEST(ScalarLiteralsTest, RejectsLargeMultiScalarBodyWithoutTruncatingItsToken)
    {
      std::string Spelling = "'";
      Spelling.append(4096, 'a');
      Spelling.push_back('\'');
      const TokenizedBuffer Result = tokenize(TestSourceFileId, Spelling);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.tokens().size(), 2U);
      EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
      EXPECT_EQ(Result.raw(Result.tokens().front()), Spelling);
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics().front().Kind, DiagnosticKind::MultipleScalarValues);
    }

    // Verifies diagnostics for unknown escapes and malformed hexadecimal or Unicode escapes.
    TEST(ScalarLiteralsTest, RejectsUnknownAndMalformedEscapes)
    {
      const std::vector<InvalidScalarCase> Cases = {
          {"'\\q'", DiagnosticKind::UnknownEscape},
          {"'\\x1'", DiagnosticKind::InvalidHexEscape},
          {"'\\x_G'", DiagnosticKind::InvalidHexEscape},
          {"'\\u0041'", DiagnosticKind::InvalidUnicodeEscape},
          {"'\\u{}'", DiagnosticKind::InvalidUnicodeEscape},
          {"'\\u{1_F600}'", DiagnosticKind::InvalidUnicodeEscape},
          {"'\\u{ 41 }'", DiagnosticKind::InvalidUnicodeEscape},
          {"'\\u{0x41}'", DiagnosticKind::InvalidUnicodeEscape},
          {"'\\u{1234567}'", DiagnosticKind::InvalidUnicodeEscape},
      };

      for (const InvalidScalarCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(TestSourceFileId, Test.Spelling);
        ASSERT_FALSE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
        EXPECT_EQ(Result.raw(Result.tokens().front()), Test.Spelling);
        EXPECT_TRUE(hasDiagnostic(Result, Test.Diagnostic));
      }

      const TokenizedBuffer ExtraScalar = tokenize(TestSourceFileId, "'\\x123'");
      ASSERT_FALSE(ExtraScalar.succeeded());
      ASSERT_EQ(ExtraScalar.tokens().size(), 2U);
      EXPECT_EQ(ExtraScalar.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
      EXPECT_EQ(ExtraScalar.raw(ExtraScalar.tokens().front()), "'\\x123'");
      EXPECT_EQ(ExtraScalar.tokens().front().Span, (SourceRange{0, 7}));
      ASSERT_EQ(ExtraScalar.diagnostics().size(), 1U);
      EXPECT_EQ(ExtraScalar.diagnostics().front().Kind, DiagnosticKind::MultipleScalarValues);
      EXPECT_EQ(ExtraScalar.diagnostics().front().Span, (SourceRange{0, 7}));
    }

    // Verifies that a Unicode escape missing its right brace stops at the scalar quote and scanning resumes afterward.
    TEST(ScalarLiteralsTest, MissingUnicodeEscapeBraceRecoversAtClosingQuote)
    {
      const std::string Source = "'\\u{41'next";
      const TokenizedBuffer Result = tokenize(TestSourceFileId, Source);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.tokens().size(), 3U);
      EXPECT_EQ(Result.tokens()[0].Kind, TokenKind::InvalidScalarLiteral);
      EXPECT_EQ(Result.tokens()[0].Span, (SourceRange{0, 7}));
      EXPECT_EQ(Result.raw(Result.tokens()[0]), "'\\u{41'");
      EXPECT_EQ(Result.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Result.tokens()[1].Span, (SourceRange{7, 11}));
      EXPECT_EQ(Result.raw(Result.tokens()[1]), "next");
      EXPECT_EQ(Result.tokens()[2].Kind, TokenKind::EndOfFile);
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics().front().Kind, DiagnosticKind::InvalidUnicodeEscape);
      EXPECT_EQ(Result.diagnostics().front().Span, (SourceRange{1, 6}));
    }

    // Verifies that a trailing backslash at end of file reports both the incomplete escape and unterminated scalar.
    TEST(ScalarLiteralsTest, TrailingBackslashAtEofReportsBothRootDiagnostics)
    {
      const std::string Source = "'\\";
      const TokenizedBuffer Result = tokenize(TestSourceFileId, Source);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.tokens().size(), 2U);
      EXPECT_EQ(Result.tokens()[0].Kind, TokenKind::InvalidScalarLiteral);
      EXPECT_EQ(Result.tokens()[0].Span, (SourceRange{0, 2}));
      EXPECT_EQ(Result.raw(Result.tokens()[0]), Source);
      EXPECT_EQ(Result.tokens()[1].Kind, TokenKind::EndOfFile);
      ASSERT_EQ(Result.diagnostics().size(), 2U);
      EXPECT_EQ(Result.diagnostics()[0].Kind, DiagnosticKind::UnknownEscape);
      EXPECT_EQ(Result.diagnostics()[0].Span, (SourceRange{1, 2}));
      EXPECT_EQ(Result.diagnostics()[1].Kind, DiagnosticKind::UnterminatedScalarLiteral);
      EXPECT_EQ(Result.diagnostics()[1].Span, (SourceRange{0, 2}));
    }

    // Verifies that invalid UTF-8 immediately after a backslash is contained within the scalar and later tokens survive recovery.
    TEST(ScalarLiteralsTest, InvalidUtf8AfterBackslashRecoversAtClosingQuote)
    {
      std::string Source = "'\\";
      Source.push_back(static_cast<char>(0x80));
      Source += "'next";
      const TokenizedBuffer Result = tokenize(TestSourceFileId, Source);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.tokens().size(), 3U);
      EXPECT_EQ(Result.tokens()[0].Kind, TokenKind::InvalidScalarLiteral);
      EXPECT_EQ(Result.tokens()[0].Span, (SourceRange{0, 4}));
      EXPECT_EQ(Result.raw(Result.tokens()[0]), std::string_view(Source.data(), 4));
      EXPECT_EQ(Result.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Result.tokens()[1].Span, (SourceRange{4, 8}));
      EXPECT_EQ(Result.raw(Result.tokens()[1]), "next");
      EXPECT_EQ(Result.tokens()[2].Kind, TokenKind::EndOfFile);
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics().front().Kind, DiagnosticKind::InvalidUtf8);
      EXPECT_EQ(Result.diagnostics().front().Span, (SourceRange{2, 3}));
    }

    // Verifies that surrogate code points and values above the Unicode range are rejected.
    TEST(ScalarLiteralsTest, RejectsSurrogatesAndValuesBeyondUnicodeRange)
    {
      const std::vector<std::string> Spellings = {
          "'\\u{D800}'",
          "'\\u{DFFF}'",
          "'\\u{110000}'",
      };

      for (const std::string &Spelling : Spellings)
      {
        SCOPED_TRACE(Spelling);
        const TokenizedBuffer Result = tokenize(TestSourceFileId, Spelling);
        ASSERT_FALSE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
        EXPECT_EQ(Result.raw(Result.tokens().front()), Spelling);
        EXPECT_TRUE(hasDiagnostic(Result, DiagnosticKind::InvalidUnicodeScalar));
      }
    }

    // Verifies that direct invisible and control characters require explicit escape spellings.
    TEST(ScalarLiteralsTest, RequiresInvisibleAndControlCharactersToBeEscaped)
    {
      const std::vector<std::string> InvisibleCharacters = {
          "\xC2\xAD",
          "\xE2\x80\x8B",
          "\xE2\x80\xAE",
          "\xEF\xB8\x8F",
          "\xEF\xBB\xBF",
          "\xF3\xA0\x84\x80",
      };

      for (const std::string &Invisible : InvisibleCharacters)
      {
        const std::string Spelling = "'" + Invisible + "'";
        SCOPED_TRACE(Spelling);
        const TokenizedBuffer Result = tokenize(TestSourceFileId, Spelling);
        ASSERT_FALSE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
        EXPECT_EQ(Result.tokens().front().Span, (SourceRange{0, Spelling.size()}));
        EXPECT_EQ(Result.raw(Result.tokens().front()), Spelling);
        ASSERT_EQ(Result.diagnostics().size(), 1U);
        EXPECT_EQ(Result.diagnostics().front().Kind, DiagnosticKind::InvisibleCharacter);
        EXPECT_EQ(Result.diagnostics().front().Span, (SourceRange{1, 1 + Invisible.size()}));
      }

      std::string ControlSpelling = "'";
      ControlSpelling.push_back('\x01');
      ControlSpelling.push_back('\'');
      const TokenizedBuffer ControlResult = tokenize(TestSourceFileId, ControlSpelling);
      ASSERT_FALSE(ControlResult.succeeded());
      EXPECT_TRUE(hasDiagnostic(ControlResult, DiagnosticKind::ForbiddenControlCharacter));
      ASSERT_FALSE(ControlResult.tokens().empty());
      EXPECT_EQ(ControlResult.tokens().front().Kind, TokenKind::InvalidScalarLiteral);

      std::string CarriageReturnSpelling = "'";
      CarriageReturnSpelling.push_back('\r');
      CarriageReturnSpelling.push_back('\'');
      const TokenizedBuffer CarriageReturnResult = tokenize(TestSourceFileId, CarriageReturnSpelling);
      ASSERT_FALSE(CarriageReturnResult.succeeded());
      ASSERT_EQ(CarriageReturnResult.tokens().size(), 2U);
      EXPECT_EQ(CarriageReturnResult.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
      EXPECT_EQ(CarriageReturnResult.tokens().front().Span, (SourceRange{0, 3}));
      EXPECT_EQ(CarriageReturnResult.raw(CarriageReturnResult.tokens().front()), CarriageReturnSpelling);
      ASSERT_EQ(CarriageReturnResult.diagnostics().size(), 1U);
      EXPECT_EQ(CarriageReturnResult.diagnostics().front().Kind, DiagnosticKind::LoneCarriageReturn);
      EXPECT_EQ(CarriageReturnResult.diagnostics().front().Span, (SourceRange{1, 2}));
      EXPECT_EQ(CarriageReturnResult.lineStarts(), (std::vector<std::size_t>{0}));
    }

    // Verifies that an unterminated scalar stops before LF or CRLF and tokenization then resumes.
    TEST(ScalarLiteralsTest, StopsAnUnterminatedLiteralBeforeThePhysicalLineBreakAndContinuesScanning)
    {
      const TokenizedBuffer LfResult = tokenize(TestSourceFileId, "'a\nnext");
      ASSERT_FALSE(LfResult.succeeded());
      ASSERT_EQ(LfResult.tokens().size(), 4U);
      EXPECT_EQ(LfResult.tokens()[0].Kind, TokenKind::InvalidScalarLiteral);
      EXPECT_EQ(LfResult.raw(LfResult.tokens()[0]), "'a");
      EXPECT_TRUE(hasDiagnostic(LfResult, DiagnosticKind::UnterminatedScalarLiteral));
      EXPECT_EQ(LfResult.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(LfResult.raw(LfResult.tokens()[1]), "\n");
      EXPECT_EQ(LfResult.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_EQ(LfResult.raw(LfResult.tokens()[2]), "next");

      const TokenizedBuffer CrlfResult = tokenize(TestSourceFileId, "'a\r\nnext");
      ASSERT_FALSE(CrlfResult.succeeded());
      ASSERT_EQ(CrlfResult.tokens().size(), 4U);
      EXPECT_EQ(CrlfResult.tokens()[0].Kind, TokenKind::InvalidScalarLiteral);
      EXPECT_EQ(CrlfResult.raw(CrlfResult.tokens()[0]), "'a");
      EXPECT_EQ(CrlfResult.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(CrlfResult.raw(CrlfResult.tokens()[1]), "\r\n");
      EXPECT_EQ(CrlfResult.tokens()[2].Kind, TokenKind::Identifier);
    }

    // Verifies that end of file before a closing quote produces an unterminated scalar token.
    TEST(ScalarLiteralsTest, RejectsEndOfFileBeforeTheClosingQuote)
    {
      const TokenizedBuffer Result = tokenize(TestSourceFileId, "'a");
      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.tokens().size(), 2U);
      EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
      EXPECT_EQ(Result.raw(Result.tokens().front()), "'a");
      EXPECT_TRUE(hasDiagnostic(Result, DiagnosticKind::UnterminatedScalarLiteral));
    }

    // Verifies that adjacent scalar literals and following identifiers remain distinct tokens.
    TEST(ScalarLiteralsTest, KeepsAdjacentLiteralsAndFollowingIdentifiersAsSeparateTokens)
    {
      const TokenizedBuffer Adjacent = tokenize(TestSourceFileId, "'a''b'");
      ASSERT_TRUE(Adjacent.succeeded());
      ASSERT_EQ(Adjacent.tokens().size(), 3U);
      EXPECT_EQ(Adjacent.tokens()[0].Kind, TokenKind::ScalarLiteral);
      EXPECT_EQ(Adjacent.raw(Adjacent.tokens()[0]), "'a'");
      EXPECT_EQ(Adjacent.tokens()[1].Kind, TokenKind::ScalarLiteral);
      EXPECT_EQ(Adjacent.raw(Adjacent.tokens()[1]), "'b'");
      EXPECT_EQ(Adjacent.tokens()[2].Kind, TokenKind::EndOfFile);

      const TokenizedBuffer Suffixed = tokenize(TestSourceFileId, "'a'name");
      ASSERT_TRUE(Suffixed.succeeded());
      ASSERT_EQ(Suffixed.tokens().size(), 3U);
      EXPECT_EQ(Suffixed.tokens()[0].Kind, TokenKind::ScalarLiteral);
      EXPECT_EQ(Suffixed.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Suffixed.raw(Suffixed.tokens()[1]), "name");
    }
  } // namespace
} // namespace ink::tokenizer
