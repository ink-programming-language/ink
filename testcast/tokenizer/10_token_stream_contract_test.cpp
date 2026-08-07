#include "ink/tokenizer/tokenizer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::tokenizer {
namespace {

static_assert(!std::is_default_constructible_v<LexedFile>);

void ExpectPartition(const LexedFile& file) {
  ASSERT_FALSE(file.tokens().empty());
  std::size_t cursor = 0;
  std::size_t eof_count = 0;
  std::string rebuilt;
  for (std::size_t index = 0; index < file.tokens().size(); ++index) {
    const Token& token = file.tokens()[index];
    if (token.kind == TokenKind::EndOfFile) {
      ++eof_count;
      EXPECT_EQ(index, file.tokens().size() - 1);
      EXPECT_EQ(token.span, (ByteSpan{file.source().size(), file.source().size()}));
      EXPECT_TRUE(file.raw(token).empty());
      continue;
    }
    EXPECT_LT(token.span.start, token.span.end);
    EXPECT_EQ(token.span.start, cursor);
    EXPECT_EQ(token.span.size(), file.raw(token).size());
    rebuilt.append(file.raw(token).data(), file.raw(token).size());
    cursor = token.span.end;
  }
  EXPECT_EQ(cursor, file.source().size());
  EXPECT_EQ(rebuilt, file.source());
  EXPECT_EQ(eof_count, 1u);
}

void ExpectToken(const LexedFile& file, std::size_t index, TokenKind kind, std::string_view raw, ByteSpan span) {
  ASSERT_LT(index, file.tokens().size());
  EXPECT_EQ(file.tokens()[index].kind, kind);
  EXPECT_EQ(std::string(file.raw(file.tokens()[index])), std::string(raw));
  EXPECT_EQ(file.tokens()[index].span, span);
}

bool HasDiagnostic(const LexedFile& file, DiagnosticKind kind) {
  return std::any_of(file.diagnostics().begin(), file.diagnostics().end(), [kind](const Diagnostic& diagnostic) { return diagnostic.kind == kind; });
}

std::vector<const Token*> SyntaxTokens(const LexedFile& file) {
  std::vector<const Token*> result;
  for (const Token& token : file.tokens()) {
    if (!token.is_trivia() && token.kind != TokenKind::EndOfFile) {
      result.push_back(&token);
    }
  }
  return result;
}

TEST(TokenStreamContractTest, EmptySourceHasExactlyOneEofToken) {
  const LexedFile file = tokenize("");

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 1u);
  EXPECT_EQ(file.tokens()[0].kind, TokenKind::EndOfFile);
  EXPECT_EQ(file.tokens()[0].span, (ByteSpan{0, 0}));
  EXPECT_TRUE(file.raw(file.tokens()[0]).empty());
  EXPECT_FALSE(file.tokens()[0].is_trivia());
  EXPECT_FALSE(file.tokens()[0].is_error());
  ExpectPartition(file);
}

TEST(TokenStreamContractTest, MixedSourceIsAnExactContiguousBytePartition) {
  const std::string source = "\xEF\xBB\xBFlet x = \"v\";\r\n//tail";
  const LexedFile file = tokenize(source);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 12u);
  ExpectToken(file, 0, TokenKind::Utf8Bom, "\xEF\xBB\xBF", ByteSpan{0, 3});
  ExpectToken(file, 1, TokenKind::Keyword, "let", ByteSpan{3, 6});
  ExpectToken(file, 2, TokenKind::SpacesAndTabs, " ", ByteSpan{6, 7});
  ExpectToken(file, 3, TokenKind::Identifier, "x", ByteSpan{7, 8});
  ExpectToken(file, 4, TokenKind::SpacesAndTabs, " ", ByteSpan{8, 9});
  ExpectToken(file, 5, TokenKind::Symbol, "=", ByteSpan{9, 10});
  ExpectToken(file, 6, TokenKind::SpacesAndTabs, " ", ByteSpan{10, 11});
  ExpectToken(file, 7, TokenKind::StringLiteral, "\"v\"", ByteSpan{11, 14});
  ExpectToken(file, 8, TokenKind::Symbol, ";", ByteSpan{14, 15});
  ExpectToken(file, 9, TokenKind::LineBreak, "\r\n", ByteSpan{15, 17});
  ExpectToken(file, 10, TokenKind::LineComment, "//tail", ByteSpan{17, 23});
  ExpectToken(file, 11, TokenKind::EndOfFile, "", ByteSpan{23, 23});
  ExpectPartition(file);
}

