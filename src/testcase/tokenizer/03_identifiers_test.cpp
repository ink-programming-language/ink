#include "ink/tokenizer/tokenizer.h"
#include "tokenizer_test_support.h"

#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <variant>
#include <vector>

namespace ink::tokenizer
{
  namespace
  {
    using core::Diagnostic;
    using core::DiagnosticArgument;
    using core::DiagnosticArgumentName;
    using core::DiagnosticKind;
    using core::DiagnosticRelatedKind;
    using core::DiagnosticSourceContext;
    using core::SourceRange;

    struct RelatedCharacterExpectation
    {
      DiagnosticRelatedKind Kind;
      SourceRange Span;
      char32_t Character;
    };

    template <typename ValueType>
    const ValueType *findArgumentValue(const std::vector<DiagnosticArgument> &Arguments, DiagnosticArgumentName Name)
    {
      for (const DiagnosticArgument &Argument : Arguments)
      {
        if (Argument.Name == Name)
        {
          return std::get_if<ValueType>(&Argument.Value);
        }
      }
      return nullptr;
    }

    void expectInvisibleDiagnostic(const Diagnostic &DiagnosticEntry, SourceRange Span, char32_t Character, DiagnosticSourceContext Context, const std::vector<RelatedCharacterExpectation> &ExpectedRelated)
    {
      EXPECT_EQ(DiagnosticEntry.Kind, DiagnosticKind::InvisibleCharacter);
      EXPECT_EQ(DiagnosticEntry.Span, Span);
      ASSERT_EQ(DiagnosticEntry.Arguments.size(), 2U);
      const char32_t *ActualCharacter = findArgumentValue<char32_t>(DiagnosticEntry.Arguments, DiagnosticArgumentName::Character);
      ASSERT_NE(ActualCharacter, nullptr);
      EXPECT_EQ(*ActualCharacter, Character);
      const DiagnosticSourceContext *ActualContext = findArgumentValue<DiagnosticSourceContext>(DiagnosticEntry.Arguments, DiagnosticArgumentName::Context);
      ASSERT_NE(ActualContext, nullptr);
      EXPECT_EQ(*ActualContext, Context);
      ASSERT_EQ(DiagnosticEntry.Related.size(), ExpectedRelated.size());
      for (const RelatedCharacterExpectation &Expected : ExpectedRelated)
      {
        const auto Related = std::find_if(DiagnosticEntry.Related.begin(), DiagnosticEntry.Related.end(), [&Expected](const core::DiagnosticRelatedInformation &Information)
                                          {
                                            return Information.Kind == Expected.Kind;
                                          });
        ASSERT_NE(Related, DiagnosticEntry.Related.end());
        EXPECT_EQ(Related->Span, Expected.Span);
        ASSERT_EQ(Related->Arguments.size(), 1U);
        const char32_t *RelatedCharacter = findArgumentValue<char32_t>(Related->Arguments, DiagnosticArgumentName::Character);
        ASSERT_NE(RelatedCharacter, nullptr);
        EXPECT_EQ(*RelatedCharacter, Expected.Character);
      }
    }

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
        const TokenizedBuffer Buffer = tokenize(TestSourceFileId, IdentifierText);
        ASSERT_TRUE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 2U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), IdentifierText);
        EXPECT_TRUE(std::holds_alternative<std::monostate>(Buffer.tokens()[0].Payload));
        expectFullFidelity(Buffer);
      }
    }

    // Tests both ends of the Unicode 15.1 CJK Extension I XID_Start range and the adjacent excluded scalars.
    TEST(IdentifiersTest, Unicode151XidStartRangeHasExactBoundaryBehavior)
    {
      const std::vector<std::string> Accepted = {
          utf8(u8"\U0002EBF0"),
          utf8(u8"\U0002EE5D"),
      };
      for (const std::string &Source : Accepted)
      {
        SCOPED_TRACE(Source);
        const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);
        ASSERT_TRUE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 2U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
        EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(Buffer.tokens()[0].Span.End, Source.size());
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
        EXPECT_TRUE(std::holds_alternative<std::monostate>(Buffer.tokens()[0].Payload));
        EXPECT_TRUE(Buffer.diagnostics().empty());
        expectFullFidelity(Buffer);
      }

      const std::vector<std::string> Rejected = {
          utf8(u8"\U0002EBEF"),
          utf8(u8"\U0002EE5E"),
      };
      for (const std::string &Source : Rejected)
      {
        SCOPED_TRACE(Source);
        const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);
        ASSERT_FALSE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 2U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidCharacter);
        EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(Buffer.tokens()[0].Span.End, Source.size());
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
        ASSERT_EQ(Buffer.diagnostics().size(), 1U);
        EXPECT_EQ(Buffer.diagnostics()[0].Kind, DiagnosticKind::InvalidCharacter);
        EXPECT_EQ(Buffer.diagnostics()[0].Span.Start, 0U);
        EXPECT_EQ(Buffer.diagnostics()[0].Span.End, Source.size());
        expectFullFidelity(Buffer);
      }
    }

    // Tests that representative non-ASCII XID_Continue-only scalars join an identifier but cannot start one.
    TEST(IdentifiersTest, NonAsciiXidContinueOnlyScalarsRequireAnIdentifierStarter)
    {
      struct ContinueCase
      {
        const char *Name;
        std::string Scalar;
      };
      const std::vector<ContinueCase> Cases = {
          {"middle dot", utf8(u8"\u00B7")},
          {"Arabic-Indic digit zero", utf8(u8"\u0660")},
          {"combining acute accent", utf8(u8"\u0301")},
      };

      for (const ContinueCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const std::string ContinuedSource = "q" + TestCase.Scalar + "z";
        const TokenizedBuffer Continued = tokenize(TestSourceFileId, ContinuedSource);
        ASSERT_TRUE(Continued.succeeded());
        ASSERT_EQ(Continued.tokens().size(), 2U);
        EXPECT_EQ(Continued.tokens()[0].Kind, TokenKind::Identifier);
        EXPECT_EQ(Continued.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(Continued.tokens()[0].Span.End, ContinuedSource.size());
        EXPECT_EQ(Continued.raw(Continued.tokens()[0]), ContinuedSource);
        EXPECT_TRUE(Continued.diagnostics().empty());
        expectFullFidelity(Continued);

        const std::string InitialSource = TestCase.Scalar + "q";
        const TokenizedBuffer Initial = tokenize(TestSourceFileId, InitialSource);
        ASSERT_FALSE(Initial.succeeded());
        ASSERT_EQ(Initial.tokens().size(), 3U);
        EXPECT_EQ(Initial.tokens()[0].Kind, TokenKind::InvalidCharacter);
        EXPECT_EQ(Initial.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(Initial.tokens()[0].Span.End, TestCase.Scalar.size());
        EXPECT_EQ(Initial.raw(Initial.tokens()[0]), TestCase.Scalar);
        EXPECT_EQ(Initial.tokens()[1].Kind, TokenKind::Identifier);
        EXPECT_EQ(Initial.tokens()[1].Span.Start, TestCase.Scalar.size());
        EXPECT_EQ(Initial.tokens()[1].Span.End, InitialSource.size());
        EXPECT_EQ(Initial.raw(Initial.tokens()[1]), "q");
        ASSERT_EQ(Initial.diagnostics().size(), 1U);
        EXPECT_EQ(Initial.diagnostics()[0].Kind, DiagnosticKind::InvalidCharacter);
        EXPECT_EQ(Initial.diagnostics()[0].Span.Start, 0U);
        EXPECT_EQ(Initial.diagnostics()[0].Span.End, TestCase.Scalar.size());
        expectFullFidelity(Initial);
      }
    }

    // Tests that an underscore followed by digits is an identifier rather than a number.
    TEST(IdentifiersTest, LeadingUnderscoreFollowedByDigitsRemainsAnIdentifier)
    {
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, "_100");

      ASSERT_TRUE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "_100");
      expectFullFidelity(Buffer);
    }

    // Tests rejection of a digit-led identifier candidate as an invalid numeric token.
    TEST(IdentifiersTest, AsciiDigitCannotStartAnIdentifierAndTheNumericCandidateIsRejected)
    {
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, "2value");

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
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, "hello-world");

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
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, "user$name");

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
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

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
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

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
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

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
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 2U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidIdentifier);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), Source);
      EXPECT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::IdentifierNotNfc));
      expectFullFidelity(Buffer);
    }

    // Tests NFC canonical-order validation and Hangul composition without applying compatibility normalization.
    TEST(IdentifiersTest, NfcValidationCoversCanonicalOrderingAndHangulComposition)
    {
      const std::string CanonicallyOrdered = utf8(u8"q\u0327\u0301");
      const TokenizedBuffer Ordered = tokenize(TestSourceFileId, CanonicallyOrdered);
      ASSERT_TRUE(Ordered.succeeded());
      ASSERT_EQ(Ordered.tokens().size(), 2U);
      EXPECT_EQ(Ordered.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Ordered.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Ordered.tokens()[0].Span.End, CanonicallyOrdered.size());
      EXPECT_EQ(Ordered.raw(Ordered.tokens()[0]), CanonicallyOrdered);
      EXPECT_TRUE(Ordered.diagnostics().empty());
      expectFullFidelity(Ordered);

      const std::string OutOfCanonicalOrder = utf8(u8"q\u0301\u0327");
      const TokenizedBuffer Reordered = tokenize(TestSourceFileId, OutOfCanonicalOrder);
      ASSERT_FALSE(Reordered.succeeded());
      ASSERT_EQ(Reordered.tokens().size(), 2U);
      EXPECT_EQ(Reordered.tokens()[0].Kind, TokenKind::InvalidIdentifier);
      EXPECT_EQ(Reordered.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Reordered.tokens()[0].Span.End, OutOfCanonicalOrder.size());
      EXPECT_EQ(Reordered.raw(Reordered.tokens()[0]), OutOfCanonicalOrder);
      ASSERT_EQ(Reordered.diagnostics().size(), 1U);
      EXPECT_EQ(Reordered.diagnostics()[0].Kind, DiagnosticKind::IdentifierNotNfc);
      EXPECT_EQ(Reordered.diagnostics()[0].Span.Start, 0U);
      EXPECT_EQ(Reordered.diagnostics()[0].Span.End, OutOfCanonicalOrder.size());
      expectFullFidelity(Reordered);

      const std::string ComposedHangul = utf8(u8"\uAC00");
      const TokenizedBuffer Composed = tokenize(TestSourceFileId, ComposedHangul);
      ASSERT_TRUE(Composed.succeeded());
      ASSERT_EQ(Composed.tokens().size(), 2U);
      EXPECT_EQ(Composed.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Composed.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Composed.tokens()[0].Span.End, ComposedHangul.size());
      EXPECT_EQ(Composed.raw(Composed.tokens()[0]), ComposedHangul);
      EXPECT_TRUE(Composed.diagnostics().empty());
      expectFullFidelity(Composed);

      const std::string DecomposedHangul = utf8(u8"\u1100\u1161");
      const TokenizedBuffer Decomposed = tokenize(TestSourceFileId, DecomposedHangul);
      ASSERT_FALSE(Decomposed.succeeded());
      ASSERT_EQ(Decomposed.tokens().size(), 2U);
      EXPECT_EQ(Decomposed.tokens()[0].Kind, TokenKind::InvalidIdentifier);
      EXPECT_EQ(Decomposed.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Decomposed.tokens()[0].Span.End, DecomposedHangul.size());
      EXPECT_EQ(Decomposed.raw(Decomposed.tokens()[0]), DecomposedHangul);
      ASSERT_EQ(Decomposed.diagnostics().size(), 1U);
      EXPECT_EQ(Decomposed.diagnostics()[0].Kind, DiagnosticKind::IdentifierNotNfc);
      EXPECT_EQ(Decomposed.diagnostics()[0].Span.Start, 0U);
      EXPECT_EQ(Decomposed.diagnostics()[0].Span.End, DecomposedHangul.size());
      expectFullFidelity(Decomposed);
    }

    // Tests case-sensitive identifier comparison and keyword classification.
    TEST(IdentifiersTest, IdentifierComparisonAndKeywordLookupAreCaseSensitive)
    {
      const std::string Source = "value Value VALUE func Func function functional";
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

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
        char32_t ExpectedCharacter;
        DiagnosticSourceContext ExpectedContext;
        TokenKind ExpectedErrorKind;
      };
      const std::vector<InvisibleCase> Cases = {
          {"zero width non joiner", utf8(u8"a\u200Cb"), utf8(u8"\u200C"), U'\u200C', DiagnosticSourceContext::Identifier, TokenKind::InvalidIdentifier},
          {"zero width joiner", utf8(u8"a\u200Db"), utf8(u8"\u200D"), U'\u200D', DiagnosticSourceContext::Identifier, TokenKind::InvalidIdentifier},
          {"zero width space", utf8(u8"a\u200Bb"), utf8(u8"\u200B"), U'\u200B', DiagnosticSourceContext::SourceText, TokenKind::InvalidCharacter},
          {"bidirectional override", utf8(u8"a\u202Eb"), utf8(u8"\u202E"), U'\u202E', DiagnosticSourceContext::SourceText, TokenKind::InvalidCharacter},
          {"variation selector", utf8(u8"a\uFE0Fb"), utf8(u8"\uFE0F"), U'\uFE0F', DiagnosticSourceContext::Identifier, TokenKind::InvalidIdentifier},
      };

      for (const InvisibleCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const TokenizedBuffer Buffer = tokenize(TestSourceFileId, TestCase.Source);
        ASSERT_FALSE(Buffer.succeeded());
        EXPECT_TRUE(hasTokenKind(Buffer, TestCase.ExpectedErrorKind));
        ASSERT_TRUE(hasDiagnosticKind(Buffer, DiagnosticKind::InvisibleCharacter));
        const std::size_t ExpectedStart = TestCase.Source.find(TestCase.Invisible);
        ASSERT_NE(ExpectedStart, std::string::npos);
        ASSERT_EQ(Buffer.diagnostics().size(), 1U);
        const std::vector<RelatedCharacterExpectation> ExpectedRelated = {
            {DiagnosticRelatedKind::PreviousVisibleCharacter, {0, 1}, U'a'},
            {DiagnosticRelatedKind::NextVisibleCharacter, {ExpectedStart + TestCase.Invisible.size(), ExpectedStart + TestCase.Invisible.size() + 1}, U'b'},
        };
        expectInvisibleDiagnostic(Buffer.diagnostics().front(), {ExpectedStart, ExpectedStart + TestCase.Invisible.size()}, TestCase.ExpectedCharacter, TestCase.ExpectedContext, ExpectedRelated);
        expectFullFidelity(Buffer);
      }

      const std::string StandaloneSource = utf8(u8"\u00AD");
      const TokenizedBuffer Standalone = tokenize(TestSourceFileId, StandaloneSource);
      ASSERT_FALSE(Standalone.succeeded());
      ASSERT_EQ(Standalone.diagnostics().size(), 1U);
      expectInvisibleDiagnostic(Standalone.diagnostics().front(), {0, StandaloneSource.size()}, U'\u00AD', DiagnosticSourceContext::SourceText, {});
      expectFullFidelity(Standalone);

      const TokenizedBuffer AfterSpace = tokenize(TestSourceFileId, utf8(u8" \u00ADa"));
      ASSERT_FALSE(AfterSpace.succeeded());
      ASSERT_EQ(AfterSpace.diagnostics().size(), 1U);
      EXPECT_EQ(AfterSpace.diagnostics()[0].Kind, DiagnosticKind::InvisibleCharacter);
      EXPECT_EQ(AfterSpace.diagnostics()[0].Span.Start, 1U);
      EXPECT_EQ(AfterSpace.diagnostics()[0].Span.End, 3U);
      const std::vector<RelatedCharacterExpectation> AfterSpaceRelated = {
          {DiagnosticRelatedKind::NextVisibleCharacter, {3, 4}, U'a'},
      };
      expectInvisibleDiagnostic(AfterSpace.diagnostics()[0], {1, 3}, U'\u00AD', DiagnosticSourceContext::SourceText, AfterSpaceRelated);
      expectFullFidelity(AfterSpace);

      const TokenizedBuffer Trailing = tokenize(TestSourceFileId, utf8(u8"a\u200C"));
      ASSERT_FALSE(Trailing.succeeded());
      ASSERT_EQ(Trailing.tokens().size(), 2U);
      EXPECT_EQ(Trailing.tokens()[0].Kind, TokenKind::InvalidIdentifier);
      ASSERT_EQ(Trailing.diagnostics().size(), 1U);
      EXPECT_EQ(Trailing.diagnostics()[0].Kind, DiagnosticKind::InvisibleCharacter);
      EXPECT_EQ(Trailing.diagnostics()[0].Span.Start, 1U);
      EXPECT_EQ(Trailing.diagnostics()[0].Span.End, 4U);
      const std::vector<RelatedCharacterExpectation> TrailingRelated = {
          {DiagnosticRelatedKind::PreviousVisibleCharacter, {0, 1}, U'a'},
      };
      expectInvisibleDiagnostic(Trailing.diagnostics()[0], {1, 4}, U'\u200C', DiagnosticSourceContext::Identifier, TrailingRelated);
      expectFullFidelity(Trailing);

      const TokenizedBuffer Consecutive = tokenize(TestSourceFileId, utf8(u8"a\u200C\u200Db"));
      ASSERT_FALSE(Consecutive.succeeded());
      ASSERT_EQ(Consecutive.tokens().size(), 2U);
      EXPECT_EQ(Consecutive.tokens()[0].Kind, TokenKind::InvalidIdentifier);
      ASSERT_EQ(Consecutive.diagnostics().size(), 2U);
      EXPECT_EQ(Consecutive.diagnostics()[0].Kind, DiagnosticKind::InvisibleCharacter);
      EXPECT_EQ(Consecutive.diagnostics()[0].Span.Start, 1U);
      EXPECT_EQ(Consecutive.diagnostics()[0].Span.End, 4U);
      EXPECT_EQ(Consecutive.diagnostics()[1].Kind, DiagnosticKind::InvisibleCharacter);
      EXPECT_EQ(Consecutive.diagnostics()[1].Span.Start, 4U);
      EXPECT_EQ(Consecutive.diagnostics()[1].Span.End, 7U);
      const std::vector<RelatedCharacterExpectation> ConsecutiveRelated = {
          {DiagnosticRelatedKind::PreviousVisibleCharacter, {0, 1}, U'a'},
          {DiagnosticRelatedKind::NextVisibleCharacter, {7, 8}, U'b'},
      };
      expectInvisibleDiagnostic(Consecutive.diagnostics()[0], {1, 4}, U'\u200C', DiagnosticSourceContext::Identifier, ConsecutiveRelated);
      expectInvisibleDiagnostic(Consecutive.diagnostics()[1], {4, 7}, U'\u200D', DiagnosticSourceContext::Identifier, ConsecutiveRelated);
      expectFullFidelity(Consecutive);
    }

    // Tests that default-ignorable XID members invalidate one complete identifier and diagnose only their exact bytes.
    TEST(IdentifiersTest, DefaultIgnorableXidMembersInvalidateTheWholeIdentifier)
    {
      struct InvisibleXidCase
      {
        const char *Name;
        std::string Source;
        std::string Invisible;
        char32_t ExpectedCharacter;
        char32_t PreviousCharacter;
        char32_t NextCharacter;
      };
      const std::vector<InvisibleXidCase> Cases = {
          {"Hangul choseong filler", utf8(u8"\u115Fa"), utf8(u8"\u115F"), U'\u115F', U'\0', U'a'},
          {"combining grapheme joiner", utf8(u8"a\u034Fb"), utf8(u8"\u034F"), U'\u034F', U'a', U'b'},
          {"supplementary variation selector", utf8(u8"a\U000E0100b"), utf8(u8"\U000E0100"), U'\U000E0100', U'a', U'b'},
      };

      for (const InvisibleXidCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const TokenizedBuffer Buffer = tokenize(TestSourceFileId, TestCase.Source);
        ASSERT_FALSE(Buffer.succeeded());
        ASSERT_EQ(Buffer.tokens().size(), 2U);
        EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::InvalidIdentifier);
        EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
        EXPECT_EQ(Buffer.tokens()[0].Span.End, TestCase.Source.size());
        EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), TestCase.Source);
        const std::size_t InvisibleStart = TestCase.Source.find(TestCase.Invisible);
        ASSERT_NE(InvisibleStart, std::string::npos);
        ASSERT_EQ(Buffer.diagnostics().size(), 1U);
        EXPECT_EQ(Buffer.diagnostics()[0].Kind, DiagnosticKind::InvisibleCharacter);
        EXPECT_EQ(Buffer.diagnostics()[0].Span.Start, InvisibleStart);
        EXPECT_EQ(Buffer.diagnostics()[0].Span.End, InvisibleStart + TestCase.Invisible.size());
        std::vector<RelatedCharacterExpectation> ExpectedRelated;
        if (TestCase.PreviousCharacter != U'\0')
        {
          ExpectedRelated.push_back({DiagnosticRelatedKind::PreviousVisibleCharacter, {InvisibleStart - 1, InvisibleStart}, TestCase.PreviousCharacter});
        }
        if (TestCase.NextCharacter != U'\0')
        {
          const std::size_t NextStart = InvisibleStart + TestCase.Invisible.size();
          ExpectedRelated.push_back({DiagnosticRelatedKind::NextVisibleCharacter, {NextStart, NextStart + 1}, TestCase.NextCharacter});
        }
        expectInvisibleDiagnostic(Buffer.diagnostics()[0], {InvisibleStart, InvisibleStart + TestCase.Invisible.size()}, TestCase.ExpectedCharacter, DiagnosticSourceContext::Identifier, ExpectedRelated);
        expectFullFidelity(Buffer);
      }
    }

    // Tests recovery when a default-ignorable scalar is not an XID member and therefore cannot join neighboring identifiers.
    TEST(IdentifiersTest, NonXidDefaultIgnorableSeparatesIdentifiersWithExactRecovery)
    {
      const std::string Isolate = utf8(u8"\u2066");
      const std::string Source = "a" + Isolate + "b";
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

      ASSERT_FALSE(Buffer.succeeded());
      ASSERT_EQ(Buffer.tokens().size(), 4U);
      EXPECT_EQ(Buffer.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[0].Span.Start, 0U);
      EXPECT_EQ(Buffer.tokens()[0].Span.End, 1U);
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[0]), "a");
      EXPECT_EQ(Buffer.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Buffer.tokens()[1].Span.Start, 1U);
      EXPECT_EQ(Buffer.tokens()[1].Span.End, 1U + Isolate.size());
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), Isolate);
      EXPECT_EQ(Buffer.tokens()[2].Kind, TokenKind::Identifier);
      EXPECT_EQ(Buffer.tokens()[2].Span.Start, 1U + Isolate.size());
      EXPECT_EQ(Buffer.tokens()[2].Span.End, Source.size());
      EXPECT_EQ(Buffer.raw(Buffer.tokens()[2]), "b");
      ASSERT_EQ(Buffer.diagnostics().size(), 1U);
      EXPECT_EQ(Buffer.diagnostics()[0].Kind, DiagnosticKind::InvisibleCharacter);
      EXPECT_EQ(Buffer.diagnostics()[0].Span.Start, 1U);
      EXPECT_EQ(Buffer.diagnostics()[0].Span.End, 1U + Isolate.size());
      const std::vector<RelatedCharacterExpectation> ExpectedRelated = {
          {DiagnosticRelatedKind::PreviousVisibleCharacter, {0, 1}, U'a'},
          {DiagnosticRelatedKind::NextVisibleCharacter, {1 + Isolate.size(), 2 + Isolate.size()}, U'b'},
      };
      expectInvisibleDiagnostic(Buffer.diagnostics()[0], {1, 1 + Isolate.size()}, U'\u2066', DiagnosticSourceContext::SourceText, ExpectedRelated);
      expectFullFidelity(Buffer);
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
        const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);
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
      const TokenizedBuffer Backtick = tokenize(TestSourceFileId, "`func`");
      ASSERT_FALSE(Backtick.succeeded());
      ASSERT_EQ(Backtick.tokens().size(), 4U);
      EXPECT_EQ(Backtick.tokens()[0].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Backtick.tokens()[1].Kind, TokenKind::Keyword);
      EXPECT_EQ(Backtick.tokens()[2].Kind, TokenKind::InvalidCharacter);
      expectFullFidelity(Backtick);

      const TokenizedBuffer Hash = tokenize(TestSourceFileId, "r#func");
      ASSERT_FALSE(Hash.succeeded());
      ASSERT_EQ(Hash.tokens().size(), 4U);
      EXPECT_EQ(Hash.tokens()[0].Kind, TokenKind::Identifier);
      EXPECT_EQ(Hash.tokens()[1].Kind, TokenKind::InvalidCharacter);
      EXPECT_EQ(Hash.tokens()[2].Kind, TokenKind::Keyword);
      expectFullFidelity(Hash);

      const TokenizedBuffer AtSign = tokenize(TestSourceFileId, "@func");
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
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Unicode16OnlyLetter);
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
      const TokenizedBuffer Buffer = tokenize(TestSourceFileId, Source);

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
