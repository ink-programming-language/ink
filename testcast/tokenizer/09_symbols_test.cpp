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

TEST(SymbolTokenTest, LexesEveryAcceptedSymbolAsOneByteToken) {
  const std::string symbols = "(){}[],;:.@+-*/%=!&|^~<>";
  const LexedFile file = tokenize(symbols);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), symbols.size() + 1);
  for (std::size_t index = 0; index < symbols.size(); ++index) {
    SCOPED_TRACE(index);
    EXPECT_EQ(file.tokens()[index].kind, TokenKind::Symbol);
    EXPECT_EQ(file.tokens()[index].span, (ByteSpan{index, index + 1}));
    EXPECT_EQ(file.raw(file.tokens()[index]), std::string_view(symbols).substr(index, 1));
    ASSERT_TRUE(std::holds_alternative<char>(file.tokens()[index].payload));
    EXPECT_EQ(std::get<char>(file.tokens()[index].payload), symbols[index]);
  }
  ExpectPartition(file);
}

TEST(SymbolTokenTest, NeverCombinesCompoundPunctuationOrOperators) {
  const std::vector<std::string> spellings = {"::", "..", "...", "->", "=>", "==", "!=", "<=", ">=", "&&", "||", "<<", ">>", "+=", ">>="};
  for (const std::string& spelling : spellings) {
    SCOPED_TRACE(spelling);
    const LexedFile file = tokenize(spelling);
    ASSERT_TRUE(file.succeeded());
    ASSERT_EQ(file.tokens().size(), spelling.size() + 1);
    for (std::size_t index = 0; index < spelling.size(); ++index) {
      EXPECT_EQ(file.tokens()[index].kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(file.tokens()[index].payload), spelling[index]);
      EXPECT_EQ(file.tokens()[index].span, (ByteSpan{index, index + 1}));
    }
    ExpectPartition(file);
  }
}

TEST(SymbolTokenTest, TriviaPreservesCompoundSymbolBoundaries) {
  const LexedFile adjacent = tokenize("a<=b");
  const LexedFile spaced = tokenize("a < = b");
  const LexedFile commented = tokenize("a < /*c*/ = b");

  ASSERT_TRUE(adjacent.succeeded());
  ASSERT_EQ(adjacent.tokens().size(), 5u);
  ExpectToken(adjacent, 1, TokenKind::Symbol, "<");
  ExpectToken(adjacent, 2, TokenKind::Symbol, "=");
  EXPECT_EQ(adjacent.tokens()[1].span.end, adjacent.tokens()[2].span.start);

  ASSERT_TRUE(spaced.succeeded());
  ASSERT_EQ(spaced.tokens().size(), 8u);
  ExpectToken(spaced, 2, TokenKind::Symbol, "<");
  ExpectToken(spaced, 3, TokenKind::SpacesAndTabs, " ");
  ExpectToken(spaced, 4, TokenKind::Symbol, "=");
  EXPECT_NE(spaced.tokens()[2].span.end, spaced.tokens()[4].span.start);

  ASSERT_TRUE(commented.succeeded());
  ASSERT_EQ(commented.tokens().size(), 10u);
  ExpectToken(commented, 2, TokenKind::Symbol, "<");
  ExpectToken(commented, 4, TokenKind::BlockComment, "/*c*/");
  ExpectToken(commented, 6, TokenKind::Symbol, "=");
  EXPECT_NE(commented.tokens()[2].span.end, commented.tokens()[6].span.start);
  ExpectPartition(adjacent);
  ExpectPartition(spaced);
  ExpectPartition(commented);
}

TEST(SymbolTokenTest, NestedGenericClosersRemainSeparateSymbols) {
  const LexedFile nested = tokenize("Vector<Vector<i32>>");
  const LexedFile assignment = tokenize("Container<Item<T>>=value");

  ASSERT_TRUE(nested.succeeded());
  ASSERT_EQ(nested.tokens().size(), 8u);
  ExpectToken(nested, 5, TokenKind::Symbol, ">");
  ExpectToken(nested, 6, TokenKind::Symbol, ">");
  EXPECT_EQ(nested.tokens()[5].span.end, nested.tokens()[6].span.start);

  ASSERT_TRUE(assignment.succeeded());
  ASSERT_GE(assignment.tokens().size(), 5u);
  const std::size_t value_index = assignment.tokens().size() - 2;
  ExpectToken(assignment, value_index - 3, TokenKind::Symbol, ">");
  ExpectToken(assignment, value_index - 2, TokenKind::Symbol, ">");
  ExpectToken(assignment, value_index - 1, TokenKind::Symbol, "=");
  ExpectToken(assignment, value_index, TokenKind::Identifier, "value");
  ExpectPartition(nested);
  ExpectPartition(assignment);
}