TEST(TokenStreamContractTest, LexedFileOwnsSourceUsedByRawViews) {
  LexedFile file = tokenize(std::string("persistent identifier"));

  ASSERT_TRUE(file.succeeded());
  EXPECT_EQ(file.source(), "persistent identifier");
  ExpectToken(file, 0, TokenKind::Identifier, "persistent", ByteSpan{0, 10});
  ExpectToken(file, 2, TokenKind::Identifier, "identifier", ByteSpan{11, 21});
  ExpectPartition(file);
}

TEST(TokenStreamContractTest, TriviaAndErrorClassificationsAreOrthogonalAndExhaustive) {
  const std::array<TokenKind, 23> all_kinds = {TokenKind::Utf8Bom, TokenKind::SpacesAndTabs, TokenKind::LineBreak, TokenKind::LineComment, TokenKind::BlockComment, TokenKind::Identifier, TokenKind::Keyword, TokenKind::BuiltinType, TokenKind::BoolLiteral, TokenKind::NullLiteral, TokenKind::IntegerLiteral, TokenKind::FloatLiteral, TokenKind::ScalarLiteral, TokenKind::StringLiteral, TokenKind::Symbol, TokenKind::InvalidEncoding, TokenKind::InvalidCharacter, TokenKind::InvalidIdentifier, TokenKind::InvalidNumber, TokenKind::InvalidScalarLiteral, TokenKind::InvalidStringLiteral, TokenKind::UnterminatedBlockComment, TokenKind::EndOfFile};
  const std::array<TokenKind, 5> trivia_kinds = {TokenKind::Utf8Bom, TokenKind::SpacesAndTabs, TokenKind::LineBreak, TokenKind::LineComment, TokenKind::BlockComment};
  const std::array<TokenKind, 7> error_kinds = {TokenKind::InvalidEncoding, TokenKind::InvalidCharacter, TokenKind::InvalidIdentifier, TokenKind::InvalidNumber, TokenKind::InvalidScalarLiteral, TokenKind::InvalidStringLiteral, TokenKind::UnterminatedBlockComment};

  for (TokenKind kind : all_kinds) {
    const bool expected_trivia = std::find(trivia_kinds.begin(), trivia_kinds.end(), kind) != trivia_kinds.end();
    const bool expected_error = std::find(error_kinds.begin(), error_kinds.end(), kind) != error_kinds.end();
    SCOPED_TRACE(token_kind_name(kind));
    EXPECT_EQ(is_trivia(kind), expected_trivia);
    EXPECT_EQ(is_error(kind), expected_error);
    EXPECT_FALSE(is_trivia(kind) && is_error(kind));
  }
}

TEST(TokenStreamContractTest, DerivedPayloadsMatchTheirRawTokens) {
  const std::string source = "let i32 true false null 0xFFu8 1.5f32 'A' \"x\" +";
  const LexedFile file = tokenize(source);
  const std::vector<const Token*> tokens = SyntaxTokens(file);

  ASSERT_TRUE(file.succeeded());
  ASSERT_EQ(tokens.size(), 10u);
  EXPECT_EQ(tokens[0]->kind, TokenKind::Keyword);
  EXPECT_EQ(std::get<KeywordKind>(tokens[0]->payload), KeywordKind::Let);
  EXPECT_EQ(tokens[1]->kind, TokenKind::BuiltinType);
  EXPECT_EQ(std::get<BuiltinTypeKind>(tokens[1]->payload), BuiltinTypeKind::I32);
  EXPECT_EQ(tokens[2]->kind, TokenKind::BoolLiteral);
  EXPECT_TRUE(std::get<bool>(tokens[2]->payload));
  EXPECT_EQ(tokens[3]->kind, TokenKind::BoolLiteral);
  EXPECT_FALSE(std::get<bool>(tokens[3]->payload));
  EXPECT_EQ(tokens[4]->kind, TokenKind::NullLiteral);
  EXPECT_TRUE(std::holds_alternative<std::monostate>(tokens[4]->payload));
  EXPECT_EQ(tokens[5]->kind, TokenKind::IntegerLiteral);
  EXPECT_EQ(std::get<NumericInfo>(tokens[5]->payload).base, 16u);
  EXPECT_EQ(std::get<NumericInfo>(tokens[5]->payload).suffix, NumericSuffix::U8);
  EXPECT_EQ(tokens[6]->kind, TokenKind::FloatLiteral);
  EXPECT_EQ(std::get<NumericInfo>(tokens[6]->payload).base, 10u);
  EXPECT_EQ(std::get<NumericInfo>(tokens[6]->payload).suffix, NumericSuffix::F32);
  EXPECT_EQ(tokens[7]->kind, TokenKind::ScalarLiteral);
  EXPECT_EQ(std::get<char32_t>(tokens[7]->payload), U'A');
  EXPECT_EQ(tokens[8]->kind, TokenKind::StringLiteral);
  EXPECT_EQ(std::get<StringInfo>(tokens[8]->payload).mode, StringMode::EscapedSingleLine);
  EXPECT_EQ(std::get<StringInfo>(tokens[8]->payload).decoded, "x");
  EXPECT_EQ(tokens[9]->kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(tokens[9]->payload), '+');
  ExpectPartition(file);
}

