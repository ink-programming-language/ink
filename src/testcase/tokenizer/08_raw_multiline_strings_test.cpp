#include "grammar_test_support.h"

namespace ink::tokenizer
{
  namespace
  {
    using namespace grammar_test;
    using core::DiagnosticKind;

    // Verifies four string modes have distinct payload modes with shared literal token classification.
    TEST(RawAndMultilineStringTest, DistinguishesAllFourStringModes)
    {
      expectString("\"text\"", StringMode::EscapedSingleLine, "text");
      expectString("r\"text\"", StringMode::RawSingleLine, "text");
      expectString("\"\"\"text\"\"\"", StringMode::EscapedMultiline, "text");
      expectString("r\"\"\"text\"\"\"", StringMode::RawMultiline, "text");
    }

    // Verifies raw single-line strings preserve backslashes and unknown escape lookalikes.
    TEST(RawAndMultilineStringTest, RawSingleLineDoesNotInterpretEscapes)
    {
      expectString("r\"\\n\\q\\u{D800}\\xGG\"", StringMode::RawSingleLine, "\\n\\q\\u{D800}\\xGG");
    }

    // Verifies a backslash before the closing quote does not escape that quote in raw mode.
    TEST(RawAndMultilineStringTest, BackslashCannotEscapeRawClosingQuote)
    {
      const TokenizedBuffer File = tokenize("r\"x\\\"after");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"r\"x\\\"", "after"});
      const StringInfo *Payload = std::get_if<StringInfo>(&File.tokens()[0].Payload);
      ASSERT_NE(Payload, nullptr);
      EXPECT_EQ(Payload->Decoded, "x\\");
    }

    // Verifies CR and LF terminate unterminated raw single-line literals with recovery at the next identifier.
    TEST(RawAndMultilineStringTest, RawSingleLineStopsBeforeEveryPhysicalLineEnding)
    {
      for (const std::string &Ending : {std::string("\r"), std::string("\n"), std::string("\r\n")})
      {
        const TokenizedBuffer File = tokenize("r\"broken" + Ending + "after");
        EXPECT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnterminatedStringLiteral));
        EXPECT_EQ(File.raw(File.tokens()[File.tokens().size() - 2]), "after");
        expectStream(File);
      }
    }

    // Verifies triple quotes allow an inline body and empty body without mandatory boundary line breaks.
    TEST(RawAndMultilineStringTest, InlineAndEmptyTripleQuotedFormsAreAccepted)
    {
      expectString("\"\"\"\"\"\"", StringMode::EscapedMultiline, "");
      expectString("r\"\"\"\"\"\"", StringMode::RawMultiline, "");
      expectString("\"\"\"inline\"\"\"", StringMode::EscapedMultiline, "inline");
      expectString("r\"\"\"inline\"\"\"", StringMode::RawMultiline, "inline");
    }

    // Verifies multiline contents retain all indentation and both boundary line breaks.
    TEST(RawAndMultilineStringTest, MultilineBodiesAreNotDedentedOrTrimmed)
    {
      const std::string Body = "\n    first\n  second\n    ";
      expectString("\"\"\"" + Body + "\"\"\"", StringMode::EscapedMultiline, Body);
      expectString("r\"\"\"" + Body + "\"\"\"", StringMode::RawMultiline, Body);
    }

    // Verifies multiline strings preserve CR, LF and CRLF bytes while line maps count physical line endings.
    TEST(RawAndMultilineStringTest, MultilineLineEndingsRemainUnmodified)
    {
      const std::string Body = "a\rb\nc\r\nd";
      for (const std::string &Prefix : {std::string("\"\"\""), std::string("r\"\"\"")})
      {
        const TokenizedBuffer File = tokenize(Prefix + Body + "\"\"\"");
        ASSERT_TRUE(File.succeeded());
        ASSERT_EQ(File.tokens().size(), 2U);
        const StringInfo *Payload = std::get_if<StringInfo>(&File.tokens()[0].Payload);
        ASSERT_NE(Payload, nullptr);
        EXPECT_EQ(Payload->Decoded, Body);
        EXPECT_EQ(File.lineStarts(), (std::vector<std::size_t>{0, Prefix.size() + 2, Prefix.size() + 4, Prefix.size() + 7}));
        expectStream(File);
      }
    }

    // Verifies the first unescaped triple quote closes a multiline string even in the middle of a physical line.
    TEST(RawAndMultilineStringTest, FirstTripleQuoteClosesAtAnyColumn)
    {
      const TokenizedBuffer File = tokenize("\"\"\"first\"\"\"after r\"\"\"second\"\"\";");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"\"\"\"first\"\"\"", "after", "r\"\"\"second\"\"\"", ";"});
    }

    // Verifies one or two quotes are body content while only a run of three ends multiline raw text.
    TEST(RawAndMultilineStringTest, OneAndTwoQuotesRemainMultilineContent)
    {
      const std::string Body = "a\"b\"\"c";
      expectString("\"\"\"" + Body + "\"\"\"", StringMode::EscapedMultiline, Body);
      expectString("r\"\"\"" + Body + "\"\"\"", StringMode::RawMultiline, Body);
    }

    // Verifies escaped quotes protect an otherwise triple-quote-looking body in escaped multiline mode.
    TEST(RawAndMultilineStringTest, EscapedQuoteDoesNotStartTheClosingDelimiter)
    {
      expectString("\"\"\"\\\"\"\"tail\"\"\"", StringMode::EscapedMultiline, "\"\"\"tail");
    }

    // Verifies escaped multiline strings interpret escapes while raw multiline strings preserve them.
    TEST(RawAndMultilineStringTest, OnlyEscapedMultilineProcessesBackslashes)
    {
      expectString("\"\"\"a\\n\\u{41}\\x42\"\"\"", StringMode::EscapedMultiline, "a\nAB");
      expectString("r\"\"\"a\\n\\u{41}\\x42\"\"\"", StringMode::RawMultiline, "a\\n\\u{41}\\x42");
    }

    // Verifies arbitrary scalar values, including NUL and invisible text, remain valid raw and multiline contents.
    TEST(RawAndMultilineStringTest, DirectControlsAndInvisibleScalarsAreContent)
    {
      const std::string Body = bytes({0, 1, 11, 0x7F}) + utf8(u8"\uFEFF\u2066\u2028");
      expectString("r\"" + Body + "\"", StringMode::RawSingleLine, Body);
      expectString("\"\"\"" + Body + "\"\"\"", StringMode::EscapedMultiline, Body);
      expectString("r\"\"\"" + Body + "\"\"\"", StringMode::RawMultiline, Body);
    }

    // Verifies recognizing triple quotes commits to a multiline token and never falls back to shorter quoted strings.
    TEST(RawAndMultilineStringTest, UnclosedTripleQuoteDoesNotBacktrack)
    {
      for (const std::string &Source : {"\"\"\"", "\"\"\"\"", "\"\"\"\"\"", "\"\"\"text", "r\"\"\"", "r\"\"\"\"\""})
      {
        const TokenizedBuffer File = tokenize(Source);
        EXPECT_FALSE(File.succeeded());
        ASSERT_EQ(File.tokens().size(), 2U);
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::InvalidStringLiteral);
        EXPECT_EQ(File.raw(File.tokens()[0]), Source);
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnterminatedMultilineStringLiteral));
        expectStream(File);
      }
    }

    // Verifies escaped multiline mode rejects a direct backslash followed by a physical newline.
    TEST(RawAndMultilineStringTest, MultilineBackslashIsStillAnEscapeIntroducer)
    {
      expectStringError("\"\"\"a\\\nb\"\"\"", DiagnosticKind::UnknownEscape);
    }

    // Verifies scanning very long raw and escaped multiline bodies uses bounded stack space.
    TEST(RawAndMultilineStringTest, LongBodiesRetainEveryCharacter)
    {
      const std::string Body(200000, 'x');
      expectString("\"\"\"" + Body + "\"\"\"", StringMode::EscapedMultiline, Body);
      expectString("r\"\"\"" + Body + "\"\"\"", StringMode::RawMultiline, Body);
    }
  } // namespace
} // namespace ink::tokenizer