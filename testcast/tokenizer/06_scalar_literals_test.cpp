#include "ink/tokenizer/tokenizer.h"

#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <variant>
#include <vector>

namespace ink::tokenizer {
namespace {

struct ValidScalarCase {
  std::string spelling;
  char32_t value;
};

struct InvalidScalarCase {
  std::string spelling;
  DiagnosticKind diagnostic;
};

bool has_diagnostic(const LexedFile& result, DiagnosticKind kind) {
  return std::any_of(result.diagnostics().begin(), result.diagnostics().end(), [kind](const Diagnostic& diagnostic) { return diagnostic.kind == kind; });
}

TEST(ScalarLiteralsTest, AcceptsExactlyOneDirectUnicodeScalar) {
  const std::vector<ValidScalarCase> cases = {{"'A'", U'A'}, {Utf8(u8"'\u00E9'"), U'\u00E9'}, {Utf8(u8"'\u4E2D'"), U'\u4E2D'}, {Utf8(u8"'\U0001F600'"), U'\U0001F600'}};

  for (const ValidScalarCase& test : cases) {
    SCOPED_TRACE(test.spelling);
    const LexedFile result = tokenize(test.spelling);
    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    const Token& token = result.tokens().front();
    EXPECT_EQ(token.kind, TokenKind::ScalarLiteral);
    EXPECT_EQ(token.span, (ByteSpan{0, test.spelling.size()}));
    EXPECT_EQ(result.raw(token), test.spelling);
    ASSERT_TRUE(std::holds_alternative<char32_t>(token.payload));
    EXPECT_EQ(std::get<char32_t>(token.payload), test.value);
  }
}

TEST(ScalarLiteralsTest, DecodesEverySimpleEscape) {
  const std::vector<ValidScalarCase> cases = {{"'\\\\'", U'\\'}, {"'\\''", U'\''}, {"'\\\"'", U'\"'}, {"'\\0'", U'\0'}, {"'\\n'", U'\n'}, {"'\\r'", U'\r'}, {"'\\t'", U'\t'}};

  for (const ValidScalarCase& test : cases) {
    SCOPED_TRACE(test.spelling);
    const LexedFile result = tokenize(test.spelling);
    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    const Token& token = result.tokens().front();
    EXPECT_EQ(token.kind, TokenKind::ScalarLiteral);
    EXPECT_EQ(result.raw(token), test.spelling);
    ASSERT_TRUE(std::holds_alternative<char32_t>(token.payload));
    EXPECT_EQ(std::get<char32_t>(token.payload), test.value);
  }
}

TEST(ScalarLiteralsTest, DecodesFixedWidthHexAndBracedUnicodeEscapes) {
  const std::vector<ValidScalarCase> cases = {{"'\\x00'", U'\0'}, {"'\\x1B'", U'\x1B'}, {"'\\x7F'", U'\x7F'}, {"'\\xFF'", U'\u00FF'}, {"'\\u{0}'", U'\0'}, {"'\\u{41}'", U'A'}, {"'\\u{4E2D}'", U'\u4E2D'}, {"'\\u{1F600}'", U'\U0001F600'}, {"'\\u{200B}'", U'\u200B'}, {"'\\u{202E}'", U'\u202E'}};

  for (const ValidScalarCase& test : cases) {
    SCOPED_TRACE(test.spelling);
    const LexedFile result = tokenize(test.spelling);
    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    const Token& token = result.tokens().front();
    EXPECT_EQ(token.kind, TokenKind::ScalarLiteral);
    EXPECT_EQ(result.raw(token), test.spelling);
    ASSERT_TRUE(std::holds_alternative<char32_t>(token.payload));
    EXPECT_EQ(std::get<char32_t>(token.payload), test.value);
  }
}

TEST(ScalarLiteralsTest, RejectsEmptyMultipleAndMultiScalarGraphemeSpellings) {
  const std::vector<InvalidScalarCase> cases = {{"''", DiagnosticKind::EmptyScalarLiteral}, {"'ab'", DiagnosticKind::MultipleScalarValues}, {Utf8(u8"'e\u0301'"), DiagnosticKind::MultipleScalarValues}, {Utf8(u8"'\U0001F1E8\U0001F1F3'"), DiagnosticKind::MultipleScalarValues}};

  for (const InvalidScalarCase& test : cases) {
    SCOPED_TRACE(test.spelling);
    const LexedFile result = tokenize(test.spelling);
    ASSERT_FALSE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    EXPECT_EQ(result.tokens().front().kind, TokenKind::InvalidScalarLiteral);
    EXPECT_EQ(result.raw(result.tokens().front()), test.spelling);
    EXPECT_TRUE(has_diagnostic(result, test.diagnostic));
  }
}

TEST(ScalarLiteralsTest, RejectsUnknownAndMalformedEscapes) {
  const std::vector<InvalidScalarCase> cases = {{"'\\q'", DiagnosticKind::UnknownEscape}, {"'\\x1'", DiagnosticKind::InvalidHexEscape}, {"'\\x_G'", DiagnosticKind::InvalidHexEscape}, {"'\\u0041'", DiagnosticKind::InvalidUnicodeEscape}, {"'\\u{}'", DiagnosticKind::InvalidUnicodeEscape}, {"'\\u{1_F600}'", DiagnosticKind::InvalidUnicodeEscape}, {"'\\u{ 41 }'", DiagnosticKind::InvalidUnicodeEscape}, {"'\\u{0x41}'", DiagnosticKind::InvalidUnicodeEscape}, {"'\\u{1234567}'", DiagnosticKind::InvalidUnicodeEscape}};

  for (const InvalidScalarCase& test : cases) {
    SCOPED_TRACE(test.spelling);
    const LexedFile result = tokenize(test.spelling);
    ASSERT_FALSE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    EXPECT_EQ(result.tokens().front().kind, TokenKind::InvalidScalarLiteral);
    EXPECT_EQ(result.raw(result.tokens().front()), test.spelling);
    EXPECT_TRUE(has_diagnostic(result, test.diagnostic));
  }

  const LexedFile extra_scalar = tokenize("'\\x123'");
  ASSERT_FALSE(extra_scalar.succeeded());
  ASSERT_EQ(extra_scalar.tokens().size(), 2U);
  EXPECT_EQ(extra_scalar.tokens().front().kind, TokenKind::InvalidScalarLiteral);
  EXPECT_EQ(extra_scalar.raw(extra_scalar.tokens().front()), "'\\x123'");
  EXPECT_TRUE(has_diagnostic(extra_scalar, DiagnosticKind::MultipleScalarValues) || has_diagnostic(extra_scalar, DiagnosticKind::InvalidHexEscape));
}

TEST(ScalarLiteralsTest, RejectsSurrogatesAndValuesBeyondUnicodeRange) {
  const std::vector<std::string> spellings = {"'\\u{D800}'", "'\\u{DFFF}'", "'\\u{110000}'"};
  for (const std::string& spelling : spellings) {
    SCOPED_TRACE(spelling);
    const LexedFile result = tokenize(spelling);
    ASSERT_FALSE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    EXPECT_EQ(result.tokens().front().kind, TokenKind::InvalidScalarLiteral);
    EXPECT_EQ(result.raw(result.tokens().front()), spelling);
    EXPECT_TRUE(has_diagnostic(result, DiagnosticKind::InvalidUnicodeScalar));
  }
}

TEST(ScalarLiteralsTest, RequiresInvisibleAndControlCharactersToBeEscaped) {
  const std::vector<std::string> invisible_spellings = {Utf8(u8"'\u200B'"), Utf8(u8"'\u202E'"), Utf8(u8"'\uFE0F'")};
  for (const std::string& spelling : invisible_spellings) {
    SCOPED_TRACE(spelling);
    const LexedFile result = tokenize(spelling);
    ASSERT_FALSE(result.succeeded());
    EXPECT_TRUE(has_diagnostic(result, DiagnosticKind::InvisibleCharacter));
    ASSERT_FALSE(result.tokens().empty());
    EXPECT_EQ(result.tokens().front().kind, TokenKind::InvalidScalarLiteral);
  }

  std::string control_spelling = "'";
  control_spelling.push_back('\x01');
  control_spelling.push_back('\'');
  const LexedFile control_result = tokenize(control_spelling);
  ASSERT_FALSE(control_result.succeeded());
  EXPECT_TRUE(has_diagnostic(control_result, DiagnosticKind::ForbiddenControlCharacter));
  ASSERT_FALSE(control_result.tokens().empty());
  EXPECT_EQ(control_result.tokens().front().kind, TokenKind::InvalidScalarLiteral);

  std::string carriage_return_spelling = "'";
  carriage_return_spelling.push_back('\r');
  carriage_return_spelling.push_back('\'');
  const LexedFile carriage_return_result = tokenize(carriage_return_spelling);
  ASSERT_FALSE(carriage_return_result.succeeded());
  EXPECT_TRUE(has_diagnostic(carriage_return_result, DiagnosticKind::LoneCarriageReturn));
  ASSERT_FALSE(carriage_return_result.tokens().empty());
  EXPECT_EQ(carriage_return_result.tokens().front().kind, TokenKind::InvalidScalarLiteral);
}

TEST(ScalarLiteralsTest, StopsAnUnterminatedLiteralBeforeThePhysicalLineBreakAndContinuesScanning) {
  const LexedFile lf_result = tokenize("'a\nnext");
  ASSERT_FALSE(lf_result.succeeded());
  ASSERT_EQ(lf_result.tokens().size(), 4U);
  EXPECT_EQ(lf_result.tokens()[0].kind, TokenKind::InvalidScalarLiteral);
  EXPECT_EQ(lf_result.raw(lf_result.tokens()[0]), "'a");
  EXPECT_TRUE(has_diagnostic(lf_result, DiagnosticKind::UnterminatedScalarLiteral));
  EXPECT_EQ(lf_result.tokens()[1].kind, TokenKind::LineBreak);
  EXPECT_EQ(lf_result.raw(lf_result.tokens()[1]), "\n");
  EXPECT_EQ(lf_result.tokens()[2].kind, TokenKind::Identifier);
  EXPECT_EQ(lf_result.raw(lf_result.tokens()[2]), "next");

  const LexedFile crlf_result = tokenize("'a\r\nnext");
  ASSERT_FALSE(crlf_result.succeeded());
  ASSERT_EQ(crlf_result.tokens().size(), 4U);
  EXPECT_EQ(crlf_result.tokens()[0].kind, TokenKind::InvalidScalarLiteral);
  EXPECT_EQ(crlf_result.raw(crlf_result.tokens()[0]), "'a");
  EXPECT_EQ(crlf_result.tokens()[1].kind, TokenKind::LineBreak);
  EXPECT_EQ(crlf_result.raw(crlf_result.tokens()[1]), "\r\n");
  EXPECT_EQ(crlf_result.tokens()[2].kind, TokenKind::Identifier);
}

TEST(ScalarLiteralsTest, RejectsEndOfFileBeforeTheClosingQuote) {
  const LexedFile result = tokenize("'a");
  ASSERT_FALSE(result.succeeded());
  ASSERT_EQ(result.tokens().size(), 2U);
  EXPECT_EQ(result.tokens().front().kind, TokenKind::InvalidScalarLiteral);
  EXPECT_EQ(result.raw(result.tokens().front()), "'a");
  EXPECT_TRUE(has_diagnostic(result, DiagnosticKind::UnterminatedScalarLiteral));
}

TEST(ScalarLiteralsTest, KeepsAdjacentLiteralsAndFollowingIdentifiersAsSeparateTokens) {
  const LexedFile adjacent = tokenize("'a''b'");
  ASSERT_TRUE(adjacent.succeeded());
  ASSERT_EQ(adjacent.tokens().size(), 3U);
  EXPECT_EQ(adjacent.tokens()[0].kind, TokenKind::ScalarLiteral);
  EXPECT_EQ(adjacent.raw(adjacent.tokens()[0]), "'a'");
  EXPECT_EQ(adjacent.tokens()[1].kind, TokenKind::ScalarLiteral);
  EXPECT_EQ(adjacent.raw(adjacent.tokens()[1]), "'b'");
  EXPECT_EQ(adjacent.tokens()[2].kind, TokenKind::EndOfFile);

  const LexedFile suffixed = tokenize("'a'name");
  ASSERT_TRUE(suffixed.succeeded());
  ASSERT_EQ(suffixed.tokens().size(), 3U);
  EXPECT_EQ(suffixed.tokens()[0].kind, TokenKind::ScalarLiteral);
  EXPECT_EQ(suffixed.tokens()[1].kind, TokenKind::Identifier);
  EXPECT_EQ(suffixed.raw(suffixed.tokens()[1]), "name");
}

}  // namespace
}  // namespace ink::tokenizer
