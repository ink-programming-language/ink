#include "ink/tokenizer/tokenizer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace ink::tokenizer {
namespace {

void ExpectPartition(const LexedFile& file) {
  ASSERT_FALSE(file.tokens().empty());
  std::size_t cursor = 0;
  std::size_t eof_count = 0;
  std::string rebuilt;
  for (const Token& token : file.tokens()) {
    if (token.kind == TokenKind::EndOfFile) {
      ++eof_count;
      EXPECT_EQ(token.span, (ByteSpan{file.source().size(), file.source().size()}));
      EXPECT_TRUE(file.raw(token).empty());
      continue;
    }
    EXPECT_EQ(token.span.start, cursor);
    EXPECT_EQ(token.span.size(), file.raw(token).size());
    rebuilt.append(file.raw(token).data(), file.raw(token).size());
    cursor = token.span.end;
  }
  EXPECT_EQ(cursor, file.source().size());
  EXPECT_EQ(rebuilt, file.source());
  EXPECT_EQ(eof_count, 1u);
  EXPECT_EQ(file.tokens().back().kind, TokenKind::EndOfFile);
}

void ExpectToken(const LexedFile& file, std::size_t index, TokenKind kind, std::string_view raw) {
  ASSERT_LT(index, file.tokens().size());
  EXPECT_EQ(file.tokens()[index].kind, kind);
  EXPECT_EQ(std::string(file.raw(file.tokens()[index])), std::string(raw));
}

bool HasDiagnostic(const LexedFile& file, DiagnosticKind kind) {
  return std::any_of(file.diagnostics().begin(), file.diagnostics().end(), [kind](const Diagnostic& diagnostic) { return diagnostic.kind == kind; });
}

const StringInfo& StringPayload(const Token& token) {
  return std::get<StringInfo>(token.payload);
}

TEST(StringLiteralTest, LexesEmptyAsciiAndUtf8Strings) {
  const std::string chinese = "\xE4\xBD\xA0\xE5\xA5\xBD";
  const std::string emoji = "\xF0\x9F\x98\x80";
  const std::string source = "\"\" \"hello\" \"" + chinese + "\" \"" + emoji + "\"";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 8u);
  ExpectToken(file, 0, TokenKind::StringLiteral, "\"\"");
  ExpectToken(file, 2, TokenKind::StringLiteral, "\"hello\"");
  ExpectToken(file, 4, TokenKind::StringLiteral, std::string("\"") + chinese + "\"");
  ExpectToken(file, 6, TokenKind::StringLiteral, std::string("\"") + emoji + "\"");
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, "");
  EXPECT_EQ(StringPayload(file.tokens()[2]).decoded, "hello");
  EXPECT_EQ(StringPayload(file.tokens()[4]).decoded, chinese);
  EXPECT_EQ(StringPayload(file.tokens()[6]).decoded, emoji);
  EXPECT_EQ(StringPayload(file.tokens()[6]).mode, StringMode::EscapedSingleLine);
  ExpectPartition(file);
}

TEST(StringLiteralTest, DecodesEverySimpleEscape) {
  const std::string source = R"ink("\\\'\"\0\n\r\t")ink";
  std::string expected;
  expected.push_back('\\');
  expected.push_back('\'');
  expected.push_back('"');
  expected.push_back('\0');
  expected.push_back('\n');
  expected.push_back('\r');
  expected.push_back('\t');

  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 2u);
  ExpectToken(file, 0, TokenKind::StringLiteral, source);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, expected);
  ExpectPartition(file);
}

TEST(StringLiteralTest, DecodesFixedWidthHexAndUnicodeEscapes) {
  const std::string source = R"ink("\x00\xFF" "\u{0}\u{4E2D}\u{1F600}")ink";
  const std::string expected_hex("\0\xC3\xBF", 3);
  const std::string expected_unicode = std::string("\0", 1) + "\xE4\xB8\xAD\xF0\x9F\x98\x80";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 4u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, expected_hex);
  EXPECT_EQ(StringPayload(file.tokens()[2]).decoded, expected_unicode);
  ExpectPartition(file);
}

