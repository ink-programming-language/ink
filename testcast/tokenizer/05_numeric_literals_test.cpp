#include "ink/tokenizer/tokenizer.h"

#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <variant>
#include <vector>

namespace ink::tokenizer {
namespace {

struct ValidNumericCase {
  const char* spelling;
  TokenKind kind;
  unsigned base;
  NumericSuffix suffix;
};

struct InvalidNumericCase {
  std::string spelling;
  DiagnosticKind diagnostic;
};

bool has_diagnostic(const LexedFile& result, DiagnosticKind kind) {
  return std::any_of(result.diagnostics().begin(), result.diagnostics().end(), [kind](const Diagnostic& diagnostic) { return diagnostic.kind == kind; });
}

std::vector<const Token*> syntax_tokens(const LexedFile& result) {
  std::vector<const Token*> tokens;
  for (const Token& token : result.tokens()) {
    if (!token.is_trivia() && token.kind != TokenKind::EndOfFile) {
      tokens.push_back(&token);
    }
  }
  return tokens;
}

TEST(NumericLiteralsTest, AcceptsAllIntegerBasesGroupingAndLeadingZeroForms) {
  const std::vector<ValidNumericCase> cases = {{"123", TokenKind::IntegerLiteral, 10, NumericSuffix::None}, {"0", TokenKind::IntegerLiteral, 10, NumericSuffix::None}, {"00", TokenKind::IntegerLiteral, 10, NumericSuffix::None}, {"0010", TokenKind::IntegerLiteral, 10, NumericSuffix::None}, {"1_000_000", TokenKind::IntegerLiteral, 10, NumericSuffix::None}, {"0b1010", TokenKind::IntegerLiteral, 2, NumericSuffix::None}, {"0b1111_0000", TokenKind::IntegerLiteral, 2, NumericSuffix::None}, {"0o755", TokenKind::IntegerLiteral, 8, NumericSuffix::None}, {"0xFF_A0", TokenKind::IntegerLiteral, 16, NumericSuffix::None}, {"0xabcdef", TokenKind::IntegerLiteral, 16, NumericSuffix::None}, {"0xABCDEF", TokenKind::IntegerLiteral, 16, NumericSuffix::None}};

  for (const ValidNumericCase& test : cases) {
    SCOPED_TRACE(test.spelling);
    const LexedFile result = tokenize(test.spelling);
    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    const Token& token = result.tokens().front();
    EXPECT_EQ(token.kind, test.kind);
    EXPECT_EQ(token.span, (ByteSpan{0, std::string(test.spelling).size()}));
    EXPECT_EQ(result.raw(token), test.spelling);
    ASSERT_TRUE(std::holds_alternative<NumericInfo>(token.payload));
    EXPECT_EQ(std::get<NumericInfo>(token.payload).base, test.base);
    EXPECT_EQ(std::get<NumericInfo>(token.payload).suffix, test.suffix);
  }
}

TEST(NumericLiteralsTest, AcceptsEveryIntegerSuffixAsPartOfOneToken) {
  const std::vector<ValidNumericCase> cases = {{"10i8", TokenKind::IntegerLiteral, 10, NumericSuffix::I8}, {"10i16", TokenKind::IntegerLiteral, 10, NumericSuffix::I16}, {"10i32", TokenKind::IntegerLiteral, 10, NumericSuffix::I32}, {"10i64", TokenKind::IntegerLiteral, 10, NumericSuffix::I64}, {"10i128", TokenKind::IntegerLiteral, 10, NumericSuffix::I128}, {"10u8", TokenKind::IntegerLiteral, 10, NumericSuffix::U8}, {"10u16", TokenKind::IntegerLiteral, 10, NumericSuffix::U16}, {"10u32", TokenKind::IntegerLiteral, 10, NumericSuffix::U32}, {"10u64", TokenKind::IntegerLiteral, 10, NumericSuffix::U64}, {"10u128", TokenKind::IntegerLiteral, 10, NumericSuffix::U128}, {"10int", TokenKind::IntegerLiteral, 10, NumericSuffix::Int}, {"10uint", TokenKind::IntegerLiteral, 10, NumericSuffix::UInt}, {"4_096ptrsize", TokenKind::IntegerLiteral, 10, NumericSuffix::PtrSize}, {"10byte", TokenKind::IntegerLiteral, 10, NumericSuffix::Byte}, {"0xFFFFu32", TokenKind::IntegerLiteral, 16, NumericSuffix::U32}, {"0b1010byte", TokenKind::IntegerLiteral, 2, NumericSuffix::Byte}};

  for (const ValidNumericCase& test : cases) {
    SCOPED_TRACE(test.spelling);
    const LexedFile result = tokenize(test.spelling);
    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    const Token& token = result.tokens().front();
    EXPECT_EQ(token.kind, TokenKind::IntegerLiteral);
    EXPECT_EQ(result.raw(token), test.spelling);
    ASSERT_TRUE(std::holds_alternative<NumericInfo>(token.payload));
    EXPECT_EQ(std::get<NumericInfo>(token.payload).base, test.base);
    EXPECT_EQ(std::get<NumericInfo>(token.payload).suffix, test.suffix);
  }
}

TEST(NumericLiteralsTest, AcceptsDecimalFloatFormsGroupingAndEveryFloatSuffix) {
  const std::vector<ValidNumericCase> cases = {{"1.0", TokenKind::FloatLiteral, 10, NumericSuffix::None}, {"0.5", TokenKind::FloatLiteral, 10, NumericSuffix::None}, {"1.25e10", TokenKind::FloatLiteral, 10, NumericSuffix::None}, {"1e10", TokenKind::FloatLiteral, 10, NumericSuffix::None}, {"1.5e-3", TokenKind::FloatLiteral, 10, NumericSuffix::None}, {"1.5E+3", TokenKind::FloatLiteral, 10, NumericSuffix::None}, {"1.234_567", TokenKind::FloatLiteral, 10, NumericSuffix::None}, {"1e10_000", TokenKind::FloatLiteral, 10, NumericSuffix::None}, {"10f16", TokenKind::FloatLiteral, 10, NumericSuffix::F16}, {"10f32", TokenKind::FloatLiteral, 10, NumericSuffix::F32}, {"10f64", TokenKind::FloatLiteral, 10, NumericSuffix::F64}, {"1.5f32", TokenKind::FloatLiteral, 10, NumericSuffix::F32}, {"1e10f64", TokenKind::FloatLiteral, 10, NumericSuffix::F64}};

  for (const ValidNumericCase& test : cases) {
    SCOPED_TRACE(test.spelling);
    const LexedFile result = tokenize(test.spelling);
    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    const Token& token = result.tokens().front();
    EXPECT_EQ(token.kind, TokenKind::FloatLiteral);
    EXPECT_EQ(result.raw(token), test.spelling);
    ASSERT_TRUE(std::holds_alternative<NumericInfo>(token.payload));
    EXPECT_EQ(std::get<NumericInfo>(token.payload).base, 10U);
    EXPECT_EQ(std::get<NumericInfo>(token.payload).suffix, test.suffix);
  }
}

TEST(NumericLiteralsTest, UsesTheLongestHexDigitSequenceBeforeConsideringSuffixes) {
  const LexedFile result = tokenize("0x10f32");
  ASSERT_TRUE(result.succeeded());
  ASSERT_EQ(result.tokens().size(), 2U);
  const Token& token = result.tokens().front();
  EXPECT_EQ(token.kind, TokenKind::IntegerLiteral);
  EXPECT_EQ(result.raw(token), "0x10f32");
  ASSERT_TRUE(std::holds_alternative<NumericInfo>(token.payload));
  EXPECT_EQ(std::get<NumericInfo>(token.payload).base, 16U);
  EXPECT_EQ(std::get<NumericInfo>(token.payload).suffix, NumericSuffix::None);
}

TEST(NumericLiteralsTest, KeepsLeadingSignsAndIncompleteDecimalPointsInSeparateTokens) {
  const LexedFile result = tokenize("-128i8 +10 .5 1. 1.member 1..10");
  ASSERT_TRUE(result.succeeded());
  const std::vector<const Token*> tokens = syntax_tokens(result);
  ASSERT_EQ(tokens.size(), 15U);
  EXPECT_EQ(tokens[0]->kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(tokens[0]->payload), '-');
  EXPECT_EQ(tokens[1]->kind, TokenKind::IntegerLiteral);
  EXPECT_EQ(result.raw(*tokens[1]), "128i8");
  EXPECT_EQ(tokens[2]->kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(tokens[2]->payload), '+');
  EXPECT_EQ(tokens[3]->kind, TokenKind::IntegerLiteral);
  EXPECT_EQ(result.raw(*tokens[3]), "10");
  EXPECT_EQ(tokens[4]->kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(tokens[4]->payload), '.');
  EXPECT_EQ(tokens[5]->kind, TokenKind::IntegerLiteral);
  EXPECT_EQ(result.raw(*tokens[5]), "5");
  EXPECT_EQ(tokens[6]->kind, TokenKind::IntegerLiteral);
  EXPECT_EQ(result.raw(*tokens[6]), "1");
  EXPECT_EQ(tokens[7]->kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(tokens[7]->payload), '.');
  EXPECT_EQ(tokens[8]->kind, TokenKind::IntegerLiteral);
  EXPECT_EQ(result.raw(*tokens[8]), "1");
  EXPECT_EQ(tokens[9]->kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(tokens[9]->payload), '.');
  EXPECT_EQ(tokens[10]->kind, TokenKind::Identifier);
  EXPECT_EQ(result.raw(*tokens[10]), "member");
  EXPECT_EQ(tokens[11]->kind, TokenKind::IntegerLiteral);
  EXPECT_EQ(result.raw(*tokens[11]), "1");
  EXPECT_EQ(tokens[12]->kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(tokens[12]->payload), '.');
  EXPECT_EQ(tokens[13]->kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(tokens[13]->payload), '.');
  EXPECT_EQ(tokens[14]->kind, TokenKind::IntegerLiteral);
  EXPECT_EQ(result.raw(*tokens[14]), "10");
}

TEST(NumericLiteralsTest, PreservesArbitrarilyLongCoefficientsWithoutHostOverflow) {
  const std::string spelling = "1234567890123456789012345678901234567890123456789012345678901234567890";
  const LexedFile result = tokenize(spelling);
  ASSERT_TRUE(result.succeeded());
  ASSERT_EQ(result.tokens().size(), 2U);
  EXPECT_EQ(result.tokens().front().kind, TokenKind::IntegerLiteral);
  EXPECT_EQ(result.raw(result.tokens().front()), spelling);
}

TEST(NumericLiteralsTest, RejectsMalformedNumbersWithSpecificDiagnosticsAndFullCandidateSpans) {
  const std::vector<InvalidNumericCase> cases = {{"0b", DiagnosticKind::MissingBaseDigits}, {"0o", DiagnosticKind::MissingBaseDigits}, {"0x", DiagnosticKind::MissingBaseDigits}, {"0xG", DiagnosticKind::MissingBaseDigits}, {"0b2", DiagnosticKind::DigitOutOfRange}, {"0o8", DiagnosticKind::DigitOutOfRange}, {"100_", DiagnosticKind::MisplacedNumericSeparator}, {"1__000", DiagnosticKind::MisplacedNumericSeparator}, {"0x_FF", DiagnosticKind::MisplacedNumericSeparator}, {"1_.0", DiagnosticKind::MisplacedNumericSeparator}, {"1._0", DiagnosticKind::MisplacedNumericSeparator}, {"1e_10", DiagnosticKind::MisplacedNumericSeparator}, {"1e10_", DiagnosticKind::MisplacedNumericSeparator}, {"10_i32", DiagnosticKind::MisplacedNumericSeparator}, {"1e", DiagnosticKind::MissingExponentDigits}, {"1e+", DiagnosticKind::MissingExponentDigits}, {"1e-", DiagnosticKind::MissingExponentDigits}, {"10foo", DiagnosticKind::UnknownNumericSuffix}, {"1.0meters", DiagnosticKind::UnknownNumericSuffix}, {"0x12u7", DiagnosticKind::UnknownNumericSuffix}, {Utf8(u8"10\u7528\u6237"), DiagnosticKind::UnknownNumericSuffix}, {"1.0i32", DiagnosticKind::InvalidNumericSuffix}, {"1e3u8", DiagnosticKind::InvalidNumericSuffix}, {"0b1.0", DiagnosticKind::UnsupportedNonDecimalFloat}, {"0xA.F", DiagnosticKind::UnsupportedNonDecimalFloat}};

  for (const InvalidNumericCase& test : cases) {
    SCOPED_TRACE(test.spelling);
    const LexedFile result = tokenize(test.spelling);
    ASSERT_FALSE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    const Token& token = result.tokens().front();
    EXPECT_EQ(token.kind, TokenKind::InvalidNumber);
    EXPECT_EQ(token.span, (ByteSpan{0, std::string(test.spelling).size()}));
    EXPECT_EQ(result.raw(token), test.spelling);
    EXPECT_TRUE(has_diagnostic(result, test.diagnostic));
    EXPECT_EQ(result.tokens().back().kind, TokenKind::EndOfFile);
  }
}

TEST(NumericLiteralsTest, RejectsUppercaseBasePrefixesInsteadOfTreatingThemAsValidPrefixes) {
  const std::vector<std::string> spellings = {"0B10", "0O7", "0XFF"};
  for (const std::string& spelling : spellings) {
    SCOPED_TRACE(spelling);
    const LexedFile result = tokenize(spelling);
    ASSERT_FALSE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    EXPECT_EQ(result.tokens().front().kind, TokenKind::InvalidNumber);
    EXPECT_EQ(result.raw(result.tokens().front()), spelling);
  }
}

TEST(NumericLiteralsTest, RejectsInvisibleCharactersInsideNumericSuffixCandidates) {
  const std::string source = Utf8(u8"1a\u200Cb");
  const LexedFile result = tokenize(source);

  ASSERT_FALSE(result.succeeded());
  ASSERT_EQ(result.tokens().size(), 2U);
  EXPECT_EQ(result.tokens().front().kind, TokenKind::InvalidNumber);
  EXPECT_EQ(result.raw(result.tokens().front()), source);
  EXPECT_TRUE(has_diagnostic(result, DiagnosticKind::UnknownNumericSuffix));
  EXPECT_TRUE(has_diagnostic(result, DiagnosticKind::InvisibleCharacter));
}

TEST(NumericLiteralsTest, TriviaSeparatesATypeNameFromTheLiteralSuffixCandidate) {
  const LexedFile result = tokenize("10 i32");
  ASSERT_TRUE(result.succeeded());
  ASSERT_EQ(result.tokens().size(), 4U);
  EXPECT_EQ(result.tokens()[0].kind, TokenKind::IntegerLiteral);
  EXPECT_EQ(result.raw(result.tokens()[0]), "10");
  EXPECT_EQ(result.tokens()[1].kind, TokenKind::SpacesAndTabs);
  EXPECT_EQ(result.tokens()[2].kind, TokenKind::BuiltinType);
  EXPECT_EQ(result.raw(result.tokens()[2]), "i32");
  EXPECT_EQ(result.tokens()[3].kind, TokenKind::EndOfFile);
}

}  // namespace
}  // namespace ink::tokenizer