TEST(TokenStreamContractTest, EveryLexicalErrorKindMakesTheResultFail) {
  std::string invalid_utf8(1, static_cast<char>(0x80));
  const std::string decomposed_identifier = "cafe\xCC\x81";
  const std::vector<std::pair<std::string, TokenKind>> cases = {{invalid_utf8, TokenKind::InvalidEncoding}, {"?", TokenKind::InvalidCharacter}, {decomposed_identifier, TokenKind::InvalidIdentifier}, {"0x", TokenKind::InvalidNumber}, {"''", TokenKind::InvalidScalarLiteral}, {"\"", TokenKind::InvalidStringLiteral}, {"/*", TokenKind::UnterminatedBlockComment}};
  for (const auto& test_case : cases) {
    SCOPED_TRACE(test_case.first);
    const LexedFile file = tokenize(test_case.first);
    ASSERT_FALSE(file.succeeded());
    ASSERT_FALSE(file.diagnostics().empty());
    ASSERT_GE(file.tokens().size(), 2u);
    EXPECT_EQ(file.tokens()[0].kind, test_case.second);
    EXPECT_TRUE(file.tokens()[0].is_error());
    EXPECT_FALSE(file.tokens()[0].is_trivia());
    ExpectPartition(file);
  }
}

TEST(TokenStreamContractTest, InvalidUtf8DoesNotConsumeFollowingKeyword) {
  std::string source(1, static_cast<char>(0x80));
  source += "let";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 3u);
  ExpectToken(file, 0, TokenKind::InvalidEncoding, std::string(1, static_cast<char>(0x80)), ByteSpan{0, 1});
  ExpectToken(file, 1, TokenKind::Keyword, "let", ByteSpan{1, 4});
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::InvalidUtf8));
  ExpectPartition(file);
}

TEST(TokenStreamContractTest, TruncatedUtf8SequenceDoesNotConsumeFollowingIdentifier) {
  std::string source;
  source.push_back(static_cast<char>(0xE2));
  source.push_back(static_cast<char>(0x82));
  source += "x";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 3u);
  ExpectToken(file, 0, TokenKind::InvalidEncoding, source.substr(0, 2), ByteSpan{0, 2});
  ExpectToken(file, 1, TokenKind::Identifier, "x", ByteSpan{2, 3});
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::InvalidUtf8));
  ExpectPartition(file);
}

TEST(TokenStreamContractTest, InvalidCharacterBetweenIdentifiersDoesNotPreventRecovery) {
  const LexedFile file = tokenize("before?after");

  ASSERT_FALSE(file.succeeded());
  ASSERT_EQ(file.tokens().size(), 4u);
  ExpectToken(file, 0, TokenKind::Identifier, "before", ByteSpan{0, 6});
  ExpectToken(file, 1, TokenKind::InvalidCharacter, "?", ByteSpan{6, 7});
  ExpectToken(file, 2, TokenKind::Identifier, "after", ByteSpan{7, 12});
  ExpectPartition(file);
}

TEST(TokenStreamContractTest, ScannerAlwaysAdvancesAcrossRepeatedInvalidInput) {
  const std::string source = "????$$$$####````\\\\";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  ASSERT_GE(file.tokens().size(), 2u);
  ASSERT_LE(file.tokens().size(), source.size() + 1);
  for (const Token& token : file.tokens()) {
    if (token.kind != TokenKind::EndOfFile) {
      EXPECT_FALSE(token.span.empty());
      EXPECT_TRUE(token.is_error());
    }
  }
  ExpectPartition(file);
}