TEST(StringLiteralTest, HexEscapeConsumesExactlyTwoDigits) {
  const std::string source = R"ink("\x123")ink";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 2u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, std::string("\x12") + "3");
  ExpectPartition(file);
}

TEST(StringLiteralTest, PreservesDistinctUnicodeScalarSequencesWithoutNormalization) {
  const std::string precomposed = "\xC3\xA9";
  const std::string decomposed = "e\xCC\x81";
  const std::string source = std::string("\"") + precomposed + "\" \"e\\u{301}\"";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 4u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, precomposed);
  EXPECT_EQ(StringPayload(file.tokens()[2]).decoded, decomposed);
  EXPECT_NE(StringPayload(file.tokens()[0]).decoded, StringPayload(file.tokens()[2]).decoded);
  ExpectPartition(file);
}

TEST(StringLiteralTest, TreatsInterpolationAndCommentDelimitersAsText) {
  const std::string source = R"ink("${value} // /* not comments */")ink";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 2u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, "${value} // /* not comments */");
  ExpectPartition(file);
}

TEST(StringLiteralTest, KeepsAdjacentStringsAndSuffixIdentifierSeparate) {
  const std::string source = R"ink("first""second"name)ink";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 4u);
  ExpectToken(file, 0, TokenKind::StringLiteral, "\"first\"");
  ExpectToken(file, 1, TokenKind::StringLiteral, "\"second\"");
  ExpectToken(file, 2, TokenKind::Identifier, "name");
  ExpectPartition(file);
}

TEST(StringLiteralTest, RejectsUnknownSimpleEscape) {
  const std::string source = R"ink("\q")ink";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  ExpectToken(file, 0, TokenKind::InvalidStringLiteral, source);
  EXPECT_TRUE(file.tokens()[0].is_error());
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::UnknownEscape));
  ExpectPartition(file);
}

TEST(StringLiteralTest, RejectsMalformedHexEscapes) {
  const std::vector<std::string> sources = {R"ink("\x")ink", R"ink("\x1")ink", R"ink("\xG0")ink"};
  for (const std::string& source : sources) {
    SCOPED_TRACE(source);
    const LexedFile file = tokenize(source);
    EXPECT_FALSE(file.succeeded());
    ASSERT_GE(file.tokens().size(), 2u);
    EXPECT_EQ(file.tokens()[0].kind, TokenKind::InvalidStringLiteral);
    EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::InvalidHexEscape));
    ExpectPartition(file);
  }
}

TEST(StringLiteralTest, RejectsMalformedUnicodeEscapes) {
  const std::vector<std::string> sources = {R"ink("\u0041")ink", R"ink("\u{}")ink", R"ink("\u{1234567}")ink", R"ink("\u{1_2}")ink", R"ink("\u{ 41}")ink", R"ink("\u{+41}")ink", R"ink("\u{0x41}")ink"};
  for (const std::string& source : sources) {
    SCOPED_TRACE(source);
    const LexedFile file = tokenize(source);
    EXPECT_FALSE(file.succeeded());
    ASSERT_GE(file.tokens().size(), 2u);
    EXPECT_EQ(file.tokens()[0].kind, TokenKind::InvalidStringLiteral);
    EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::InvalidUnicodeEscape));
    ExpectPartition(file);
  }
}

TEST(StringLiteralTest, RejectsNonScalarUnicodeEscapes) {
  const std::vector<std::string> sources = {R"ink("\u{D800}")ink", R"ink("\u{DFFF}")ink", R"ink("\u{110000}")ink"};
  for (const std::string& source : sources) {
    SCOPED_TRACE(source);
    const LexedFile file = tokenize(source);
    EXPECT_FALSE(file.succeeded());
    ASSERT_GE(file.tokens().size(), 2u);
    EXPECT_EQ(file.tokens()[0].kind, TokenKind::InvalidStringLiteral);
    EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::InvalidUnicodeScalar));
    ExpectPartition(file);
  }
}

