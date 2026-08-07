#include "ink/tokenizer/tokenizer.h"
#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <vector>

namespace ink::tokenizer {
namespace {

std::string Bytes(std::initializer_list<unsigned int> values) {
  std::string result;
  result.reserve(values.size());
  for (const unsigned int value : values) {
    result.push_back(static_cast<char>(value));
  }
  return result;
}

bool HasTokenKind(const LexedFile& lexed, TokenKind kind) {
  for (const Token& token : lexed.tokens()) {
    if (token.kind == kind) {
      return true;
    }
  }
  return false;
}

bool HasDiagnosticKind(const LexedFile& lexed, DiagnosticKind kind) {
  for (const Diagnostic& diagnostic : lexed.diagnostics()) {
    if (diagnostic.kind == kind) {
      return true;
    }
  }
  return false;
}

void ExpectFullFidelity(const LexedFile& lexed) {
  const std::vector<Token>& tokens = lexed.tokens();
  ASSERT_FALSE(tokens.empty());
  EXPECT_EQ(tokens.back().kind, TokenKind::EndOfFile);
  EXPECT_EQ(tokens.back().span.start, lexed.source().size());
  EXPECT_EQ(tokens.back().span.end, lexed.source().size());
  EXPECT_TRUE(lexed.raw(tokens.back()).empty());

  std::size_t next_byte = 0;
  std::size_t eof_count = 0;
  std::string reconstructed;
  for (const Token& token : tokens) {
    EXPECT_LE(token.span.start, token.span.end);
    EXPECT_LE(token.span.end, lexed.source().size());
    EXPECT_EQ(token.is_trivia(), is_trivia(token.kind));
    EXPECT_EQ(token.is_error(), is_error(token.kind));
    if (token.kind == TokenKind::EndOfFile) {
      ++eof_count;
      EXPECT_EQ(&token, &tokens.back());
      continue;
    }
    EXPECT_EQ(token.span.start, next_byte);
    EXPECT_EQ(lexed.raw(token).size(), token.span.size());
    reconstructed.append(lexed.raw(token));
    next_byte = token.span.end;
  }
  EXPECT_EQ(eof_count, 1U);
  EXPECT_EQ(next_byte, lexed.source().size());
  EXPECT_EQ(reconstructed, lexed.source());
}

TEST(SourceEncodingTest, EmptyFileHasExactlyOneEofToken) {
  const LexedFile lexed = tokenize("");

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_TRUE(lexed.diagnostics().empty());
  ASSERT_EQ(lexed.tokens().size(), 1U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::EndOfFile);
  ExpectFullFidelity(lexed);
}

TEST(SourceEncodingTest, InitialBomAndMultibyteCharactersUseOriginalByteSpans) {
  const std::string source = Utf8(u8"\uFEFF\u7528\u6237\r\n\tname");
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 6U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::Utf8Bom);
  EXPECT_EQ(lexed.tokens()[0].span.start, 0U);
  EXPECT_EQ(lexed.tokens()[0].span.end, 3U);
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.tokens()[1].span.start, 3U);
  EXPECT_EQ(lexed.tokens()[1].span.end, 9U);
  EXPECT_EQ(lexed.tokens()[2].kind, TokenKind::LineBreak);
  EXPECT_EQ(lexed.tokens()[2].span.start, 9U);
  EXPECT_EQ(lexed.tokens()[2].span.end, 11U);
  EXPECT_EQ(lexed.raw(lexed.tokens()[2]), "\r\n");
  EXPECT_EQ(lexed.tokens()[3].kind, TokenKind::SpacesAndTabs);
  EXPECT_EQ(lexed.tokens()[3].span.start, 11U);
  EXPECT_EQ(lexed.tokens()[3].span.end, 12U);
  EXPECT_EQ(lexed.tokens()[4].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.tokens()[4].span.start, 12U);
  EXPECT_EQ(lexed.tokens()[4].span.end, 16U);
  ExpectFullFidelity(lexed);
}

TEST(SourceEncodingTest, StrictUtf8AcceptsValidScalarsIncludingMaximumScalar) {
  std::string source = "//";
  source.append(Utf8(u8"\u00E9\u4E2D\U0001F600"));
  source.append(Bytes({0xF4, 0x8F, 0xBF, 0xBF}));
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 2U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::LineComment);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), source);
  ExpectFullFidelity(lexed);
}

