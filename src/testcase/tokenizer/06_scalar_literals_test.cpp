#include "ink/tokenizer/tokenizer.h"

#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
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

    bool hasDiagnostic(const LexedFile &Result, DiagnosticKind Kind)
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
          {utf8(u8"'\U0001F600'"), U'\U0001F600'},
      };

      for (const ValidScalarCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const LexedFile Result = tokenize(Test.Spelling);
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
        const LexedFile Result = tokenize(Test.Spelling);
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
          {"'\\u{1F600}'", U'\U0001F600'},
          {"'\\u{200B}'", U'\u200B'},
          {"'\\u{202E}'", U'\u202E'},
      };

      for (const ValidScalarCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const LexedFile Result = tokenize(Test.Spelling);
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
        const LexedFile Result = tokenize(Test.Spelling);
        ASSERT_FALSE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
        EXPECT_EQ(Result.raw(Result.tokens().front()), Test.Spelling);
        EXPECT_TRUE(hasDiagnostic(Result, Test.Diagnostic));
      }
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
        const LexedFile Result = tokenize(Test.Spelling);
        ASSERT_FALSE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
        EXPECT_EQ(Result.raw(Result.tokens().front()), Test.Spelling);
        EXPECT_TRUE(hasDiagnostic(Result, Test.Diagnostic));
      }

      const LexedFile ExtraScalar = tokenize("'\\x123'");
      ASSERT_FALSE(ExtraScalar.succeeded());
      ASSERT_EQ(ExtraScalar.tokens().size(), 2U);
      EXPECT_EQ(ExtraScalar.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
      EXPECT_EQ(ExtraScalar.raw(ExtraScalar.tokens().front()), "'\\x123'");
      EXPECT_TRUE(hasDiagnostic(ExtraScalar, DiagnosticKind::MultipleScalarValues) || hasDiagnostic(ExtraScalar, DiagnosticKind::InvalidHexEscape));
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
        const LexedFile Result = tokenize(Spelling);
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
      const std::vector<std::string> InvisibleSpellings = {
          utf8(u8"'\u200B'"),
          utf8(u8"'\u202E'"),
          utf8(u8"'\uFE0F'"),
      };

      for (const std::string &Spelling : InvisibleSpellings)
      {
        SCOPED_TRACE(Spelling);
        const LexedFile Result = tokenize(Spelling);
        ASSERT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasDiagnostic(Result, DiagnosticKind::InvisibleCharacter));
        ASSERT_FALSE(Result.tokens().empty());
        EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
      }

      std::string ControlSpelling = "'";
      ControlSpelling.push_back('\x01');
      ControlSpelling.push_back('\'');
      const LexedFile ControlResult = tokenize(ControlSpelling);
      ASSERT_FALSE(ControlResult.succeeded());
      EXPECT_TRUE(hasDiagnostic(ControlResult, DiagnosticKind::ForbiddenControlCharacter));
      ASSERT_FALSE(ControlResult.tokens().empty());
      EXPECT_EQ(ControlResult.tokens().front().Kind, TokenKind::InvalidScalarLiteral);

      std::string CarriageReturnSpelling = "'";
      CarriageReturnSpelling.push_back('\r');
      CarriageReturnSpelling.push_back('\'');
      const LexedFile CarriageReturnResult = tokenize(CarriageReturnSpelling);
      ASSERT_FALSE(CarriageReturnResult.succeeded());
      EXPECT_TRUE(hasDiagnostic(CarriageReturnResult, DiagnosticKind::LoneCarriageReturn));
      ASSERT_FALSE(CarriageReturnResult.tokens().empty());
      EXPECT_EQ(CarriageReturnResult.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
    }

    // Verifies that an unterminated scalar stops before LF or CRLF and tokenization then resumes.
    TEST(ScalarLiteralsTest, StopsAnUnterminatedLiteralBeforeThePhysicalLineBreakAndContinuesScanning)
    {
      const LexedFile LfResult = tokenize("'a\nnext");
      ASSERT_FALSE(LfResult.succeeded());
      ASSERT_EQ(LfResult.tokens().size(), 4U);
      EXPECT_EQ(LfResult.tokens()[0].Kind, TokenKind::InvalidScalarLiteral);
      EXPECT_EQ(LfResult.raw(LfResult.tokens()[0]), "'a");
      EXPECT_TRUE(hasDiagnostic(LfResult, DiagnosticKind::UnterminatedScalarLiteral));
      EXPECT_EQ(LfResult.tokens()[1].Kind, TokenKind::LineBreak);
      EXPECT_EQ(LfResult.raw(LfResult.tokens()[1]), "\n");
      EXPECT_EQ(LfResult.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_EQ(LfResult.raw(LfResult.tokens()[2]), "next");

      const LexedFile CrlfResult = tokenize("'a\r\nnext");
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
      const LexedFile Result = tokenize("'a");
      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.tokens().size(), 2U);
      EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidScalarLiteral);
      EXPECT_EQ(Result.raw(Result.tokens().front()), "'a");
      EXPECT_TRUE(hasDiagnostic(Result, DiagnosticKind::UnterminatedScalarLiteral));
    }

    // Verifies that adjacent scalar literals and following identifiers remain distinct tokens.
    TEST(ScalarLiteralsTest, KeepsAdjacentLiteralsAndFollowingIdentifiersAsSeparateTokens)
    {
      const LexedFile Adjacent = tokenize("'a''b'");
      ASSERT_TRUE(Adjacent.succeeded());
      ASSERT_EQ(Adjacent.tokens().size(), 3U);
      EXPECT_EQ(Adjacent.tokens()[0].Kind, TokenKind::ScalarLiteral);
      EXPECT_EQ(Adjacent.raw(Adjacent.tokens()[0]), "'a'");
      EXPECT_EQ(Adjacent.tokens()[1].Kind, TokenKind::ScalarLiteral);
      EXPECT_EQ(Adjacent.raw(Adjacent.tokens()[1]), "'b'");
      EXPECT_EQ(Adjacent.tokens()[2].Kind, TokenKind::EndOfFile);

      const LexedFile Suffixed = tokenize("'a'name");
      ASSERT_TRUE(Suffixed.succeeded());
      ASSERT_EQ(Suffixed.tokens().size(), 3U);
      EXPECT_EQ(Suffixed.tokens()[0].Kind, TokenKind::ScalarLiteral);
      EXPECT_EQ(Suffixed.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Suffixed.raw(Suffixed.tokens()[1]), "name");
    }
  } // namespace
} // namespace ink::tokenizer
