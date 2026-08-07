#include "ink/tokenizer/tokenizer.h"

#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace ink::tokenizer {
namespace {

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

bool HasErrorToken(const LexedFile& lexed) {
  for (const Token& token : lexed.tokens()) {
    if (token.is_error()) {
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

TEST(TriviaCommentsTest, TriviaTokensShareTheSingleOrderedTokenList) {
  const std::string source = Utf8(u8"\uFEFF \tlet\r\n//comment\n/**/");
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 8U);
  const std::vector<TokenKind> expected = {TokenKind::Utf8Bom, TokenKind::SpacesAndTabs, TokenKind::Keyword, TokenKind::LineBreak, TokenKind::LineComment, TokenKind::LineBreak, TokenKind::BlockComment, TokenKind::EndOfFile};
  for (std::size_t index = 0; index < expected.size(); ++index) {
    EXPECT_EQ(lexed.tokens()[index].kind, expected[index]);
  }
  for (std::size_t index = 0; index + 1 < lexed.tokens().size(); ++index) {
    EXPECT_EQ(lexed.tokens()[index].is_trivia(), index != 2U);
  }
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, AdjacentSpacesAndTabsFormOneMaximalTriviaToken) {
  const LexedFile lexed = tokenize(" \t  \t");

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 2U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::SpacesAndTabs);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), " \t  \t");
  EXPECT_TRUE(lexed.tokens()[0].is_trivia());
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, LineCommentExcludesLfOrCrLfTerminator) {
  struct LineEndingCase {
    const char* source;
    const char* line_break;
  };
  const std::vector<LineEndingCase> cases = {{"// comment\n", "\n"}, {"// comment\r\n", "\r\n"}};

  for (const LineEndingCase& test_case : cases) {
    SCOPED_TRACE(test_case.line_break);
    const LexedFile lexed = tokenize(test_case.source);
    ASSERT_TRUE(lexed.succeeded());
    ASSERT_EQ(lexed.tokens().size(), 3U);
    EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::LineComment);
    EXPECT_EQ(lexed.raw(lexed.tokens()[0]), "// comment");
    EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::LineBreak);
    EXPECT_EQ(lexed.raw(lexed.tokens()[1]), test_case.line_break);
    ExpectFullFidelity(lexed);
  }
}