TEST(StringLiteralTest, StopsInvalidSingleLineStringBeforeLfAndRecovers) {
  const std::string source = "\"oops\nnext";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 4u);
  ExpectToken(file, 0, TokenKind::InvalidStringLiteral, "\"oops");
  ExpectToken(file, 1, TokenKind::LineBreak, "\n");
  ExpectToken(file, 2, TokenKind::Identifier, "next");
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::UnterminatedStringLiteral));
  ExpectPartition(file);
}

TEST(StringLiteralTest, StopsInvalidSingleLineStringBeforeCrlfAndRecovers) {
  const std::string source = "\"oops\r\nnext";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 4u);
  ExpectToken(file, 0, TokenKind::InvalidStringLiteral, "\"oops");
  ExpectToken(file, 1, TokenKind::LineBreak, "\r\n");
  ExpectToken(file, 2, TokenKind::Identifier, "next");
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::UnterminatedStringLiteral));
  ExpectPartition(file);
}

TEST(StringLiteralTest, BackslashDoesNotContinueASingleLineStringAcrossLf) {
  const std::string source = "\"oops\\\nnext";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 4u);
  ExpectToken(file, 0, TokenKind::InvalidStringLiteral, "\"oops\\");
  ExpectToken(file, 1, TokenKind::LineBreak, "\n");
  ExpectToken(file, 2, TokenKind::Identifier, "next");
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::UnknownEscape));
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::UnterminatedStringLiteral));
  ExpectPartition(file);

  const std::string crlf_source = "\"oops\\\r\nnext";
  const LexedFile crlf_file = tokenize(crlf_source);
  ASSERT_FALSE(crlf_file.succeeded());
  ASSERT_EQ(crlf_file.tokens().size(), 4u);
  ExpectToken(crlf_file, 0, TokenKind::InvalidStringLiteral, "\"oops\\");
  ExpectToken(crlf_file, 1, TokenKind::LineBreak, "\r\n");
  ExpectToken(crlf_file, 2, TokenKind::Identifier, "next");
  EXPECT_TRUE(HasDiagnostic(crlf_file, DiagnosticKind::UnknownEscape));
  EXPECT_FALSE(HasDiagnostic(crlf_file, DiagnosticKind::LoneCarriageReturn));
  ExpectPartition(crlf_file);
}

TEST(StringLiteralTest, RejectsEofBeforeClosingQuote) {
  const std::string source = "\"unterminated";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  ExpectToken(file, 0, TokenKind::InvalidStringLiteral, source);
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::UnterminatedStringLiteral));
  ExpectPartition(file);
}

TEST(StringLiteralTest, RejectsDirectInvisibleFormatCharacters) {
  const std::vector<std::string> invisible_characters = {"\xE2\x80\x8B", "\xE2\x80\xAE", "\xEF\xB8\x8F"};
  for (const std::string& invisible : invisible_characters) {
    const std::string source = std::string("\"a") + invisible + "b\"";
    SCOPED_TRACE(source);
    const LexedFile file = tokenize(source);
    EXPECT_FALSE(file.succeeded());
    EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::InvisibleCharacter));
    ExpectPartition(file);
  }
}

TEST(StringLiteralTest, EscapeRecoveryStillDiagnosesRawInvisibleCharacters) {
  const std::string invisible = "\xE2\x80\x8C";
  const std::vector<std::string> sources = {std::string("\"\\") + invisible + "\"", std::string("\"\\u{") + invisible + "}\""};
  for (const std::string& source : sources) {
    SCOPED_TRACE(source);
    const LexedFile file = tokenize(source);
    EXPECT_FALSE(file.succeeded());
    EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::InvisibleCharacter));
    ExpectPartition(file);
  }
}

TEST(StringLiteralTest, AllowsInvisibleValuesThroughUnicodeEscapes) {
  const std::string source = R"ink("\u{200B}\u{202E}\u{FE0F}")ink";
  const std::string expected = "\xE2\x80\x8B\xE2\x80\xAE\xEF\xB8\x8F";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 2u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, expected);
  ExpectPartition(file);
}

TEST(StringLiteralTest, RejectsRawControlCharacterInsideSourceText) {
  std::string source = "\"before";
  source.push_back('\0');
  source += "after\"";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::ForbiddenControlCharacter));
  ExpectPartition(file);
}

}  // namespace
}  // namespace ink::tokenizer
