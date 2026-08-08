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

    struct ValidNumericCase
    {
      const char *Spelling;
      TokenKind Kind;
      unsigned Base;
      NumericSuffix Suffix;
    };

    struct InvalidNumericCase
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

    std::vector<const Token *> syntaxTokens(const TokenizedBuffer &Result)
    {
      std::vector<const Token *> Tokens;
      for (const Token &Token : Result.tokens())
      {
        if (!Token.isTrivia() && Token.Kind != TokenKind::EndOfFile)
        {
          Tokens.push_back(&Token);
        }
      }
      return Tokens;
    }

    // Verifies integer literals in every base, including grouped digits and leading zeroes.
    TEST(NumericLiteralsTest, AcceptsAllIntegerBasesGroupingAndLeadingZeroForms)
    {
      const std::vector<ValidNumericCase> Cases = {
          {"123", TokenKind::IntegerLiteral, 10, NumericSuffix::None},
          {"0", TokenKind::IntegerLiteral, 10, NumericSuffix::None},
          {"00", TokenKind::IntegerLiteral, 10, NumericSuffix::None},
          {"0010", TokenKind::IntegerLiteral, 10, NumericSuffix::None},
          {"1_000_000", TokenKind::IntegerLiteral, 10, NumericSuffix::None},
          {"0b1010", TokenKind::IntegerLiteral, 2, NumericSuffix::None},
          {"0b1111_0000", TokenKind::IntegerLiteral, 2, NumericSuffix::None},
          {"0o755", TokenKind::IntegerLiteral, 8, NumericSuffix::None},
          {"0xFF_A0", TokenKind::IntegerLiteral, 16, NumericSuffix::None},
          {"0xabcdef", TokenKind::IntegerLiteral, 16, NumericSuffix::None},
          {"0xABCDEF", TokenKind::IntegerLiteral, 16, NumericSuffix::None},
      };

      for (const ValidNumericCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(Test.Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        const Token &Token = Result.tokens().front();
        EXPECT_EQ(Token.Kind, Test.Kind);
        EXPECT_EQ(Token.Span, (SourceRange{0, std::string(Test.Spelling).size()}));
        EXPECT_EQ(Result.raw(Token), Test.Spelling);
        ASSERT_TRUE(std::holds_alternative<NumericInfo>(Token.Payload));
        EXPECT_EQ(std::get<NumericInfo>(Token.Payload).Base, Test.Base);
        EXPECT_EQ(std::get<NumericInfo>(Token.Payload).Suffix, Test.Suffix);
      }
    }

    // Verifies that each supported integer suffix is consumed as part of one numeric token.
    TEST(NumericLiteralsTest, AcceptsEveryIntegerSuffixAsPartOfOneToken)
    {
      const std::vector<ValidNumericCase> Cases = {
          {"10i8", TokenKind::IntegerLiteral, 10, NumericSuffix::I8},
          {"10i16", TokenKind::IntegerLiteral, 10, NumericSuffix::I16},
          {"10i32", TokenKind::IntegerLiteral, 10, NumericSuffix::I32},
          {"10i64", TokenKind::IntegerLiteral, 10, NumericSuffix::I64},
          {"10i128", TokenKind::IntegerLiteral, 10, NumericSuffix::I128},
          {"10u8", TokenKind::IntegerLiteral, 10, NumericSuffix::U8},
          {"10u16", TokenKind::IntegerLiteral, 10, NumericSuffix::U16},
          {"10u32", TokenKind::IntegerLiteral, 10, NumericSuffix::U32},
          {"10u64", TokenKind::IntegerLiteral, 10, NumericSuffix::U64},
          {"10u128", TokenKind::IntegerLiteral, 10, NumericSuffix::U128},
          {"10int", TokenKind::IntegerLiteral, 10, NumericSuffix::Int},
          {"10uint", TokenKind::IntegerLiteral, 10, NumericSuffix::UInt},
          {"4_096ptrsize", TokenKind::IntegerLiteral, 10, NumericSuffix::PtrSize},
          {"10byte", TokenKind::IntegerLiteral, 10, NumericSuffix::Byte},
          {"0xFFFFu32", TokenKind::IntegerLiteral, 16, NumericSuffix::U32},
          {"0b1010byte", TokenKind::IntegerLiteral, 2, NumericSuffix::Byte},
      };

      for (const ValidNumericCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(Test.Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        const Token &Token = Result.tokens().front();
        EXPECT_EQ(Token.Kind, TokenKind::IntegerLiteral);
        EXPECT_EQ(Result.raw(Token), Test.Spelling);
        ASSERT_TRUE(std::holds_alternative<NumericInfo>(Token.Payload));
        EXPECT_EQ(std::get<NumericInfo>(Token.Payload).Base, Test.Base);
        EXPECT_EQ(std::get<NumericInfo>(Token.Payload).Suffix, Test.Suffix);
      }
    }

    // Verifies decimal floating-point forms, digit grouping, exponents, and float suffixes.
    TEST(NumericLiteralsTest, AcceptsDecimalFloatFormsGroupingAndEveryFloatSuffix)
    {
      const std::vector<ValidNumericCase> Cases = {
          {"1.0", TokenKind::FloatLiteral, 10, NumericSuffix::None},
          {"0.5", TokenKind::FloatLiteral, 10, NumericSuffix::None},
          {"1.25e10", TokenKind::FloatLiteral, 10, NumericSuffix::None},
          {"1e10", TokenKind::FloatLiteral, 10, NumericSuffix::None},
          {"1.5e-3", TokenKind::FloatLiteral, 10, NumericSuffix::None},
          {"1.5E+3", TokenKind::FloatLiteral, 10, NumericSuffix::None},
          {"1.234_567", TokenKind::FloatLiteral, 10, NumericSuffix::None},
          {"1e10_000", TokenKind::FloatLiteral, 10, NumericSuffix::None},
          {"10f16", TokenKind::FloatLiteral, 10, NumericSuffix::F16},
          {"10f32", TokenKind::FloatLiteral, 10, NumericSuffix::F32},
          {"10f64", TokenKind::FloatLiteral, 10, NumericSuffix::F64},
          {"1.5f32", TokenKind::FloatLiteral, 10, NumericSuffix::F32},
          {"1e10f64", TokenKind::FloatLiteral, 10, NumericSuffix::F64},
      };

      for (const ValidNumericCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(Test.Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        const Token &Token = Result.tokens().front();
        EXPECT_EQ(Token.Kind, TokenKind::FloatLiteral);
        EXPECT_EQ(Result.raw(Token), Test.Spelling);
        ASSERT_TRUE(std::holds_alternative<NumericInfo>(Token.Payload));
        EXPECT_EQ(std::get<NumericInfo>(Token.Payload).Base, 10U);
        EXPECT_EQ(std::get<NumericInfo>(Token.Payload).Suffix, Test.Suffix);
      }
    }

    // Verifies that hexadecimal digits take precedence over suffix-like trailing text.
    TEST(NumericLiteralsTest, UsesTheLongestHexDigitSequenceBeforeConsideringSuffixes)
    {
      const TokenizedBuffer Result = tokenize("0x10f32");
      ASSERT_TRUE(Result.succeeded());
      ASSERT_EQ(Result.tokens().size(), 2U);
      const Token &Token = Result.tokens().front();
      EXPECT_EQ(Token.Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(Result.raw(Token), "0x10f32");
      ASSERT_TRUE(std::holds_alternative<NumericInfo>(Token.Payload));
      EXPECT_EQ(std::get<NumericInfo>(Token.Payload).Base, 16U);
      EXPECT_EQ(std::get<NumericInfo>(Token.Payload).Suffix, NumericSuffix::None);
    }

    // Verifies that signs and incomplete decimal points remain separate syntax tokens.
    TEST(NumericLiteralsTest, KeepsLeadingSignsAndIncompleteDecimalPointsInSeparateTokens)
    {
      const TokenizedBuffer Result = tokenize("-128i8 +10 .5 1. 1.member 1..10");
      ASSERT_TRUE(Result.succeeded());
      const std::vector<const Token *> Tokens = syntaxTokens(Result);
      ASSERT_EQ(Tokens.size(), 15U);
      EXPECT_EQ(Tokens[0]->Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Tokens[0]->Payload), '-');
      EXPECT_EQ(Tokens[1]->Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(Result.raw(*Tokens[1]), "128i8");
      EXPECT_EQ(Tokens[2]->Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Tokens[2]->Payload), '+');
      EXPECT_EQ(Tokens[3]->Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(Result.raw(*Tokens[3]), "10");
      EXPECT_EQ(Tokens[4]->Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Tokens[4]->Payload), '.');
      EXPECT_EQ(Tokens[5]->Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(Result.raw(*Tokens[5]), "5");
      EXPECT_EQ(Tokens[6]->Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(Result.raw(*Tokens[6]), "1");
      EXPECT_EQ(Tokens[7]->Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Tokens[7]->Payload), '.');
      EXPECT_EQ(Tokens[8]->Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(Result.raw(*Tokens[8]), "1");
      EXPECT_EQ(Tokens[9]->Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Tokens[9]->Payload), '.');
      EXPECT_EQ(Tokens[10]->Kind, TokenKind::Identifier);
      EXPECT_EQ(Result.raw(*Tokens[10]), "member");
      EXPECT_EQ(Tokens[11]->Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(Result.raw(*Tokens[11]), "1");
      EXPECT_EQ(Tokens[12]->Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Tokens[12]->Payload), '.');
      EXPECT_EQ(Tokens[13]->Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Tokens[13]->Payload), '.');
      EXPECT_EQ(Tokens[14]->Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(Result.raw(*Tokens[14]), "10");
    }

    // Verifies that arbitrarily long coefficients are preserved without host integer conversion.
    TEST(NumericLiteralsTest, PreservesArbitrarilyLongCoefficientsWithoutHostOverflow)
    {
      const std::string Spelling = "1234567890123456789012345678901234567890123456789012345678901234567890";
      const TokenizedBuffer Result = tokenize(Spelling);
      ASSERT_TRUE(Result.succeeded());
      ASSERT_EQ(Result.tokens().size(), 2U);
      EXPECT_EQ(Result.tokens().front().Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(Result.raw(Result.tokens().front()), Spelling);
    }

    // Verifies malformed numeric candidates produce precise diagnostics and full candidate spans.
    TEST(NumericLiteralsTest, RejectsMalformedNumbersWithSpecificDiagnosticsAndFullCandidateSpans)
    {
      const std::vector<InvalidNumericCase> Cases = {
          {"0b", DiagnosticKind::MissingBaseDigits},
          {"0o", DiagnosticKind::MissingBaseDigits},
          {"0x", DiagnosticKind::MissingBaseDigits},
          {"0xG", DiagnosticKind::MissingBaseDigits},
          {"0b2", DiagnosticKind::DigitOutOfRange},
          {"0o8", DiagnosticKind::DigitOutOfRange},
          {"100_", DiagnosticKind::MisplacedNumericSeparator},
          {"1__000", DiagnosticKind::MisplacedNumericSeparator},
          {"0x_FF", DiagnosticKind::MisplacedNumericSeparator},
          {"1_.0", DiagnosticKind::MisplacedNumericSeparator},
          {"1._0", DiagnosticKind::MisplacedNumericSeparator},
          {"1e_10", DiagnosticKind::MisplacedNumericSeparator},
          {"1e10_", DiagnosticKind::MisplacedNumericSeparator},
          {"10_i32", DiagnosticKind::MisplacedNumericSeparator},
          {"1e", DiagnosticKind::MissingExponentDigits},
          {"1e+", DiagnosticKind::MissingExponentDigits},
          {"1e-", DiagnosticKind::MissingExponentDigits},
          {"10foo", DiagnosticKind::UnknownNumericSuffix},
          {"1.0meters", DiagnosticKind::UnknownNumericSuffix},
          {"0x12u7", DiagnosticKind::UnknownNumericSuffix},
          {utf8(u8"10\u7528\u6237"), DiagnosticKind::UnknownNumericSuffix},
          {"1.0i32", DiagnosticKind::InvalidNumericSuffix},
          {"1e3u8", DiagnosticKind::InvalidNumericSuffix},
          {"0b1f32", DiagnosticKind::InvalidNumericSuffix},
          {"0o7f64", DiagnosticKind::InvalidNumericSuffix},
          {"0b1.0", DiagnosticKind::UnsupportedNonDecimalFloat},
          {"0xA.F", DiagnosticKind::UnsupportedNonDecimalFloat},
          {"0b1e2", DiagnosticKind::UnsupportedNonDecimalFloat},
          {"0x1p4", DiagnosticKind::UnsupportedNonDecimalFloat},
      };

      for (const InvalidNumericCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(Test.Spelling);
        ASSERT_FALSE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        const Token &Token = Result.tokens().front();
        EXPECT_EQ(Token.Kind, TokenKind::InvalidNumber);
        EXPECT_EQ(Token.Span, (SourceRange{0, Test.Spelling.size()}));
        EXPECT_EQ(Result.raw(Token), Test.Spelling);
        EXPECT_TRUE(hasDiagnostic(Result, Test.Diagnostic));
        EXPECT_EQ(Result.tokens().back().Kind, TokenKind::EndOfFile);
      }
    }

    // Verifies exact recovery across malformed numbers followed by a delimiter, trivia, and a valid type token.
    TEST(NumericLiteralsTest, RecoversAfterMalformedNumbersAtDelimiterAndTriviaBoundaries)
    {
      const std::string Source = "0b1f32, 0x1p4 i32";
      const TokenizedBuffer Result = tokenize(Source);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.tokens().size(), 7U);
      EXPECT_EQ(Result.tokens()[0].Kind, TokenKind::InvalidNumber);
      EXPECT_EQ(Result.tokens()[0].Span, (SourceRange{0, 6}));
      EXPECT_EQ(Result.raw(Result.tokens()[0]), "0b1f32");
      EXPECT_EQ(Result.tokens()[1].Kind, TokenKind::Symbol);
      EXPECT_EQ(Result.tokens()[1].Span, (SourceRange{6, 7}));
      EXPECT_EQ(Result.raw(Result.tokens()[1]), ",");
      ASSERT_TRUE(std::holds_alternative<char>(Result.tokens()[1].Payload));
      EXPECT_EQ(std::get<char>(Result.tokens()[1].Payload), ',');
      EXPECT_EQ(Result.tokens()[2].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Result.tokens()[2].Span, (SourceRange{7, 8}));
      EXPECT_EQ(Result.raw(Result.tokens()[2]), " ");
      EXPECT_EQ(Result.tokens()[3].Kind, TokenKind::InvalidNumber);
      EXPECT_EQ(Result.tokens()[3].Span, (SourceRange{8, 13}));
      EXPECT_EQ(Result.raw(Result.tokens()[3]), "0x1p4");
      EXPECT_EQ(Result.tokens()[4].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Result.tokens()[4].Span, (SourceRange{13, 14}));
      EXPECT_EQ(Result.raw(Result.tokens()[4]), " ");
      EXPECT_EQ(Result.tokens()[5].Kind, TokenKind::BuiltinType);
      EXPECT_EQ(Result.tokens()[5].Span, (SourceRange{14, 17}));
      EXPECT_EQ(Result.raw(Result.tokens()[5]), "i32");
      ASSERT_TRUE(std::holds_alternative<BuiltinTypeKind>(Result.tokens()[5].Payload));
      EXPECT_EQ(std::get<BuiltinTypeKind>(Result.tokens()[5].Payload), BuiltinTypeKind::I32);
      EXPECT_EQ(Result.tokens()[6].Kind, TokenKind::EndOfFile);
      EXPECT_EQ(Result.tokens()[6].Span, (SourceRange{17, 17}));
      EXPECT_EQ(Result.raw(Result.tokens()[6]), "");

      const std::vector<Diagnostic> ExpectedDiagnostics = {
          {DiagnosticKind::InvalidNumericSuffix, {3, 6}, "numeric suffix is not valid for this literal"},
          {DiagnosticKind::UnsupportedNonDecimalFloat, {11, 13}, "non-decimal floating-point literals are not supported"},
      };
      EXPECT_EQ(Result.diagnostics(), ExpectedDiagnostics);
    }

    // Verifies that malformed base digits and exponent tails report only their primary numeric diagnostic.
    TEST(NumericLiteralsTest, ReportsOnlyThePrimaryDiagnosticForMalformedNumericCandidates)
    {
      const std::vector<InvalidNumericCase> Cases = {
          {"0b2", DiagnosticKind::DigitOutOfRange},
          {"0o8", DiagnosticKind::DigitOutOfRange},
          {"0xG", DiagnosticKind::MissingBaseDigits},
          {"1efoo", DiagnosticKind::MissingExponentDigits},
      };

      for (const InvalidNumericCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(Test.Spelling);
        ASSERT_FALSE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidNumber);
        EXPECT_EQ(Result.raw(Result.tokens().front()), Test.Spelling);
        ASSERT_EQ(Result.diagnostics().size(), 1U);
        EXPECT_EQ(Result.diagnostics().front().Kind, Test.Diagnostic);
      }
    }

    // Verifies that uppercase base prefixes are rejected as invalid numeric candidates.
    TEST(NumericLiteralsTest, RejectsUppercaseBasePrefixesInsteadOfTreatingThemAsValidPrefixes)
    {
      const std::vector<std::string> Spellings = {
          "0B10",
          "0O7",
          "0XFF",
      };

      for (const std::string &Spelling : Spellings)
      {
        SCOPED_TRACE(Spelling);
        const TokenizedBuffer Result = tokenize(Spelling);
        ASSERT_FALSE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidNumber);
        EXPECT_EQ(Result.raw(Result.tokens().front()), Spelling);
      }
    }

    // Verifies that invisible Unicode characters in suffix candidates are diagnosed.
    TEST(NumericLiteralsTest, RejectsInvisibleCharactersInsideNumericSuffixCandidates)
    {
      const std::string Source = utf8(u8"1a\u200Cb");
      const TokenizedBuffer Result = tokenize(Source);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.tokens().size(), 2U);
      EXPECT_EQ(Result.tokens().front().Kind, TokenKind::InvalidNumber);
      EXPECT_EQ(Result.raw(Result.tokens().front()), Source);
      ASSERT_EQ(Result.diagnostics().size(), 1U);
      EXPECT_EQ(Result.diagnostics().front().Kind, DiagnosticKind::InvisibleCharacter);
    }

    // Verifies that trivia prevents a following built-in type from becoming a numeric suffix.
    TEST(NumericLiteralsTest, TriviaSeparatesATypeNameFromTheLiteralSuffixCandidate)
    {
      const TokenizedBuffer Result = tokenize("10 i32");
      ASSERT_TRUE(Result.succeeded());
      ASSERT_EQ(Result.tokens().size(), 4U);
      EXPECT_EQ(Result.tokens()[0].Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(Result.raw(Result.tokens()[0]), "10");
      EXPECT_EQ(Result.tokens()[1].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Result.tokens()[2].Kind, TokenKind::BuiltinType);
      EXPECT_EQ(Result.raw(Result.tokens()[2]), "i32");
      EXPECT_EQ(Result.tokens()[3].Kind, TokenKind::EndOfFile);
    }
  } // namespace
} // namespace ink::tokenizer