TEST(TriviaCommentsTest, LineCommentMayEndAtEof) {
  const LexedFile lexed = tokenize("// no final line break");

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 2U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::LineComment);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), "// no final line break");
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, UnicodeNewlineLookalikesDoNotEndLineComments) {
  const std::string source = Utf8(u8"//a\u0085b\u2028c\u2029d\nx");
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 4U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::LineComment);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), Utf8(u8"//a\u0085b\u2028c\u2029d"));
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::LineBreak);
  EXPECT_EQ(lexed.raw(lexed.tokens()[1]), "\n");
  EXPECT_EQ(lexed.tokens()[2].kind, TokenKind::Identifier);
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, NonInitialByteOrderMarkIsPreservedAsCommentText) {
  const std::string source = Utf8(u8"//a\uFEFFb");
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 2U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::LineComment);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), source);
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, NestedBlockCommentIsOneTokenIncludingInternalLineBreaks) {
  const std::string source = "/* outer\n/* inner */ \" // still outer */tail";
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 3U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::BlockComment);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), "/* outer\n/* inner */ \" // still outer */");
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.raw(lexed.tokens()[1]), "tail");
  EXPECT_FALSE(HasTokenKind(lexed, TokenKind::LineBreak));
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, EmptyBlockCommentIsLegal) {
  const LexedFile lexed = tokenize("/**/");

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 2U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::BlockComment);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), "/**/");
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, CommentDelimitersOnlyActInTheActiveLexicalState) {
  const std::string source = "// /* not a block */ \"not a string\"\n\"https://example.com/*text*/\" /* // nested text */";
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 6U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::LineComment);
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::LineBreak);
  EXPECT_EQ(lexed.tokens()[2].kind, TokenKind::StringLiteral);
  EXPECT_EQ(lexed.tokens()[3].kind, TokenKind::SpacesAndTabs);
  EXPECT_EQ(lexed.tokens()[4].kind, TokenKind::BlockComment);
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, TriviaForcesIdentifierAndSymbolBoundaries) {
  const std::string source = "first/* comment */second +/* comment */+";
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  const std::vector<TokenKind> expected = {TokenKind::Identifier, TokenKind::BlockComment, TokenKind::Identifier, TokenKind::SpacesAndTabs, TokenKind::Symbol, TokenKind::BlockComment, TokenKind::Symbol, TokenKind::EndOfFile};
  ASSERT_EQ(lexed.tokens().size(), expected.size());
  for (std::size_t index = 0; index < expected.size(); ++index) {
    EXPECT_EQ(lexed.tokens()[index].kind, expected[index]);
  }
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), "first");
  EXPECT_EQ(lexed.raw(lexed.tokens()[2]), "second");
  EXPECT_EQ(std::get<char>(lexed.tokens()[4].payload), '+');
  EXPECT_EQ(std::get<char>(lexed.tokens()[6].payload), '+');
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, DocumentationLikeCommentsRemainOrdinaryComments) {
  const std::string source = "/// line\n//! inner\n/** block */ /*! inner */";
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  const std::vector<TokenKind> expected = {TokenKind::LineComment, TokenKind::LineBreak, TokenKind::LineComment, TokenKind::LineBreak, TokenKind::BlockComment, TokenKind::SpacesAndTabs, TokenKind::BlockComment, TokenKind::EndOfFile};
  ASSERT_EQ(lexed.tokens().size(), expected.size());
  for (std::size_t index = 0; index < expected.size(); ++index) {
    EXPECT_EQ(lexed.tokens()[index].kind, expected[index]);
  }
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, StarSlashOutsideBlockCommentIsTwoSymbols) {
  const LexedFile lexed = tokenize("*/");

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 3U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(lexed.tokens()[0].payload), '*');
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(lexed.tokens()[1].payload), '/');
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, UnterminatedNestedBlockCommentCoversFromOutermostStartToEof) {
  const std::string source = "/* outer /* inner";
  const LexedFile lexed = tokenize(source);

  ASSERT_FALSE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 2U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::UnterminatedBlockComment);
  EXPECT_EQ(lexed.tokens()[0].span.start, 0U);
  EXPECT_EQ(lexed.tokens()[0].span.end, source.size());
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), source);
  EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::UnterminatedBlockComment));
  bool detailed_diagnostic_found = false;
  for (const Diagnostic& diagnostic : lexed.diagnostics()) {
    if (diagnostic.kind == DiagnosticKind::UnterminatedBlockComment && diagnostic.message.find("outermost opening byte: 0") != std::string::npos && diagnostic.message.find("remaining nesting depth: 2") != std::string::npos && diagnostic.message.find("most recent unclosed opening byte: 9") != std::string::npos) {
      detailed_diagnostic_found = true;
    }
  }
  EXPECT_TRUE(detailed_diagnostic_found);
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, ConfiguredBlockCommentNestingLimitProducesAnErrorWithoutLosingBytes) {
  const std::string source = "/* one /* two /* three */ two */ one */tail";
  const LexedFile lexed = tokenize(source, TokenizerOptions{2});

  EXPECT_FALSE(lexed.succeeded());
  EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::BlockCommentNestingLimit));
  EXPECT_TRUE(HasErrorToken(lexed));
  ExpectFullFidelity(lexed);
}

TEST(TriviaCommentsTest, BackslashBeforeLineBreakDoesNotContinueTheLine) {
  const LexedFile lexed = tokenize("a\\\nb");

  ASSERT_FALSE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 5U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::InvalidCharacter);
  EXPECT_EQ(lexed.raw(lexed.tokens()[1]), "\\");
  EXPECT_EQ(lexed.tokens()[2].kind, TokenKind::LineBreak);
  EXPECT_EQ(lexed.tokens()[3].kind, TokenKind::Identifier);
  ExpectFullFidelity(lexed);
}

}  // namespace
}  // namespace ink::tokenizer
