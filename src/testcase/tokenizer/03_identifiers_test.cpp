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

    bool hasTokenKind(const TokenizedBuffer &Buffer, TokenKind Kind)
    {
      for (const Token &TokenEntry : Buffer.tokens())
      {
        if (TokenEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    bool hasDiagnosticKind(const TokenizedBuffer &Buffer, DiagnosticKind Kind)
    {
      for (const Diagnostic &DiagnosticEntry : Buffer.diagnostics())
      {
        if (DiagnosticEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    void expectFullFidelity(const TokenizedBuffer &Buffer)
    {
      const std::vector<Token> &Tokens = Buffer.tokens();
      ASSERT_FALSE(Tokens.empty());
      EXPECT_EQ(Tokens.back().Kind, TokenKind::EndOfFile);
      EXPECT_EQ(Tokens.back().Span.Start, Buffer.source().size());
      EXPECT_EQ(Tokens.back().Span.End, Buffer.source().size());
      EXPECT_TRUE(Buffer.raw(Tokens.back()).empty());

      std::size_t NextByte = 0;
      std::size_t EofCount = 0;
      std::string Reconstructed;
      for (const Token &TokenEntry : Tokens)
      {
        EXPECT_LE(TokenEntry.Span.Start, TokenEntry.Span.End);
        EXPECT_LE(TokenEntry.Span.End, Buffer.source().size());
        EXPECT_EQ(TokenEntry.isTrivia(), isTrivia(TokenEntry.Kind));
        EXPECT_EQ(TokenEntry.isError(), isError(TokenEntry.Kind));
        if (TokenEntry.Kind == TokenKind::EndOfFile)
        {
          ++EofCount;
          EXPECT_EQ(&TokenEntry, &Tokens.back());
          continue;
        }
        EXPECT_EQ(TokenEntry.Span.Start, NextByte);
        EXPECT_EQ(Buffer.raw(TokenEntry).size(), TokenEntry.Span.size());
        Reconstructed.append(Buffer.raw(TokenEntry));
        NextByte = TokenEntry.Span.End;
      }
      EXPECT_EQ(EofCount, 1U);
      EXPECT_EQ(NextByte, Buffer.source().size());
      EXPECT_EQ(Reconstructed, Buffer.source());
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
        const TokenizedBuffer Buffer = tokenize(IdentifierText);
        ASSERT_TRUE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 2U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), IdentifierText);
        EXPECT_TRUE(std::holds_alternative<std::monostate>(Buffer.tokens()[0].Payload));
        expectFullFidelity(Buffer);
      }
    }

    // Tests that an underscore followed by digits is an identifier rather than a number.
    TEST(IdentifiersTest, LeadingUnderscoreFollowedByDigitsRemainsAnIdentifier)
    {
      const TokenizedBuffer Buffer = tokenize("_100");

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "_100");
      expectFullFidelity(Buffer);
    }

    // Tests rejection of a digit-led identifier candidate as an invalid numeric token.
    TEST(IdentifiersTest, AsciiDigitCannotStartAnIdentifierAndTheNumericCandidateIsRejected)
    {
      const TokenizedBuffer Buffer = tokenize("2value");

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidNumber);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "2value");
      EXPECT_FALSE(hasTokenKind(Buffer, TokenKind::Identifier));
      EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::UnknownNumericSuffix));
      expectFullFidelity(Buffer);
    }

    // Tests that a valid symbol cleanly terminates an identifier.
    TEST(IdentifiersTest, ValidSymbolEndsAnIdentifierWithoutProducingALexicalError)
    {
      const TokenizedBuffer Buffer = tokenize("hello-world");

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 4U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "hello");
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Buffer.tokens()[1].Payload), '-');
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[2]), "world");
      expectFullFidelity(Buffer);
    }

    // Tests recovery around an invalid identifier-continuation character.
    TEST(IdentifiersTest, InvalidContinuationCharacterIsDiagnosedAndRecoveryKeepsBothIdentifiers)
    {
      const TokenizedBuffer Buffer = tokenize("user$name");

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 4U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "user");
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "$");
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[2]), "name");
      EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::InvalidCharacter));
      expectFullFidelity(Buffer);
    }

    // Tests that a combining mark cannot begin an identifier.
    TEST(IdentifiersTest, CombiningMarkCannotStartAnIdentifier)
    {
      const std::string Source = utf8(u8"\u0301name");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), utf8(u8"\u0301"));
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "name");
      expectFullFidelity(Buffer);
    }

    // Tests emoji rejection without consuming a following valid identifier.
    TEST(IdentifiersTest, EmojiCannotStartAnIdentifierAndDoesNotConsumeFollowingIdentifier)
    {
      const std::string Source = utf8(u8"\U0001F600value");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 3U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), utf8(u8"\U0001F600"));
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "value");
      expectFullFidelity(Buffer);
    }

    // Tests acceptance and byte preservation of an NFC-normalized identifier.
    TEST(IdentifiersTest, NfcIdentifierIsAcceptedWithoutChangingItsRawSpelling)
    {
      const std::string Source = utf8(u8"caf\u00E9");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
      expectFullFidelity(Buffer);
    }

    // Tests rejection of a canonically equivalent non-NFC identifier candidate.
    TEST(IdentifiersTest, CanonicallyEquivalentButNonNfcIdentifierIsRejectedAsOneCandidate)
    {
      const std::string Source = utf8(u8"cafe\u0301");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidIdentifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
      EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::IdentifierNotNfc));
      expectFullFidelity(Buffer);
    }

    // Tests case-sensitive identifier comparison and keyword classification.
    TEST(IdentifiersTest, IdentifierComparisonAndKeywordLookupAreCaseSensitive)
    {
      const std::string Source = "value Value VALUE func Func function functional";
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 14U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[4].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[6].Kind, TokenKind::Keyword);
      EXPECT_EQ(std::get<KeywordKind>(Buffer.tokens()[6].Payload), KeywordKind::Func);
      EXPECT_EQ(Buffer.tokens()[8].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[10].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[12].Kind, TokenKind::Identifier);
      expectFullFidelity(Buffer);
    }

    // Tests invisible-format diagnostics, exact byte spans, and neighboring scalar context.
    TEST(IdentifiersTest, InvisibleFormatCharactersAreRejectedWithExactSourceSpans)
    {
      struct InvisibleCase
      {
        const char *Name;
        std::string Source;
        std::string Invisible;
        TokenKind ExpectedErrorKind;
      };
      const std::vector<InvisibleCase> Cases = {
          {"zero width non joiner", utf8(u8"a\u200Cb"), utf8(u8"\u200C"), TokenKind::InvalidIdentifier},
          {"zero width joiner", utf8(u8"a\u200Db"), utf8(u8"\u200D"), TokenKind::InvalidIdentifier},
          {"zero width space", utf8(u8"a\u200Bb"), utf8(u8"\u200B"), TokenKind::InvalidCharacter},
          {"bidirectional override", utf8(u8"a\u202Eb"), utf8(u8"\u202E"), TokenKind::InvalidCharacter},
          {"variation selector", utf8(u8"a\uFE0Fb"), utf8(u8"\uFE0F"), TokenKind::InvalidIdentifier},
      };

      for (const InvisibleCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const TokenizedBuffer Buffer = tokenize(TestCase.Source);
        ASSERT_FALSE(Buffer.succeeded());
        EXPECT_TRUE(hasTokenKind(Buffer, TestCase.ExpectedErrorKind));
        ASSERT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::InvisibleCharacter));
        const std::size_t ExpectedStart = TestCase.Source.find(TestCase.Invisible);
        bool FoundExactSpan = false;
        bool FoundDetailedMessage = false;
        for (const Diagnostic &DiagnosticEntry : Buffer.diagnostics())
        {
          if (DiagnosticEntry.Kind == DiagnosticKind::InvisibleCharacter && DiagnosticEntry.Span.Start == ExpectedStart && DiagnosticEntry.Span.End == ExpectedStart + TestCase.Invisible.size())
          {
            FoundExactSpan = true;
            FoundDetailedMessage = DiagnosticEntry.Message.find("U+") != std::string::npos && DiagnosticEntry.Message.find("U+0061 ('a')") != std::string::npos && DiagnosticEntry.Message.find("U+0062 ('b')") != std::string::npos;
          }
        }
        EXPECT_TRUE(FoundExactSpan);
        EXPECT_TRUE(FoundDetailedMessage);
        expectFullFidelity(Buffer);
      }

      const TokenizedBuffer Standalone = tokenize(utf8(u8"\u00AD"));
      ASSERT_FALSE(Standalone.succeeded());
      ASSERT_EQ(Standalone.diagnostics().size(), 1U);
      EXPECT_NE(Standalone.diagnostics().front().Message.find("U+00AD"), std::string::npos);
      EXPECT_NE(Standalone.diagnostics().front().Message.find("source text"), std::string::npos);
      EXPECT_EQ(Standalone.diagnostics().front().Message.find("identifier"), std::string::npos);
      expectFullFidelity(Standalone);

      const TokenizedBuffer AfterSpace = tokenize(utf8(u8" \u00ADa"));
      ASSERT_FALSE(AfterSpace.succeeded());
      ASSERT_EQ(AfterSpace.diagnostics().size(), 1U);
      EXPECT_EQ(AfterSpace.diagnostics()[0].Kind, DiagnosticKind::InvisibleCharacter);
      EXPECT_EQ(AfterSpace.diagnostics()[0].Span.Start, 1U);
      EXPECT_EQ(AfterSpace.diagnostics()[0].Span.End, 3U);
      EXPECT_EQ(AfterSpace.diagnostics()[0].Message, "invisible format character U+00AD appears before U+0061 ('a')");
      expectFullFidelity(AfterSpace);

      const TokenizedBuffer Trailing = tokenize(utf8(u8"a\u200C"));
      ASSERT_FALSE(Trailing.succeeded());
      ASSERT_EQ(Trailing.tokens().size(), 2U);
      EXPECT_EQ(Trailing.tokens()[0].Kind, TokenKind::InvalidIdentifier);
      ASSERT_EQ(Trailing.diagnostics().size(), 1U);
      EXPECT_EQ(Trailing.diagnostics()[0].Kind, DiagnosticKind::InvisibleCharacter);
      EXPECT_EQ(Trailing.diagnostics()[0].Span.Start, 1U);
      EXPECT_EQ(Trailing.diagnostics()[0].Span.End, 4U);
      EXPECT_EQ(Trailing.diagnostics()[0].Message, "invisible format character U+200C appears after U+0061 ('a')");
      expectFullFidelity(Trailing);

      const TokenizedBuffer Consecutive = tokenize(utf8(u8"a\u200C\u200Db"));
      ASSERT_FALSE(Consecutive.succeeded());
      ASSERT_EQ(Consecutive.tokens().size(), 2U);
      EXPECT_EQ(Consecutive.tokens()[0].Kind, TokenKind::InvalidIdentifier);
      ASSERT_EQ(Consecutive.diagnostics().size(), 2U);
      EXPECT_EQ(Consecutive.diagnostics()[0].Kind, DiagnosticKind::InvisibleCharacter);
      EXPECT_EQ(Consecutive.diagnostics()[0].Span.Start, 1U);
      EXPECT_EQ(Consecutive.diagnostics()[0].Span.End, 4U);
      EXPECT_EQ(Consecutive.diagnostics()[0].Message, "invisible format character U+200C appears between U+0061 ('a') and U+0062 ('b')");
      EXPECT_EQ(Consecutive.diagnostics()[1].Kind, DiagnosticKind::InvisibleCharacter);
      EXPECT_EQ(Consecutive.diagnostics()[1].Span.Start, 4U);
      EXPECT_EQ(Consecutive.diagnostics()[1].Span.End, 7U);
      EXPECT_EQ(Consecutive.diagnostics()[1].Message, "invisible format character U+200D appears between U+0061 ('a') and U+0062 ('b')");
      expectFullFidelity(Consecutive);
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
        const TokenizedBuffer Buffer = tokenize(Source);
        ASSERT_TRUE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 2U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
        expectFullFidelity(Buffer);
      }
    }

    // Tests that punctuation spellings cannot escape hard-keyword classification.
    TEST(IdentifiersTest, BacktickHashAndAtSignDoNotEscapeHardKeywords)
    {
      const TokenizedBuffer Backtick = tokenize("`func`");
      ASSERT_FALSE(Backtick.succeeded());
      ASSERT_EQ(Backtick.tokens().size(), 4U);
      EXPECT_EQ(Backtick.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Backtick.tokens()[1].Kind, TokenKind::Keyword);
      EXPECT_EQ(Backtick.tokens()[2].Kind, TokenKind::InvalidCharacter);
      expectFullFidelity(Backtick);

      const TokenizedBuffer Hash = tokenize("r#func");
      ASSERT_FALSE(Hash.succeeded());
      ASSERT_EQ(Hash.tokens().size(), 4U);
      EXPECT_EQ(Hash.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Hash.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Hash.tokens()[2].Kind, TokenKind::Keyword);
      expectFullFidelity(Hash);

      const TokenizedBuffer AtSign = tokenize("@func");
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
      const TokenizedBuffer Buffer = tokenize(Unicode16OnlyLetter);
      ASSERT_FALSE(Buffer.succeeded());
      EXPECT_FALSE(hasTokenKind(Buffer, TokenKind::Identifier));
      EXPECT_FALSE(hasDiagnosticKind(Buffer, DiagnosticKind::InvalidUtf8));
      EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::InvalidCharacter));
      expectFullFidelity(Buffer);
    }

    // Tests that multibyte identifier spans are measured in original UTF-8 bytes.
    TEST(IdentifiersTest, MultibyteIdentifierSpanIsMeasuredInUtf8Bytes)
    {
      const std::string Source = utf8(u8"\u7528\u6237ID");
      const TokenizedBuffer Buffer = tokenize(Source);

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Source.size(), 8U);
      EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Buffer.tokens()[0].Span.End, 8U);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
      expectFullFidelity(Buffer);
    }
  } // namespace
} // namespace ink::tokenizer
