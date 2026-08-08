#include "ink/tokenizer/tokenizer.h"

#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <string>
#include <variant>
#include <vector>

namespace ink::tokenizer
{
  namespace
  {
    using core::Diagnostic;
    using core::DiagnosticKind;

    bool hasTokenKind(const LexedFile &Lexed, TokenKind Kind)
    {
      for (const Token &TokenEntry : Lexed.tokens())
      {
        if (TokenEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    bool hasDiagnosticKind(const LexedFile &Lexed, DiagnosticKind Kind)
    {
      for (const Diagnostic &DiagnosticEntry : Lexed.diagnostics())
      {
        if (DiagnosticEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    void expectFullFidelity(const LexedFile &Lexed)
    {
      const std::vector<Token> &Tokens = Lexed.tokens();
      ASSERT_FALSE(Tokens.empty());
      EXPECT_EQ(Tokens.back().Kind, TokenKind::EndOfFile);
      EXPECT_EQ(Tokens.back().Span.Start, Lexed.source().size());
      EXPECT_EQ(Tokens.back().Span.End, Lexed.source().size());
      EXPECT_TRUE(Lexed.raw(Tokens.back()).empty());

      std::size_t NextByte = 0;
      std::size_t EofCount = 0;
      std::string Reconstructed;
      for (const Token &TokenEntry : Tokens)
      {
        EXPECT_LE(TokenEntry.Span.Start, TokenEntry.Span.End);
        EXPECT_LE(TokenEntry.Span.End, Lexed.source().size());
        EXPECT_EQ(TokenEntry.isTrivia(), isTrivia(TokenEntry.Kind));
        EXPECT_EQ(TokenEntry.isError(), isError(TokenEntry.Kind));
        if (TokenEntry.Kind == TokenKind::EndOfFile)
        {
          ++EofCount;
          EXPECT_EQ(&TokenEntry, &Tokens.back());
          continue;
        }
        EXPECT_EQ(TokenEntry.Span.Start, NextByte);
        EXPECT_EQ(Lexed.raw(TokenEntry).size(), TokenEntry.Span.size());
        Reconstructed.append(Lexed.raw(TokenEntry));
        NextByte = TokenEntry.Span.End;
      }
      EXPECT_EQ(EofCount, 1U);
      EXPECT_EQ(NextByte, Lexed.source().size());
      EXPECT_EQ(Reconstructed, Lexed.source());
    }

    // Tests ASCII and Unicode XID_Start and XID_Continue identifier spellings.
    TEST(IdentifiersTest, XidStartAndContinueAcceptAsciiAndUnicodeIdentifiers)
    {
      const std::vector<std::string> Identifiers = {
          "value",
          "_private",
          "_",
          "__internal",
          utf8(u8"\u7528\u6237"),
          utf8(u8"\u7528\u6237ID"),
          utf8(u8"\u0394value"),
          utf8(u8"\u0434\u0430\u043D\u043D\u044B\u0435"),
          utf8(u8"caf\u00E92"),
          utf8(u8"user\u6570\u91CF"),
          utf8(u8"HTTP\u72B6\u6001"),
      };

      for (const std::string &IdentifierText : Identifiers)
      {
        SCOPED_TRACE(IdentifierText);
        const LexedFile Lexed = tokenize(IdentifierText);
        ASSERT_TRUE(Lexed.succeeded());
        ASSERT_EQ(Lexed.tokens().size(), 2U);
        EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::Identifier);
        EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), IdentifierText);
        EXPECT_TRUE(std::holds_alternative<std::monostate>(Lexed.tokens()[0].Payload));
        expectFullFidelity(Lexed);
      }
    }

    // Tests that an underscore followed by digits is an identifier rather than a number.
    TEST(IdentifiersTest, LeadingUnderscoreFollowedByDigitsRemainsAnIdentifier)
    {
      const LexedFile Lexed = tokenize("_100");

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 2U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), "_100");
      expectFullFidelity(Lexed);
    }

    // Tests rejection of a digit-led identifier candidate as an invalid numeric token.
    TEST(IdentifiersTest, AsciiDigitCannotStartAnIdentifierAndTheNumericCandidateIsRejected)
    {
      const LexedFile Lexed = tokenize("2value");

      ASSERT_FALSE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 2U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::InvalidNumber);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), "2value");
      EXPECT_FALSE(hasTokenKind(Lexed, TokenKind::Identifier));
      EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::UnknownNumericSuffix));
      expectFullFidelity(Lexed);
    }

    // Tests that a valid symbol cleanly terminates an identifier.
    TEST(IdentifiersTest, ValidSymbolEndsAnIdentifierWithoutProducingALexicalError)
    {
      const LexedFile Lexed = tokenize("hello-world");

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 4U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), "hello");
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Lexed.tokens()[1].Payload), '-');
      EXPECT_EQ(Lexed.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[2]), "world");
      expectFullFidelity(Lexed);
    }

    // Tests recovery around an invalid identifier-continuation character.
    TEST(IdentifiersTest, InvalidContinuationCharacterIsDiagnosedAndRecoveryKeepsBothIdentifiers)
    {
      const LexedFile Lexed = tokenize("user$name");

      ASSERT_FALSE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 4U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), "user");
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[1]), "$");
      EXPECT_EQ(Lexed.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[2]), "name");
      EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::InvalidCharacter));
      expectFullFidelity(Lexed);
    }

    // Tests that a combining mark cannot begin an identifier.
    TEST(IdentifiersTest, CombiningMarkCannotStartAnIdentifier)
    {
      const std::string Source = utf8(u8"\u0301name");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_FALSE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 3U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), utf8(u8"\u0301"));
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[1]), "name");
      expectFullFidelity(Lexed);
    }

    // Tests emoji rejection without consuming a following valid identifier.
    TEST(IdentifiersTest, EmojiCannotStartAnIdentifierAndDoesNotConsumeFollowingIdentifier)
    {
      const std::string Source = utf8(u8"\U0001F600value");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_FALSE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 3U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), utf8(u8"\U0001F600"));
      EXPECT_EQ(Lexed.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[1]), "value");
      expectFullFidelity(Lexed);
    }

    // Tests acceptance and byte preservation of an NFC-normalized identifier.
    TEST(IdentifiersTest, NfcIdentifierIsAcceptedWithoutChangingItsRawSpelling)
    {
      const std::string Source = utf8(u8"caf\u00E9");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 2U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), Source);
      expectFullFidelity(Lexed);
    }

    // Tests rejection of a canonically equivalent non-NFC identifier candidate.
    TEST(IdentifiersTest, CanonicallyEquivalentButNonNfcIdentifierIsRejectedAsOneCandidate)
    {
      const std::string Source = utf8(u8"cafe\u0301");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_FALSE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 2U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::InvalidIdentifier);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), Source);
      EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::IdentifierNotNfc));
      expectFullFidelity(Lexed);
    }

    // Tests case-sensitive identifier comparison and keyword classification.
    TEST(IdentifiersTest, IdentifierComparisonAndKeywordLookupAreCaseSensitive)
    {
      const std::string Source = "value Value VALUE func Func function functional";
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 14U);
      EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.tokens()[4].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.tokens()[6].Kind, TokenKind::Keyword);
      EXPECT_EQ(std::get<KeywordKind>(Lexed.tokens()[6].Payload), KeywordKind::Func);
      EXPECT_EQ(Lexed.tokens()[8].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.tokens()[10].Kind, TokenKind::Identifier);
      EXPECT_EQ(Lexed.tokens()[12].Kind, TokenKind::Identifier);
      expectFullFidelity(Lexed);
    }

    // Tests invisible-format diagnostics, exact byte spans, and neighboring scalar context.
    TEST(IdentifiersTest, InvisibleFormatCharactersAreRejectedWithExactSourceSpans)
    {
      struct InvisibleCase
      {
        const char *Name;
        std::string Source;
        std::string Invisible;
      };
      const std::vector<InvisibleCase> Cases = {
          {"zero width non joiner", utf8(u8"a\u200Cb"), utf8(u8"\u200C")},
          {"zero width joiner", utf8(u8"a\u200Db"), utf8(u8"\u200D")},
          {"zero width space", utf8(u8"a\u200Bb"), utf8(u8"\u200B")},
          {"bidirectional override", utf8(u8"a\u202Eb"), utf8(u8"\u202E")},
          {"variation selector", utf8(u8"a\uFE0Fb"), utf8(u8"\uFE0F")},
      };

      for (const InvisibleCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const LexedFile Lexed = tokenize(TestCase.Source);
        ASSERT_FALSE(Lexed.succeeded());
        EXPECT_TRUE(hasTokenKind(Lexed, TokenKind::InvalidIdentifier) || hasTokenKind(Lexed, TokenKind::InvalidCharacter));
        ASSERT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::InvisibleCharacter));
        const std::size_t ExpectedStart = TestCase.Source.find(TestCase.Invisible);
        bool FoundExactSpan = false;
        bool FoundDetailedMessage = false;
        for (const Diagnostic &DiagnosticEntry : Lexed.diagnostics())
        {
          if (DiagnosticEntry.Kind == DiagnosticKind::InvisibleCharacter && DiagnosticEntry.Span.Start == ExpectedStart && DiagnosticEntry.Span.End == ExpectedStart + TestCase.Invisible.size())
          {
            FoundExactSpan = true;
            FoundDetailedMessage = DiagnosticEntry.Message.find("U+") != std::string::npos && DiagnosticEntry.Message.find("U+0061 ('a')") != std::string::npos && DiagnosticEntry.Message.find("U+0062 ('b')") != std::string::npos;
          }
        }
        EXPECT_TRUE(FoundExactSpan);
        EXPECT_TRUE(FoundDetailedMessage);
        expectFullFidelity(Lexed);
      }

      const LexedFile Standalone = tokenize(utf8(u8"\u00AD"));
      ASSERT_FALSE(Standalone.succeeded());
      ASSERT_EQ(Standalone.diagnostics().size(), 1U);
      EXPECT_NE(Standalone.diagnostics().front().Message.find("U+00AD"), std::string::npos);
      EXPECT_NE(Standalone.diagnostics().front().Message.find("source text"), std::string::npos);
      EXPECT_EQ(Standalone.diagnostics().front().Message.find("identifier"), std::string::npos);

      const LexedFile AfterSpace = tokenize(utf8(u8" \u00ADa"));
      ASSERT_FALSE(AfterSpace.succeeded());
      for (const Diagnostic &DiagnosticEntry : AfterSpace.diagnostics())
      {
        if (DiagnosticEntry.Kind == DiagnosticKind::InvisibleCharacter)
        {
          EXPECT_EQ(DiagnosticEntry.Message.find("U+0020"), std::string::npos);
        }
      }
    }

    // Tests lexical acceptance of identifiers that mix supported writing systems.
    TEST(IdentifiersTest, MixedWritingSystemsAreLexicallyValid)
    {
      const std::vector<std::string> Cases = {
          utf8(u8"user\u6570\u91CF"),
          utf8(u8"HTTP\u72B6\u6001"),
      };

      for (const std::string &Source : Cases)
      {
        SCOPED_TRACE(Source);
        const LexedFile Lexed = tokenize(Source);
        ASSERT_TRUE(Lexed.succeeded());
        ASSERT_EQ(Lexed.tokens().size(), 2U);
        EXPECT_EQ(Lexed.tokens()[0].Kind, TokenKind::Identifier);
        EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), Source);
        expectFullFidelity(Lexed);
      }
    }

    // Tests that punctuation spellings cannot escape hard-keyword classification.
    TEST(IdentifiersTest, BacktickHashAndAtSignDoNotEscapeHardKeywords)
    {
      const LexedFile Backtick = tokenize("`func`");
      ASSERT_FALSE(Backtick.succeeded());
      ASSERT_EQ(Backtick.tokens().size(), 4U);
      EXPECT_EQ(Backtick.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Backtick.tokens()[1].Kind, TokenKind::Keyword);
      EXPECT_EQ(Backtick.tokens()[2].Kind, TokenKind::InvalidCharacter);
      expectFullFidelity(Backtick);

      const LexedFile Hash = tokenize("r#func");
      ASSERT_FALSE(Hash.succeeded());
      ASSERT_EQ(Hash.tokens().size(), 4U);
      EXPECT_EQ(Hash.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Hash.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Hash.tokens()[2].Kind, TokenKind::Keyword);
      expectFullFidelity(Hash);

      const LexedFile AtSign = tokenize("@func");
      ASSERT_TRUE(AtSign.succeeded());
      ASSERT_EQ(AtSign.tokens().size(), 3U);
      EXPECT_EQ(AtSign.tokens()[0].Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(AtSign.tokens()[0].Payload), '@');
      EXPECT_EQ(AtSign.tokens()[1].Kind, TokenKind::Keyword);
      expectFullFidelity(AtSign);
    }

    // Tests the declared Unicode version and exclusion of newer identifier characters.
    TEST(IdentifiersTest, UnicodeTablesArePinnedToTheDeclaredLanguageVersion)
    {
      EXPECT_STREQ(UnicodeVersion, "15.1.0");

      const std::string Unicode16OnlyLetter = "\xE1\xB2\x89";
      const LexedFile Lexed = tokenize(Unicode16OnlyLetter);
      ASSERT_FALSE(Lexed.succeeded());
      EXPECT_FALSE(hasTokenKind(Lexed, TokenKind::Identifier));
      EXPECT_FALSE(hasDiagnosticKind(Lexed, DiagnosticKind::InvalidUtf8));
      EXPECT_TRUE(hasDiagnosticKind(Lexed, DiagnosticKind::InvalidCharacter));
      expectFullFidelity(Lexed);
    }

    // Tests that multibyte identifier spans are measured in original UTF-8 bytes.
    TEST(IdentifiersTest, MultibyteIdentifierSpanIsMeasuredInUtf8Bytes)
    {
      const std::string Source = utf8(u8"\u7528\u6237ID");
      const LexedFile Lexed = tokenize(Source);

      ASSERT_TRUE(Lexed.succeeded());
      ASSERT_EQ(Lexed.tokens().size(), 2U);
      EXPECT_EQ(Source.size(), 8U);
      EXPECT_EQ(Lexed.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Lexed.tokens()[0].Span.End, 8U);
      EXPECT_EQ(Lexed.raw(Lexed.tokens()[0]), Source);
      expectFullFidelity(Lexed);
    }
  } // namespace
} // namespace ink::tokenizer
