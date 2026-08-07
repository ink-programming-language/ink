#include "ink/tokenizer/tokenizer.h"

#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <string>
#include <variant>
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

TEST(IdentifiersTest, XidStartAndContinueAcceptAsciiAndUnicodeIdentifiers) {
  const std::vector<std::string> identifiers = {"value", "_private", "_", "__internal", Utf8(u8"\u7528\u6237"), Utf8(u8"\u7528\u6237ID"), Utf8(u8"\u0394value"), Utf8(u8"\u0434\u0430\u043D\u043D\u044B\u0435"), Utf8(u8"caf\u00E92"), Utf8(u8"user\u6570\u91CF"), Utf8(u8"HTTP\u72B6\u6001")};

  for (const std::string& identifier : identifiers) {
    SCOPED_TRACE(identifier);
    const LexedFile lexed = tokenize(identifier);
    ASSERT_TRUE(lexed.succeeded());
    ASSERT_EQ(lexed.tokens().size(), 2U);
    EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::Identifier);
    EXPECT_EQ(lexed.raw(lexed.tokens()[0]), identifier);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(lexed.tokens()[0].payload));
    ExpectFullFidelity(lexed);
  }
}

TEST(IdentifiersTest, LeadingUnderscoreFollowedByDigitsRemainsAnIdentifier) {
  const LexedFile lexed = tokenize("_100");

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 2U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), "_100");
  ExpectFullFidelity(lexed);
}

TEST(IdentifiersTest, AsciiDigitCannotStartAnIdentifierAndTheNumericCandidateIsRejected) {
  const LexedFile lexed = tokenize("2value");

  ASSERT_FALSE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 2U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::InvalidNumber);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), "2value");
  EXPECT_FALSE(HasTokenKind(lexed, TokenKind::Identifier));
  EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::UnknownNumericSuffix));
  ExpectFullFidelity(lexed);
}

TEST(IdentifiersTest, ValidSymbolEndsAnIdentifierWithoutProducingALexicalError) {
  const LexedFile lexed = tokenize("hello-world");

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 4U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), "hello");
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(lexed.tokens()[1].payload), '-');
  EXPECT_EQ(lexed.tokens()[2].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.raw(lexed.tokens()[2]), "world");
  ExpectFullFidelity(lexed);
}

TEST(IdentifiersTest, InvalidContinuationCharacterIsDiagnosedAndRecoveryKeepsBothIdentifiers) {
  const LexedFile lexed = tokenize("user$name");

  ASSERT_FALSE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 4U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), "user");
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::InvalidCharacter);
  EXPECT_EQ(lexed.raw(lexed.tokens()[1]), "$");
  EXPECT_EQ(lexed.tokens()[2].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.raw(lexed.tokens()[2]), "name");
  EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::InvalidCharacter));
  ExpectFullFidelity(lexed);
}

TEST(IdentifiersTest, CombiningMarkCannotStartAnIdentifier) {
  const std::string source = Utf8(u8"\u0301name");
  const LexedFile lexed = tokenize(source);

  ASSERT_FALSE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 3U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::InvalidCharacter);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), Utf8(u8"\u0301"));
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.raw(lexed.tokens()[1]), "name");
  ExpectFullFidelity(lexed);
}

TEST(IdentifiersTest, EmojiCannotStartAnIdentifierAndDoesNotConsumeFollowingIdentifier) {
  const std::string source = Utf8(u8"\U0001F600value");
  const LexedFile lexed = tokenize(source);

  ASSERT_FALSE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 3U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::InvalidCharacter);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), Utf8(u8"\U0001F600"));
  EXPECT_EQ(lexed.tokens()[1].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.raw(lexed.tokens()[1]), "value");
  ExpectFullFidelity(lexed);
}

TEST(IdentifiersTest, NfcIdentifierIsAcceptedWithoutChangingItsRawSpelling) {
  const std::string source = Utf8(u8"caf\u00E9");
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 2U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), source);
  ExpectFullFidelity(lexed);
}

