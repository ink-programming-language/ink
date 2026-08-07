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

TEST(RawAndMultilineStringTest, DistinguishesAllFourStringModes) {
  const std::string source = R"ink("line\n" r"line\n" """
  line\nnext
  """ r"""
  line\nnext
  """)ink";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 8u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).mode, StringMode::EscapedSingleLine);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, "line\n");
  EXPECT_EQ(StringPayload(file.tokens()[2]).mode, StringMode::RawSingleLine);
  EXPECT_EQ(StringPayload(file.tokens()[2]).decoded, "line\\n");
  EXPECT_EQ(StringPayload(file.tokens()[4]).mode, StringMode::EscapedMultiline);
  EXPECT_EQ(StringPayload(file.tokens()[4]).decoded, "line\nnext");
  EXPECT_EQ(StringPayload(file.tokens()[6]).mode, StringMode::RawMultiline);
  EXPECT_EQ(StringPayload(file.tokens()[6]).decoded, "line\\nnext");
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, RawPrefixMustBeLowercaseAndAdjacent) {
  const std::string source = R"ink(r"raw" r "ordinary" raw"ordinary" R"ordinary")ink";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 12u);
  ExpectToken(file, 0, TokenKind::StringLiteral, "r\"raw\"");
  EXPECT_EQ(StringPayload(file.tokens()[0]).mode, StringMode::RawSingleLine);
  ExpectToken(file, 2, TokenKind::Identifier, "r");
  ExpectToken(file, 4, TokenKind::StringLiteral, "\"ordinary\"");
  ExpectToken(file, 6, TokenKind::Identifier, "raw");
  ExpectToken(file, 7, TokenKind::StringLiteral, "\"ordinary\"");
  ExpectToken(file, 9, TokenKind::Identifier, "R");
  ExpectToken(file, 10, TokenKind::StringLiteral, "\"ordinary\"");
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, RawSingleLineDoesNotInterpretBackslashes) {
  const std::string source = R"ink(r"C:\Users\Hello\file.txt" r"\d+\.\d+" r"\n" r"${value}")ink";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 8u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, R"ink(C:\Users\Hello\file.txt)ink");
  EXPECT_EQ(StringPayload(file.tokens()[2]).decoded, R"ink(\d+\.\d+)ink");
  EXPECT_EQ(StringPayload(file.tokens()[4]).decoded, R"ink(\n)ink");
  EXPECT_EQ(StringPayload(file.tokens()[6]).decoded, "${value}");
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, BackslashCannotEscapeRawClosingQuote) {
  const std::string source = R"ink(r"a\")ink";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 2u);
  ExpectToken(file, 0, TokenKind::StringLiteral, source);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, "a\\");
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, TrimsExactMultilineIndentation) {
  const std::string source = "\"\"\"\n\t first\n\t   second\n\t third\n\t \"\"\"";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 2u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).mode, StringMode::EscapedMultiline);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, "first\n  second\nthird");
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, EmptyPrefixPreservesBodyIndentation) {
  const std::string source = "\"\"\"\n  first\n    second\n\"\"\"";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 2u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, "  first\n    second");
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, ExcludesBoundaryLineBreaksAndSupportsTrailingLineBreak) {
  const std::string empty_source = "\"\"\"\n    \"\"\"";
  const std::string text_source = "\"\"\"\n    first\n    second\n    \"\"\"";
  const std::string trailing_source = "\"\"\"\n    first\n    \n    \"\"\"";
  const LexedFile empty_file = tokenize(empty_source);
  const LexedFile text_file = tokenize(text_source);
  const LexedFile trailing_file = tokenize(trailing_source);

  ASSERT_TRUE(empty_file.succeeded());
  ASSERT_TRUE(text_file.succeeded());
  ASSERT_TRUE(trailing_file.succeeded());
  EXPECT_EQ(StringPayload(empty_file.tokens()[0]).decoded, "");
  EXPECT_EQ(StringPayload(text_file.tokens()[0]).decoded, "first\nsecond");
  EXPECT_EQ(StringPayload(trailing_file.tokens()[0]).decoded, "first\n");
  ExpectPartition(empty_file);
  ExpectPartition(text_file);
  ExpectPartition(trailing_file);
}

TEST(RawAndMultilineStringTest, HandlesWhitespaceOnlyLinesRelativeToClosingIndentation) {
  const std::string source = "\"\"\"\n    first\n      \n   \n    \"\"\"";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 2u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, "first\n  \n");
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, NormalizesCrlfToLfOnlyInDecodedValue) {
  const std::string lf_source = "\"\"\"\n  first\n  second\n  \"\"\"";
  const std::string crlf_source = "\"\"\"\r\n  first\r\n  second\r\n  \"\"\"";
  const LexedFile lf_file = tokenize(lf_source);
  const LexedFile crlf_file = tokenize(crlf_source);

  ASSERT_TRUE(lf_file.succeeded());
  ASSERT_TRUE(crlf_file.succeeded());
  EXPECT_EQ(StringPayload(lf_file.tokens()[0]).decoded, "first\nsecond");
  EXPECT_EQ(StringPayload(crlf_file.tokens()[0]).decoded, "first\nsecond");
  EXPECT_EQ(std::string(crlf_file.raw(crlf_file.tokens()[0])), crlf_source);
  EXPECT_NE(std::string(lf_file.raw(lf_file.tokens()[0])), std::string(crlf_file.raw(crlf_file.tokens()[0])));
  ExpectPartition(lf_file);
  ExpectPartition(crlf_file);
}

TEST(RawAndMultilineStringTest, MidlineTripleQuotesAreBodyText) {
  const std::string source = R"ink(r"""
  before """ after
  """)ink";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 2u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).mode, StringMode::RawMultiline);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, "before \"\"\" after");
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, ClosingDelimiterMayBeFollowedBySyntaxOnSameLine) {
  const std::string source = "\"\"\"\n  value\n  \"\"\";";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 3u);
  ExpectToken(file, 0, TokenKind::StringLiteral, "\"\"\"\n  value\n  \"\"\"");
  ExpectToken(file, 1, TokenKind::Symbol, ";");
  EXPECT_EQ(std::get<char>(file.tokens()[1].payload), ';');
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, EscapedMultilineProcessesEscapesAfterIndentation) {
  const std::string source = R"ink("""
    tab:\tvalue
    quote:\"
    triple:\"\"\"
    emoji:\u{1F600}
    """)ink";
  const std::string expected = std::string("tab:\tvalue\nquote:\"\ntriple:\"\"\"\nemoji:") + "\xF0\x9F\x98\x80";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 2u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, expected);
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, RawMultilineKeepsEscapesInterpolationAndCommentMarkersAsText) {
  const std::string source = R"ink(r"""
  \n \u{200B} ${value} // /* text */
  """)ink";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 2u);
  EXPECT_EQ(StringPayload(file.tokens()[0]).decoded, R"ink(\n \u{200B} ${value} // /* text */)ink");
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, RejectsInlineTripleQuotedForms) {
  const std::vector<std::string> sources = {R"ink("""inline""")ink", R"ink(r"""inline""")ink"};
  for (const std::string& source : sources) {
    SCOPED_TRACE(source);
    const LexedFile file = tokenize(source);
    EXPECT_FALSE(file.succeeded());
    EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::MultilineOpeningLineBreakRequired));
    ExpectPartition(file);
  }
}

TEST(RawAndMultilineStringTest, InlineOpeningRecoveryPrefersANextLineClosingCandidate) {
  const std::string source = "\"\"\"inline\nbody \"\"\" middle\n  \"\"\"; next";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  ASSERT_GE(file.tokens().size(), 5u);
  ExpectToken(file, 0, TokenKind::InvalidStringLiteral, "\"\"\"inline\nbody \"\"\" middle\n  \"\"\"");
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::MultilineOpeningLineBreakRequired));
  EXPECT_FALSE(HasDiagnostic(file, DiagnosticKind::UnterminatedMultilineStringLiteral));
  ExpectToken(file, 1, TokenKind::Symbol, ";");
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, StopsRawSingleLineAtPhysicalLineBreakAndRecovers) {
  const std::string source = "r\"oops\nnext";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 4u);
  ExpectToken(file, 0, TokenKind::InvalidStringLiteral, "r\"oops");
  ExpectToken(file, 1, TokenKind::LineBreak, "\n");
  ExpectToken(file, 2, TokenKind::Identifier, "next");
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::UnterminatedStringLiteral));
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, RejectsMissingMultilineClosingDelimiter) {
  const std::vector<std::string> sources = {"\"\"\"\n  text", "r\"\"\"\n  text", "r\"\"\"\n  before \"\"\" after"};
  for (const std::string& source : sources) {
    SCOPED_TRACE(source);
    const LexedFile file = tokenize(source);
    EXPECT_FALSE(file.succeeded());
    EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::UnterminatedMultilineStringLiteral));
    ExpectPartition(file);
  }
}

TEST(RawAndMultilineStringTest, RejectsMismatchedBodyIndentation) {
  const std::vector<std::string> sources = {"\"\"\"\n  too shallow\n    \"\"\"", "\"\"\"\n    spaces\n\t\"\"\"", "\"\"\"\n  \n\t \"\"\""};
  for (const std::string& source : sources) {
    SCOPED_TRACE(source);
    const LexedFile file = tokenize(source);
    EXPECT_FALSE(file.succeeded());
    EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::InvalidMultilineIndentation));
    ExpectPartition(file);
  }
}

TEST(RawAndMultilineStringTest, RejectsEscapeFollowedByPhysicalLineBreak) {
  const std::string source = "\"\"\"\n  first\\\n  second\n  \"\"\"";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::UnknownEscape));
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, RejectsUnknownEscapeInEscapedMultilineMode) {
  const std::string source = R"ink("""
  bad:\q
  """)ink";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::UnknownEscape));
  ExpectPartition(file);
}

TEST(RawAndMultilineStringTest, RejectsDirectInvisibleCharacterInRawModes) {
  const std::string invisible = "\xE2\x80\x8B";
  const std::vector<std::string> sources = {std::string("r\"a") + invisible + "b\"", std::string("r\"\"\"\n  a") + invisible + "b\n  \"\"\""};
  for (const std::string& source : sources) {
    SCOPED_TRACE(source);
    const LexedFile file = tokenize(source);
    EXPECT_FALSE(file.succeeded());
    EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::InvisibleCharacter));
    ExpectPartition(file);
  }
}

TEST(RawAndMultilineStringTest, RawModeCannotBypassForbiddenSourceControls) {
  std::string single_source = "r\"before";
  single_source.push_back('\0');
  single_source += "after\"";
  std::string multiline_source = "r\"\"\"\n  before";
  multiline_source.push_back('\x1B');
  multiline_source += "after\n  \"\"\"";
  const LexedFile single_file = tokenize(single_source);
  const LexedFile multiline_file = tokenize(multiline_source);

  EXPECT_FALSE(single_file.succeeded());
  EXPECT_FALSE(multiline_file.succeeded());
  EXPECT_TRUE(HasDiagnostic(single_file, DiagnosticKind::ForbiddenControlCharacter));
  EXPECT_TRUE(HasDiagnostic(multiline_file, DiagnosticKind::ForbiddenControlCharacter));
  ExpectPartition(single_file);
  ExpectPartition(multiline_file);
}

}  // namespace
}  // namespace ink::tokenizer