TEST(SourceEncodingTest, StrictUtf8RejectsEveryInvalidSequenceClass) {
  struct InvalidUtf8Case {
    const char* name;
    std::string bytes;
  };
  const std::vector<InvalidUtf8Case> cases = {{"invalid leading byte", Bytes({0x80})}, {"invalid continuation byte", Bytes({0xC2, 0x41})}, {"truncated sequence", Bytes({0xE2, 0x82})}, {"two byte overlong encoding", Bytes({0xC0, 0x80})}, {"three byte overlong encoding", Bytes({0xE0, 0x80, 0x80})}, {"surrogate", Bytes({0xED, 0xA0, 0x80})}, {"above unicode maximum", Bytes({0xF4, 0x90, 0x80, 0x80})}, {"invalid four byte leader", Bytes({0xF5, 0x80, 0x80, 0x80})}, {"utf16 little endian bom", Bytes({0xFF, 0xFE})}};

  for (const InvalidUtf8Case& test_case : cases) {
    SCOPED_TRACE(test_case.name);
    const LexedFile lexed = tokenize(test_case.bytes);
    EXPECT_FALSE(lexed.succeeded());
    EXPECT_TRUE(HasTokenKind(lexed, TokenKind::InvalidEncoding));
    EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::InvalidUtf8));
    ExpectFullFidelity(lexed);
  }
}

TEST(SourceEncodingTest, InvalidUtf8RecoveryDoesNotConsumeFollowingToken) {
  const std::vector<std::string> invalid_prefixes = {Bytes({0xE2}), Bytes({0xE2, 0x82}), Bytes({0xF0, 0x80})};
  for (const std::string& invalid_prefix : invalid_prefixes) {
    std::string source = invalid_prefix;
    source.push_back('x');
    const LexedFile lexed = tokenize(source);

    ASSERT_FALSE(lexed.succeeded());
    ASSERT_EQ(lexed.tokens().size(), 3U);
    EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::InvalidEncoding);
    EXPECT_EQ(lexed.raw(lexed.tokens()[0]), invalid_prefix);
    EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::Identifier);
    EXPECT_EQ(lexed.raw(lexed.tokens()[1]), "x");
    ExpectFullFidelity(lexed);
  }
}

TEST(SourceEncodingTest, StrictUtf8ValidationAlsoAppliesInsideCommentsAndLiterals) {
  const std::string invalid_byte = Bytes({0x80});
  const std::vector<std::string> cases = {std::string("//") + invalid_byte, std::string("/*") + invalid_byte, std::string("\"") + invalid_byte + "\"", std::string("r\"") + invalid_byte + "\"", std::string("'\\u{") + invalid_byte + "}'", std::string("\"\"\"\n") + invalid_byte, std::string("\"\"\"inline\n") + invalid_byte};

  for (const std::string& source : cases) {
    SCOPED_TRACE(source.size());
    const LexedFile lexed = tokenize(source);
    EXPECT_FALSE(lexed.succeeded());
    EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::InvalidUtf8));
    ExpectFullFidelity(lexed);
  }
}

TEST(SourceEncodingTest, Utf16AndUtf32ByteOrderMarksAreRejected) {
  const std::vector<std::string> cases = {Bytes({0xFF, 0xFE}), Bytes({0xFE, 0xFF}), Bytes({0xFF, 0xFE, 0x00, 0x00}), Bytes({0x00, 0x00, 0xFE, 0xFF})};

  for (const std::string& source : cases) {
    SCOPED_TRACE(source.size());
    const LexedFile lexed = tokenize(source);
    EXPECT_FALSE(lexed.succeeded());
    EXPECT_TRUE(HasTokenKind(lexed, TokenKind::InvalidEncoding) || HasTokenKind(lexed, TokenKind::InvalidCharacter));
    ExpectFullFidelity(lexed);
  }
}

TEST(SourceEncodingTest, BomOutsideTheInitialPositionIsAnErrorAndNotWhitespace) {
  const std::string source = Utf8(u8"a\uFEFFb");
  const LexedFile lexed = tokenize(source);

  ASSERT_FALSE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 4U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::InvalidCharacter);
  EXPECT_EQ(lexed.raw(lexed.tokens()[1]), Utf8(u8"\uFEFF"));
  EXPECT_EQ(lexed.tokens()[2].kind, TokenKind::Identifier);
  EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::UnexpectedBom));
  ExpectFullFidelity(lexed);
}

TEST(SourceEncodingTest, LfAndCrLfAreSingleLogicalLineBreakTokensWithDistinctRawBytes) {
  const std::string source = "a \t\tb\nc\r\nd";
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 8U);
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::SpacesAndTabs);
  EXPECT_EQ(lexed.raw(lexed.tokens()[1]), " \t\t");
  EXPECT_EQ(lexed.tokens()[3].kind, TokenKind::LineBreak);
  EXPECT_EQ(lexed.raw(lexed.tokens()[3]), "\n");
  EXPECT_EQ(lexed.tokens()[5].kind, TokenKind::LineBreak);
  EXPECT_EQ(lexed.raw(lexed.tokens()[5]), "\r\n");
  ExpectFullFidelity(lexed);
}