TEST(IdentifiersTest, CanonicallyEquivalentButNonNfcIdentifierIsRejectedAsOneCandidate) {
  const std::string source = Utf8(u8"cafe\u0301");
  const LexedFile lexed = tokenize(source);

  ASSERT_FALSE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 2U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::InvalidIdentifier);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), source);
  EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::IdentifierNotNfc));
  ExpectFullFidelity(lexed);
}

TEST(IdentifiersTest, IdentifierComparisonAndKeywordLookupAreCaseSensitive) {
  const std::string source = "value Value VALUE func Func function functional";
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 14U);
  EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.tokens()[2].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.tokens()[4].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.tokens()[6].kind, TokenKind::Keyword);
  EXPECT_EQ(std::get<KeywordKind>(lexed.tokens()[6].payload), KeywordKind::Func);
  EXPECT_EQ(lexed.tokens()[8].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.tokens()[10].kind, TokenKind::Identifier);
  EXPECT_EQ(lexed.tokens()[12].kind, TokenKind::Identifier);
  ExpectFullFidelity(lexed);
}

TEST(IdentifiersTest, InvisibleFormatCharactersAreRejectedWithExactSourceSpans) {
  struct InvisibleCase {
    const char* name;
    std::string source;
    std::string invisible;
  };
  const std::vector<InvisibleCase> cases = {{"zero width non joiner", Utf8(u8"a\u200Cb"), Utf8(u8"\u200C")}, {"zero width joiner", Utf8(u8"a\u200Db"), Utf8(u8"\u200D")}, {"zero width space", Utf8(u8"a\u200Bb"), Utf8(u8"\u200B")}, {"bidirectional override", Utf8(u8"a\u202Eb"), Utf8(u8"\u202E")}, {"variation selector", Utf8(u8"a\uFE0Fb"), Utf8(u8"\uFE0F")}};

  for (const InvisibleCase& test_case : cases) {
    SCOPED_TRACE(test_case.name);
    const LexedFile lexed = tokenize(test_case.source);
    ASSERT_FALSE(lexed.succeeded());
    EXPECT_TRUE(HasTokenKind(lexed, TokenKind::InvalidIdentifier) || HasTokenKind(lexed, TokenKind::InvalidCharacter));
    ASSERT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::InvisibleCharacter));
    const std::size_t expected_start = test_case.source.find(test_case.invisible);
    bool found_exact_span = false;
    bool found_detailed_message = false;
    for (const Diagnostic& diagnostic : lexed.diagnostics()) {
      if (diagnostic.kind == DiagnosticKind::InvisibleCharacter && diagnostic.span.start == expected_start && diagnostic.span.end == expected_start + test_case.invisible.size()) {
        found_exact_span = true;
        found_detailed_message = diagnostic.message.find("U+") != std::string::npos && diagnostic.message.find("U+0061 ('a')") != std::string::npos && diagnostic.message.find("U+0062 ('b')") != std::string::npos;
      }
    }
    EXPECT_TRUE(found_exact_span);
    EXPECT_TRUE(found_detailed_message);
    ExpectFullFidelity(lexed);
  }

  const LexedFile standalone = tokenize(Utf8(u8"\u00AD"));
  ASSERT_FALSE(standalone.succeeded());
  ASSERT_EQ(standalone.diagnostics().size(), 1U);
  EXPECT_NE(standalone.diagnostics().front().message.find("U+00AD"), std::string::npos);
  EXPECT_NE(standalone.diagnostics().front().message.find("source text"), std::string::npos);
  EXPECT_EQ(standalone.diagnostics().front().message.find("identifier"), std::string::npos);

  const LexedFile after_space = tokenize(Utf8(u8" \u00ADa"));
  ASSERT_FALSE(after_space.succeeded());
  for (const Diagnostic& diagnostic : after_space.diagnostics()) {
    if (diagnostic.kind == DiagnosticKind::InvisibleCharacter) {
      EXPECT_EQ(diagnostic.message.find("U+0020"), std::string::npos);
    }
  }
}

