#include "grammar_test_support.h"

namespace ink::tokenizer
{
  namespace
  {
    using namespace grammar_test;
    using core::DiagnosticKind;

    // Verifies ASCII and NFC Unicode XID spellings are preserved without normalization or case folding.
    TEST(IdentifiersTest, AcceptsAsciiUnicodeAndMixedWritingSystems)
    {
      const std::vector<std::string> Cases = {
          "value",
          "_100",
          "__",
          "x9",
          "Class",
          utf8(u8"变量"),
          utf8(u8"café"),
          utf8(u8"Ελληνικά"),
          utf8(u8"аa"),
      };
      for (const std::string &Source : Cases)
      {
        SCOPED_TRACE(Source);
        const TokenizedBuffer File = tokenize(Source);
        ASSERT_TRUE(File.succeeded());
        expectRaws(File, {Source});
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::Identifier);
        EXPECT_TRUE(std::holds_alternative<std::monostate>(File.tokens()[0].Payload));
      }
    }

    // Verifies underscore alone is punctuation but any longer valid underscore spelling is an identifier.
    TEST(IdentifiersTest, UnderscoreHasItsOwnSymbolToken)
    {
      const TokenizedBuffer File = tokenize("_ __ _0 _a");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"_", "__", "_0", "_a"});
      EXPECT_EQ(File.tokens()[0].Kind, TokenKind::Symbol);
      for (std::size_t Index = 1; Index < 4; ++Index)
      {
        EXPECT_EQ(File.tokens()[Index].Kind, TokenKind::Identifier);
      }
    }

    // Verifies both endpoints of the pinned Unicode 15.1 CJK Extension I range and adjacent excluded scalars.
    TEST(IdentifiersTest, Unicode151XidStartRangeHasExactBoundaryBehavior)
    {
      for (const std::string &Source : {utf8(u8"\U0002EBF0"), utf8(u8"\U0002EE5D")})
      {
        const TokenizedBuffer File = tokenize(Source);
        ASSERT_TRUE(File.succeeded());
        expectRaws(File, {Source});
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::Identifier);
      }
      for (const std::string &Source : {utf8(u8"\U0002EBEF"), utf8(u8"\U0002EE5E")})
      {
        const TokenizedBuffer File = tokenize(Source);
        EXPECT_FALSE(File.succeeded());
        ASSERT_EQ(File.tokens().size(), 2U);
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::InvalidCharacter);
        EXPECT_EQ(File.tokens()[0].Span, (core::SourceRange{0, Source.size()}));
        expectStream(File);
      }
      EXPECT_STREQ(UnicodeVersion, "15.1.0");
    }

    // Verifies continue-only XID scalars join names but cannot begin them.
    TEST(IdentifiersTest, ContinueOnlyScalarsRequireAnIdentifierStarter)
    {
      for (const std::string &Scalar : {utf8(u8"\u00B7"), utf8(u8"\u0660"), utf8(u8"\u0301")})
      {
        const TokenizedBuffer Continued = tokenize("q" + Scalar + "z");
        EXPECT_TRUE(Continued.succeeded());
        expectRaws(Continued, {"q" + Scalar + "z"});
        const TokenizedBuffer Initial = tokenize(Scalar + "q");
        EXPECT_FALSE(Initial.succeeded());
        ASSERT_EQ(Initial.tokens().size(), 3U);
        EXPECT_EQ(Initial.tokens()[0].Kind, TokenKind::InvalidCharacter);
        EXPECT_EQ(Initial.tokens()[0].Span, (core::SourceRange{0, Scalar.size()}));
        EXPECT_EQ(Initial.raw(Initial.tokens()[1]), "q");
        expectStream(Initial);
      }
    }

    // Verifies the complete XID candidate is rejected for non-NFC composition and canonical ordering.
    TEST(IdentifiersTest, RejectsNonNfcWithoutChangingOriginalSource)
    {
      const std::vector<std::string> Cases = {
          utf8(u8"cafe\u0301"),
          utf8(u8"\u1100\u1161"),
          utf8(u8"q\u0315\u0300"),
          utf8(u8"\u212B"),
      };
      for (const std::string &Source : Cases)
      {
        const TokenizedBuffer File = tokenize(Source + "; after");
        EXPECT_FALSE(File.succeeded());
        ASSERT_EQ(File.tokens().size(), 4U);
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::InvalidIdentifier);
        EXPECT_EQ(File.tokens()[0].Span, (core::SourceRange{0, Source.size()}));
        EXPECT_EQ(File.raw(File.tokens()[0]), Source);
        EXPECT_EQ(File.raw(File.tokens()[2]), "after");
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::IdentifierNotNfc));
        expectStream(File);
      }
    }

    // Verifies valid default-ignorable XID members are not rejected by an additional hidden source policy.
    TEST(IdentifiersTest, DefaultIgnorableXidMembersAreAccepted)
    {
      for (const std::string &Source : {utf8(u8"\u115Fa"), utf8(u8"a\u034Fb"), utf8(u8"a\U000E0100b")})
      {
        const TokenizedBuffer File = tokenize(Source);
        ASSERT_TRUE(File.succeeded());
        EXPECT_TRUE(testDiagnostics(File).empty());
        expectRaws(File, {Source});
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::Identifier);
      }
    }

    // Verifies non-XID format characters and emoji fail at their own bytes and keep names on both sides.
    TEST(IdentifiersTest, InvalidScalarsSeparateIdentifiersWithoutSwallowingThem)
    {
      for (const std::string &Scalar : {utf8(u8"\u2066"), utf8(u8"😀"), std::string("@")})
      {
        const TokenizedBuffer File = tokenize("before" + Scalar + "after");
        EXPECT_FALSE(File.succeeded());
        ASSERT_EQ(File.tokens().size(), 4U);
        EXPECT_EQ(File.raw(File.tokens()[0]), "before");
        EXPECT_EQ(File.tokens()[1].Kind, TokenKind::InvalidCharacter);
        EXPECT_EQ(File.tokens()[1].Span, (core::SourceRange{6, 6 + Scalar.size()}));
        EXPECT_EQ(File.raw(File.tokens()[2]), "after");
        expectStream(File);
      }
    }

    // Verifies a leading decimal run ends before an identifier instead of committing to a malformed numeric candidate.
    TEST(IdentifiersTest, LeadingDigitsAndIdentifierAreSeparateTokens)
    {
      const TokenizedBuffer File = tokenize("123abc 10" + utf8(u8"变量"));
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"123", "abc", "10", utf8(u8"变量")});
      EXPECT_EQ(File.tokens()[0].Kind, TokenKind::IntegerLiteral);
      EXPECT_EQ(File.tokens()[1].Kind, TokenKind::Identifier);
    }

    // Verifies long and multibyte identifiers use byte spans and do not depend on recursive character scanning.
    TEST(IdentifiersTest, LongIdentifierPreservesItsEntireByteSpan)
    {
      const std::string Source = utf8(u8"变量") + std::string(100000, 'x');
      const TokenizedBuffer File = tokenize(Source);
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {Source});
      EXPECT_EQ(File.tokens()[0].Span, (core::SourceRange{0, Source.size()}));
    }
  } // namespace
} // namespace ink::tokenizer