TEST(SourceEncodingTest, LogicalLineStartsRetainOriginalByteOffsets) {
  const std::string source = Utf8(u8"用户\r\nname\nlast");
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  EXPECT_EQ(lexed.line_starts(), (std::vector<std::size_t>{0, 8, 13}));
  EXPECT_EQ(lexed.line_number(0), 1U);
  EXPECT_EQ(lexed.line_number(6), 1U);
  EXPECT_EQ(lexed.line_number(8), 2U);
  EXPECT_EQ(lexed.line_number(13), 3U);
  EXPECT_EQ(lexed.line_number(source.size()), 3U);
  ExpectFullFidelity(lexed);
}

TEST(SourceEncodingTest, LoneCarriageReturnIsAnErrorAndRecoveryContinues) {
  const LexedFile lexed = tokenize("a\rb");

  ASSERT_FALSE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 4U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::InvalidCharacter);
  EXPECT_EQ(lexed.raw(lexed.tokens()[1]), "\r");
  EXPECT_EQ(lexed.tokens()[2].kind, TokenKind::Identifier);
  EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::LoneCarriageReturn));
  ExpectFullFidelity(lexed);
}

TEST(SourceEncodingTest, UnicodeWhitespaceAndUnicodeLineSeparatorsAreNotCodeWhitespaceOrLineBreaks) {
  const std::vector<std::string> cases = {Utf8(u8"a\u0085b"), Utf8(u8"a\u00A0b"), Utf8(u8"a\u2003b"), Utf8(u8"a\u2028b"), Utf8(u8"a\u2029b"), Utf8(u8"a\u3000b")};

  for (const std::string& source : cases) {
    SCOPED_TRACE(source);
    const LexedFile lexed = tokenize(source);
    EXPECT_FALSE(lexed.succeeded());
    EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::NonAsciiWhitespace));
    EXPECT_FALSE(HasTokenKind(lexed, TokenKind::LineBreak));
    ExpectFullFidelity(lexed);
  }
}

TEST(SourceEncodingTest, RawControlsAreForbiddenEvenInsideCommentsAndLiterals) {
  std::string nul_in_comment = "//before";
  nul_in_comment.push_back('\0');
  nul_in_comment.append("after");
  std::string escape_in_string = "\"before";
  escape_in_string.push_back(static_cast<char>(0x1B));
  escape_in_string.append("after\"");
  std::string del_in_block_comment = "/*before";
  del_in_block_comment.push_back(static_cast<char>(0x7F));
  del_in_block_comment.append("after*/");
  std::string nul_in_unterminated_block = "/*before";
  nul_in_unterminated_block.push_back('\0');
  std::string nul_in_unterminated_multiline = "\"\"\"\n  before";
  nul_in_unterminated_multiline.push_back('\0');
  std::string nul_after_escape = "\"\\";
  nul_after_escape.push_back('\0');
  nul_after_escape.push_back('"');
  std::string nul_in_unicode_escape = "\"\\u{";
  nul_in_unicode_escape.push_back('\0');
  nul_in_unicode_escape.append("}\"");
  const std::vector<std::string> cases = {nul_in_comment, escape_in_string, del_in_block_comment, nul_in_unterminated_block, nul_in_unterminated_multiline, nul_after_escape, nul_in_unicode_escape};

  for (const std::string& source : cases) {
    SCOPED_TRACE(source.size());
    const LexedFile lexed = tokenize(source);
    EXPECT_FALSE(lexed.succeeded());
    EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::ForbiddenControlCharacter));
    ExpectFullFidelity(lexed);
  }
}

TEST(SourceEncodingTest, TokenizerDoesNotNormalizeTheWholeSource) {
  const std::string source = Utf8(u8"//\u00E9 e\u0301");
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 2U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::LineComment);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), source);
  EXPECT_EQ(lexed.source(), source);
  ExpectFullFidelity(lexed);
}

TEST(SourceEncodingTest, AUnicodeScalarThatCannotStartAnyTokenProducesAnErrorToken) {
  const std::string source = Utf8(u8"\U0001F600value");
  const LexedFile lexed = tokenize(source);

  ASSERT_FALSE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 3U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::InvalidCharacter);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), Utf8(u8"\U0001F600"));
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.raw(lexed.tokens()[1]), "value");
  EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::InvalidCharacter));
  ExpectFullFidelity(lexed);
}

}  // namespace
}  // namespace ink::tokenizer
