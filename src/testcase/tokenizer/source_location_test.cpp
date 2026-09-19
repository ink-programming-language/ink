#include "tokenizer_test_support.h"

#include <limits>
#include <type_traits>

namespace ink::tokenizer::test
{
  // Invalid locations are distinct from byte zero and retain a compact raw encoding.
  TEST(TokenizerSourceLocationTest, InvalidAndZeroOffsetAreDistinct)
  {
    using core::SourceLocation;
    static_assert(sizeof(SourceLocation) == sizeof(std::uint32_t));
    static_assert(sizeof(core::SourceRange) == 2 * sizeof(SourceLocation));
    static_assert(std::is_trivially_copyable_v<SourceLocation>);
    static_assert(std::is_trivially_copyable_v<core::SourceRange>);
    constexpr SourceLocation Invalid;
    constexpr auto Zero = SourceLocation::fromByteOffset(0);
    static_assert(Invalid.isInvalid() && Zero.isValid());
    EXPECT_NE(Invalid, Zero);
    EXPECT_EQ(Zero.getByteOffset(), 0U);
    EXPECT_EQ(SourceLocation::getFromRawEncoding(Zero.getRawEncoding()), Zero);
    EXPECT_EQ(Zero.getHashValue(), Zero.getRawEncoding());
  }

  // Offset arithmetic handles both ends of the encoding without wrapping.
  TEST(TokenizerSourceLocationTest, CheckedOffsetArithmetic)
  {
    using core::SourceLocation;
    const auto First = SourceLocation::fromByteOffset(0);
    const auto Last = SourceLocation::fromByteOffset(SourceLocation::MaxByteOffset);
    EXPECT_TRUE(First.getLocWithOffset(-1).isInvalid());
    EXPECT_TRUE(Last.getLocWithOffset(1).isInvalid());
    EXPECT_TRUE(SourceLocation::fromByteOffset(SourceLocation::MaxByteOffset + 1).isInvalid());
    EXPECT_TRUE(SourceLocation{}.getLocWithOffset(1).isInvalid());
    EXPECT_TRUE(First.getLocWithOffset(std::numeric_limits<std::int64_t>::min()).isInvalid());
    EXPECT_TRUE(First.getLocWithOffset(std::numeric_limits<std::int64_t>::max()).isInvalid());
    EXPECT_EQ(First.getLocWithOffset(123).getByteOffset(), 123U);
    EXPECT_EQ(Last.getLocWithOffset(-static_cast<std::int64_t>(SourceLocation::MaxByteOffset)), First);
    EXPECT_LT(First, Last);
    EXPECT_LE(First, First);
    EXPECT_GT(Last, First);
    EXPECT_GE(Last, Last);
  }

  // Half-open ranges distinguish invalid, reversed, nonempty and valid EOF ranges.
  TEST(TokenizerSourceLocationTest, RangeValidityAndContainment)
  {
    using core::SourceLocation;
    using core::SourceRange;
    EXPECT_TRUE(SourceRange{}.isInvalid());
    EXPECT_FALSE(SourceRange{}.empty());
    EXPECT_TRUE(SourceRange::fromByteOffsets(9, 2).isInvalid());
    auto Range = SourceRange::fromByteOffsets(2, 9);
    EXPECT_EQ(Range.size(), 7U);
    EXPECT_TRUE(Range.fullyContains(SourceRange::fromByteOffsets(3, 9)));
    EXPECT_TRUE(Range.fullyContains(SourceRange(SourceLocation::fromByteOffset(9))));
    EXPECT_FALSE(Range.fullyContains(SourceRange::fromByteOffsets(1, 9)));
    EXPECT_FALSE(Range.fullyContains({}));
    Range.setBegin(SourceLocation::fromByteOffset(4));
    Range.setEnd(SourceLocation::fromByteOffset(4));
    EXPECT_TRUE(Range.empty());
    EXPECT_EQ(Range.size(), 0U);
  }