TEST(SymbolTokenTest, NumericScannerOwnsOnlyDecimalPointFollowedByDigit) {
  const std::string source = "1.member 1..10 0...value";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 15u);
  ExpectToken(file, 0, TokenKind::IntegerLiteral, "1");
  ExpectToken(file, 1, TokenKind::Symbol, ".");
  ExpectToken(file, 2, TokenKind::Identifier, "member");
  ExpectToken(file, 4, TokenKind::IntegerLiteral, "1");
  ExpectToken(file, 5, TokenKind::Symbol, ".");
  ExpectToken(file, 6, TokenKind::Symbol, ".");
  ExpectToken(file, 7, TokenKind::IntegerLiteral, "10");
  ExpectToken(file, 9, TokenKind::IntegerLiteral, "0");
  ExpectToken(file, 10, TokenKind::Symbol, ".");
  ExpectToken(file, 11, TokenKind::Symbol, ".");
  ExpectToken(file, 12, TokenKind::Symbol, ".");
  ExpectToken(file, 13, TokenKind::Identifier, "value");
  ExpectPartition(file);
}

TEST(SymbolTokenTest, CommentDelimitersTakePriorityOverSlashSymbols) {
  const std::string source = "//line\n/* block */ / / / * */";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 15u);
  ExpectToken(file, 0, TokenKind::LineComment, "//line");
  ExpectToken(file, 1, TokenKind::LineBreak, "\n");
  ExpectToken(file, 2, TokenKind::BlockComment, "/* block */");
  ExpectToken(file, 4, TokenKind::Symbol, "/");
  ExpectToken(file, 6, TokenKind::Symbol, "/");
  ExpectToken(file, 8, TokenKind::Symbol, "/");
  ExpectToken(file, 10, TokenKind::Symbol, "*");
  ExpectToken(file, 12, TokenKind::Symbol, "*");
  ExpectToken(file, 13, TokenKind::Symbol, "/");
  ExpectPartition(file);
}

TEST(SymbolTokenTest, LiteralDelimitersTakePriorityOverSymbolScanning) {
  const std::string source = R"ink('/' "/" r"/*")ink";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 6u);
  ExpectToken(file, 0, TokenKind::ScalarLiteral, "'/'");
  ExpectToken(file, 2, TokenKind::StringLiteral, "\"/\"");
  ExpectToken(file, 4, TokenKind::StringLiteral, "r\"/*\"");
  ExpectPartition(file);
}

TEST(SymbolTokenTest, SameSymbolPayloadIsIndependentOfSyntacticRole) {
  const std::string source = "a*b value:T* *pointer a&b value:T& &value a<b Vector<i32>";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  std::vector<char> symbol_values;
  for (const Token& token : file.tokens()) {
    if (token.kind == TokenKind::Symbol) {
      symbol_values.push_back(std::get<char>(token.payload));
    }
  }
  EXPECT_EQ(symbol_values, (std::vector<char>{'*', ':', '*', '*', '&', ':', '&', '&', '<', '<', '>'}));
  ExpectPartition(file);
}

TEST(SymbolTokenTest, RejectsAsciiCharactersOutsideTheSymbolTable) {
  const std::vector<std::string> sources = {"?", "$", "#", "`", "\\"};
  for (const std::string& source : sources) {
    SCOPED_TRACE(source);
    const LexedFile file = tokenize(source);
    ASSERT_FALSE(file.succeeded());
    ASSERT_EQ(file.tokens().size(), 2u);
    ExpectToken(file, 0, TokenKind::InvalidCharacter, source);
    EXPECT_TRUE(file.tokens()[0].is_error());
    EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::InvalidCharacter));
    ExpectPartition(file);
  }
}

TEST(SymbolTokenTest, RejectsNonIdentifierUnicodeCharacterAsOneErrorToken) {
  const std::string emoji = "\xF0\x9F\x98\x80";
  const LexedFile file = tokenize(emoji);

  ASSERT_FALSE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 2u);
  ExpectToken(file, 0, TokenKind::InvalidCharacter, emoji);
  EXPECT_EQ(file.tokens()[0].span, (ByteSpan{0, 4}));
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::InvalidCharacter));
  ExpectPartition(file);
}

}  // namespace
}  // namespace ink::tokenizer