TEST(IdentifiersTest, MixedWritingSystemsAreLexicallyValid) {
  const std::vector<std::string> cases = {Utf8(u8"user\u6570\u91CF"), Utf8(u8"HTTP\u72B6\u6001")};

  for (const std::string& source : cases) {
    SCOPED_TRACE(source);
    const LexedFile lexed = tokenize(source);
    ASSERT_TRUE(lexed.succeeded());
    ASSERT_EQ(lexed.tokens().size(), 2U);
    EXPECT_EQ(lexed.tokens()[0].kind, TokenKind::Identifier);
    EXPECT_EQ(lexed.raw(lexed.tokens()[0]), source);
    ExpectFullFidelity(lexed);
  }
}

TEST(IdentifiersTest, BacktickHashAndAtSignDoNotEscapeHardKeywords) {
  const LexedFile backtick = tokenize("`func`");
  ASSERT_FALSE(backtick.succeeded());
  ASSERT_EQ(backtick.tokens().size(), 4U);
  EXPECT_EQ(backtick.tokens()[0].kind, TokenKind::InvalidCharacter);
  EXPECT_EQ(backtick.tokens()[1].kind, TokenKind::Keyword);
  EXPECT_EQ(backtick.tokens()[2].kind, TokenKind::InvalidCharacter);
  ExpectFullFidelity(backtick);

  const LexedFile hash = tokenize("r#func");
  ASSERT_FALSE(hash.succeeded());
  ASSERT_EQ(hash.tokens().size(), 4U);
  EXPECT_EQ(hash.tokens()[0].kind, TokenKind::Identifier);
  EXPECT_EQ(hash.tokens()[1].kind, TokenKind::InvalidCharacter);
  EXPECT_EQ(hash.tokens()[2].kind, TokenKind::Keyword);
  ExpectFullFidelity(hash);

  const LexedFile at_sign = tokenize("@func");
  ASSERT_TRUE(at_sign.succeeded());
  ASSERT_EQ(at_sign.tokens().size(), 3U);
  EXPECT_EQ(at_sign.tokens()[0].kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(at_sign.tokens()[0].payload), '@');
  EXPECT_EQ(at_sign.tokens()[1].kind, TokenKind::Keyword);
  ExpectFullFidelity(at_sign);
}

TEST(IdentifiersTest, UnicodeTablesArePinnedToTheDeclaredLanguageVersion) {
  EXPECT_STREQ(kUnicodeVersion, "15.1.0");

  const std::string unicode16OnlyLetter = "\xE1\xB2\x89";
  const LexedFile lexed = tokenize(unicode16OnlyLetter);
  ASSERT_FALSE(lexed.succeeded());
  EXPECT_FALSE(HasTokenKind(lexed, TokenKind::Identifier));
  EXPECT_FALSE(HasDiagnosticKind(lexed, DiagnosticKind::InvalidUtf8));
  EXPECT_TRUE(HasDiagnosticKind(lexed, DiagnosticKind::InvalidCharacter));
  ExpectFullFidelity(lexed);
}

TEST(IdentifiersTest, MultibyteIdentifierSpanIsMeasuredInUtf8Bytes) {
  const std::string source = Utf8(u8"\u7528\u6237ID");
  const LexedFile lexed = tokenize(source);

  ASSERT_TRUE(lexed.succeeded());
  ASSERT_EQ(lexed.tokens().size(), 2U);
  EXPECT_EQ(source.size(), 8U);
  EXPECT_EQ(lexed.tokens()[0].span.start, 0U);
  EXPECT_EQ(lexed.tokens()[0].span.end, 8U);
  EXPECT_EQ(lexed.raw(lexed.tokens()[0]), source);
  ExpectFullFidelity(lexed);
}

}  // namespace
}  // namespace ink::tokenizer
