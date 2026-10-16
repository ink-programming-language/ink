#include "tokenizer_test_support.h"
#include "ink/core/config/numeric_literal.h"

#include <limits>

namespace ink::frontend::tokenizer::test
{
  // Numeric metadata accepts exactly the four bases supported by numeric literal prefixes.
  TEST(TokenizerNumericInfoTest, SupportedBases)
  {
    constexpr NumericInfo Decimal;
    static_assert(Decimal.hasValidBase());
    static_assert(noexcept(Decimal.hasValidBase()));
    for (const core::NumericBase Base : {core::NumericBase::Binary, core::NumericBase::Octal, core::NumericBase::Decimal, core::NumericBase::Hexadecimal})
    {
      EXPECT_TRUE(NumericInfo{Base}.hasValidBase()) << static_cast<unsigned>(Base);
    }
    for (const unsigned Base : {0U, 1U, 3U, 7U, 9U, 11U, 15U, 17U, 36U, std::numeric_limits<unsigned>::max()})
    {
      EXPECT_FALSE(NumericInfo{static_cast<core::NumericBase>(Base)}.hasValidBase()) << Base;
    }
  }

  // Restored integer and float tokens reject unsupported base metadata before exposing a token buffer.
  TEST_F(TokenizerTest, SnapshotRejectsUnsupportedNumericBases)
  {
    for (const TokenKind Kind : {TokenKind::IntegerLiteral, TokenKind::FloatLiteral})
    {
      const std::vector<Token> Tokens = {
          {Kind, core::SourceRange::fromByteOffsets(0, 1), NumericInfo{static_cast<core::NumericBase>(3)}},
          {TokenKind::EndOfFile, core::SourceRange::fromByteOffsets(1, 1), {}},
      };
      EXPECT_FALSE(TokenizedBuffer::fromSnapshot(Sources, "invalid-base", "1", Tokens, true).has_value());
    }
  }

  // Decimal leading zeroes remain decimal; prefixes select only binary, octal and hexadecimal.
  TEST_F(TokenizerTest, IntegerFormsAndBases)
  {
    struct Case
    {
        const char *Source;
        core::NumericBase Base;
    };
    const Case Cases[] = {
        {"0", core::NumericBase::Decimal},
        {"0009", core::NumericBase::Decimal},
        {"1234567890", core::NumericBase::Decimal},
        {"0b101", core::NumericBase::Binary},
        {"0B0", core::NumericBase::Binary},
        {"0o765", core::NumericBase::Octal},
        {"0O0", core::NumericBase::Octal},
        {"0xabcdef", core::NumericBase::Hexadecimal},
        {"0X0123456789ABCDEF", core::NumericBase::Hexadecimal},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      const auto Buffer = lex(Entry.Source);
      expectKinds(Buffer, {TokenKind::IntegerLiteral});
      EXPECT_EQ(std::get<NumericInfo>(Buffer.tokens()[0].Payload).Base, Entry.Base);
    }
  }

  // Decimal exponents and fractions are floats, and signs outside an exponent are separate operators.
  TEST_F(TokenizerTest, DecimalFloatForms)
  {
    for (const auto &Source : {"1e3", "1.0", "1.0e3", "1e99999", "00.09", "1E+2", "1e-2", "0.0E-0001"})
    {
      expectKinds(lex(Source), {TokenKind::FloatLiteral});
    }
    expectKinds(lex("-1 +2 -1e-3 +2.0E+4"), {TokenKind::Minus, TokenKind::IntegerLiteral, TokenKind::Plus, TokenKind::IntegerLiteral, TokenKind::Minus, TokenKind::FloatLiteral, TokenKind::Plus, TokenKind::FloatLiteral});
  }

  // A decimal point needs digits on both sides; dots and ellipses retain their own token boundaries.
  TEST_F(TokenizerTest, DecimalPointBoundaries)
  {
    expectKinds(lex(".5 1. 1..2 1...2 1.2.3 0xFF.member"), {TokenKind::Dot, TokenKind::IntegerLiteral, TokenKind::IntegerLiteral, TokenKind::Dot, TokenKind::IntegerLiteral, TokenKind::Dot, TokenKind::Dot, TokenKind::IntegerLiteral, TokenKind::IntegerLiteral, TokenKind::Ellipsis, TokenKind::IntegerLiteral, TokenKind::FloatLiteral, TokenKind::Dot, TokenKind::IntegerLiteral, TokenKind::IntegerLiteral, TokenKind::Dot, TokenKind::Identifier});
  }

  // Lexical validation never parses a numeric value into a fixed-width host number.
  TEST_F(TokenizerTest, ArbitrarilyLargeNumbersAreLexicallyValid)
  {
    expectKinds(lex(std::string(10000, '9')), {TokenKind::IntegerLiteral});
    expectKinds(lex("0x" + std::string(10000, 'F')), {TokenKind::IntegerLiteral});
    expectKinds(lex("1e" + std::string(10000, '9')), {TokenKind::FloatLiteral});
  }

  // Missing base digits and invalid digits fail the entire numeric token without backtracking.
  TEST_F(TokenizerTest, BaseErrorsDoNotSplitIntoValidTokens)
  {
    for (const auto &Source : {"0b", "0B", "0o", "0O", "0x", "0X", "0xG"})
    {
      expectError(Source, DiagnosticKind::MissingBaseDigits);
    }
    for (const auto &Source : {"0b2", "0b102", "0o8", "0O778"})
    {
      expectError(Source, DiagnosticKind::DigitOutOfRange);
    }
    for (const auto &Source : {"0x1p2", "0x1P+2", "0x1.fp2", "0x1.2", "0x1.p2"})
    {
      expectError(Source, DiagnosticKind::HexadecimalFloat);
    }
  }

  // Exponent syntax requires ASCII decimal digits after its optional sign.
  TEST_F(TokenizerTest, IncompleteExponentsFail)
  {
    for (const auto &Source : {"1e", "1E+", "1e-", "1.0e", "1.0E-foo", "1e+ 2"})
    {
      expectError(Source, DiagnosticKind::MissingExponentDigits);
    }
    expectError("1e" + utf8(0x0661), DiagnosticKind::MissingExponentDigits);
  }

  // Any adjacent XID continuation, including Unicode letters or marks, is an illegal numeric suffix.
  TEST_F(TokenizerTest, NumericSeparatorsAndSuffixesFail)
  {
    for (const auto &Source : {"1_000", "1_", "0xFF_u8", "1f", "1u32", "1.0d", "1e3abc", "0o7z", "0b1a", "1r\"x\""})
    {
      expectError(Source, DiagnosticKind::InvalidNumericSuffix);
    }
    for (const char32_t Scalar : {U'\u03B1', U'\u4E2D', U'\u0301', U'\u0661', U'\U0002EBF0'})
    {
      expectError("1" + utf8(Scalar), DiagnosticKind::InvalidNumericSuffix);
      expectError("1e2" + utf8(Scalar), DiagnosticKind::InvalidNumericSuffix);
    }
  }

  // A failed number after a legal prefix never becomes a valid integer or a synthetic EOF.
  TEST_F(TokenizerTest, NumericFailurePreservesOnlyCompletedTokens)
  {
    const auto Buffer = lex("var x = 12abc");
    EXPECT_FALSE(Buffer.succeeded());
    ASSERT_EQ(Buffer.tokens().size(), 3U);
    EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::KwVar);
    EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
    EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::Assign);
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Span, core::SourceRange::fromByteOffsets(8, 13));
  }
} // namespace ink::frontend::tokenizer::test