TEST(TokenStreamContractTest, ArbitraryByteInputsAlwaysAdvanceAndPreserveTheExactPartition) {
  std::mt19937 generator(0x1A2B3C4D);
  std::uniform_int_distribution<int> length_distribution(0, 64);
  std::uniform_int_distribution<int> byte_distribution(0, 255);
  for (std::size_t case_index = 0; case_index < 1000; ++case_index) {
    std::string source(static_cast<std::size_t>(length_distribution(generator)), '\0');
    for (char& value : source) {
      value = static_cast<char>(byte_distribution(generator));
    }
    SCOPED_TRACE(case_index);
    const LexedFile file = tokenize(source);
    ASSERT_LE(file.tokens().size(), source.size() + 1);
    ExpectPartition(file);
  }
}

TEST(TokenStreamContractTest, DiagnosticsUseSourceByteSpans) {
  const std::string source = "ok ? 0x \"unterminated";
  const LexedFile file = tokenize(source);

  ASSERT_FALSE(file.succeeded());
  ASSERT_FALSE(file.diagnostics().empty());
  for (const Diagnostic& diagnostic : file.diagnostics()) {
    EXPECT_LE(diagnostic.span.start, diagnostic.span.end);
    EXPECT_LE(diagnostic.span.end, source.size());
  }
  ExpectPartition(file);
}

TEST(TokenStreamContractTest, RepeatedTokenizationIsDeterministic) {
  const std::string source = "let value: i32 = 0xFFu8; // comment\r\n\"text\\n\"";
  const LexedFile first = tokenize(source);
  const LexedFile second = tokenize(source);

  ASSERT_EQ(first.succeeded(), second.succeeded());
  ASSERT_EQ(first.tokens().size(), second.tokens().size());
  ASSERT_EQ(first.diagnostics().size(), second.diagnostics().size());
  for (std::size_t index = 0; index < first.tokens().size(); ++index) {
    EXPECT_EQ(first.tokens()[index].kind, second.tokens()[index].kind);
    EXPECT_EQ(first.tokens()[index].span, second.tokens()[index].span);
    EXPECT_EQ(first.raw(first.tokens()[index]), second.raw(second.tokens()[index]));
    EXPECT_EQ(first.tokens()[index].payload.index(), second.tokens()[index].payload.index());
  }
  for (std::size_t index = 0; index < first.diagnostics().size(); ++index) {
    EXPECT_EQ(first.diagnostics()[index].kind, second.diagnostics()[index].kind);
    EXPECT_EQ(first.diagnostics()[index].span, second.diagnostics()[index].span);
  }
  ExpectPartition(first);
  ExpectPartition(second);
}

TEST(TokenStreamContractTest, SyntacticallyInvalidButLexicallyValidSourcesStillSucceed) {
  const std::vector<std::string> sources = {")(", "unknown_name", "let", "{[(", "import \"definitely-missing.ink\"", "a < /* gap */ = b", "\"value\"name", "999999999999999999999999999999999999999999i8"};
  for (const std::string& source : sources) {
    SCOPED_TRACE(source);
    const LexedFile file = tokenize(source);
    EXPECT_TRUE(file.succeeded());
    EXPECT_TRUE(file.diagnostics().empty());
    EXPECT_TRUE(std::none_of(file.tokens().begin(), file.tokens().end(), [](const Token& token) { return token.is_error(); }));
    ExpectPartition(file);
  }
}

TEST(TokenStreamContractTest, BlockCommentDepthLimitFailsWithoutBreakingPartition) {
  const std::string source = "/* outer /* inner */ outer */ after";
  TokenizerOptions options;
  options.max_block_comment_depth = 1;
  const LexedFile file = tokenize(source, options);

  ASSERT_FALSE(file.succeeded());
  EXPECT_TRUE(HasDiagnostic(file, DiagnosticKind::BlockCommentNestingLimit));
  EXPECT_TRUE(std::any_of(file.tokens().begin(), file.tokens().end(), [](const Token& token) { return token.is_error(); }));
  ExpectPartition(file);
}

}  // namespace
}  // namespace ink::tokenizer