  // NFC and literal decoding never move byte ranges or alter retained source bytes.
  TEST_F(TokenizerTest, OriginalByteRangesSurviveAllTransformations)
  {
    const std::string Source = u8"e\u0301 r\"x\\\" \"\"\"a\r\nb\rc\"\"\" '\\xFF'";
    const auto Buffer = lex(Source);
    expectKinds(Buffer, {TokenKind::Identifier, TokenKind::StringLiteral, TokenKind::StringLiteral, TokenKind::CharLiteral});
    ASSERT_EQ(Buffer.tokens().size(), 5U);
    EXPECT_EQ(Buffer.source(), Source);
    EXPECT_EQ(Buffer.tokens()[0].Span, core::SourceRange::fromByteOffsets(0, 3));
    EXPECT_EQ(std::get<IdentifierInfo>(Buffer.tokens()[0].Payload).Name, u8"\u00E9");
    EXPECT_EQ(Buffer.raw(Buffer.tokens()[1]), "r\"x\\\"");
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[1].Payload).Decoded, "x\\");
    EXPECT_EQ(Buffer.raw(Buffer.tokens()[2]), "\"\"\"a\r\nb\rc\"\"\"");
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[2].Payload).Decoded, "a\nb\nc");
    EXPECT_EQ(Buffer.raw(Buffer.tokens()[3]), "'\\xFF'");
    EXPECT_EQ(std::get<CharInfo>(Buffer.tokens()[3].Payload).Value, U'\u00FF');
  }

  // Source ownership survives tokenization and source IDs remain context-specific.
  TEST_F(TokenizerTest, RegisteredBuffersAndLineNumbers)
  {
    const auto Id = Compilation.sourceManager().addSource("sample.ink", "a\r\nb\rc\nd");
    const auto Buffer = tokenizeSource(Context, Id);
    expectKinds(Buffer, {TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier, TokenKind::Identifier});
    EXPECT_EQ(Buffer.sourceName(), "sample.ink");
    EXPECT_EQ(Buffer.sourceId(), Id);
    EXPECT_TRUE(Buffer.isRegisteredWith(Compilation.sourceManager()));
    EXPECT_EQ(Buffer.lineStarts(), (std::vector<std::size_t>{0, 3, 5, 7}));
    EXPECT_EQ(Buffer.lineNumber(0), 1U);
    EXPECT_EQ(Buffer.lineNumber(2), 1U);
    EXPECT_EQ(Buffer.lineNumber(3), 2U);
    EXPECT_EQ(Buffer.lineNumber(5), 3U);
    EXPECT_EQ(Buffer.lineNumber(8), 4U);
    core::SourceManager Other;
    EXPECT_FALSE(Buffer.isRegisteredWith(Other));
    Token Invalid;
    EXPECT_TRUE(Buffer.raw(Invalid).empty());
    Invalid.Span = core::SourceRange::fromByteOffsets(0, 999);
    EXPECT_TRUE(Buffer.raw(Invalid).empty());
  }

  // An unknown source ID produces an explicit diagnostic without a fake EOF.
  TEST_F(TokenizerTest, UnregisteredSourceReportsFailure)
  {
    const auto Buffer = tokenizeSource(Context, {});
    EXPECT_FALSE(Buffer.succeeded());
    EXPECT_TRUE(Buffer.tokens().empty());
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, DiagnosticKind::TokenizerSourceNotFound);
    EXPECT_TRUE(Diagnostics.diagnostics()[0].Span.isInvalid());
  }

  // A buffer owns its source and decoded payload after its compilation context is destroyed.
  TEST(TokenizerSourceLocationTest, BufferOutlivesContext)
  {
    const auto Buffer = []()
    {
      core::CompilationContext Compilation;
      core::FrontendContext Context(Compilation);
      return tokenize(Context, u8"e\u0301 \"\\x00\"");
    }();
    ASSERT_TRUE(Buffer.succeeded());
    EXPECT_EQ(Buffer.source(), u8"e\u0301 \"\\x00\"");
    EXPECT_EQ(std::get<IdentifierInfo>(Buffer.tokens()[0].Payload).Name, u8"\u00E9");
    EXPECT_EQ(std::get<StringInfo>(Buffer.tokens()[1].Payload).Decoded, std::string(1, '\0'));
  }
} // namespace ink::tokenizer::test
