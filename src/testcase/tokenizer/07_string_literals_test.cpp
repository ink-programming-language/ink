#include "grammar_test_support.h"

namespace ink::tokenizer
{
  namespace
  {
    using namespace grammar_test;
    using core::DiagnosticKind;

    // Verifies empty, ASCII, non-ASCII and supplementary-plane contents in ordinary strings.
    TEST(StringLiteralTest, AcceptsEmptyAsciiAndUtf8Strings)
    {
      for (const std::string &Body : {std::string(), std::string("text"), utf8(u8"中文😀")})
      {
        expectString("\"" + Body + "\"", StringMode::EscapedSingleLine, Body);
      }
    }

    // Verifies every simple escape, including an embedded zero, decodes to the specified character.
    TEST(StringLiteralTest, DecodesEverySimpleEscape)
    {
      const std::vector<std::pair<std::string, char>> Cases = {
          {"\\\"", '"'},
          {"\\\\", '\\'},
          {"\\0", '\0'},
          {"\\a", '\a'},
          {"\\b", '\b'},
          {"\\f", '\f'},
          {"\\n", '\n'},
          {"\\r", '\r'},
          {"\\t", '\t'},
          {"\\v", '\v'},
      };
      for (const auto &Entry : Cases)
      {
        expectString("\"" + Entry.first + "\"", StringMode::EscapedSingleLine, std::string(1, Entry.second));
      }
    }

    // Verifies two-digit hex escapes decode U+00NN and do not consume a third digit.
    TEST(StringLiteralTest, HexEscapesHaveExactlyTwoDigits)
    {
      expectString("\"\\x00\\x41\\xFF\\x414\"", StringMode::EscapedSingleLine, std::string(1, '\0') + "A" + utf8(u8"\u00FF") + "A4");
    }

    // Verifies braced Unicode escapes accept one to six digits and exact scalar boundaries.
    TEST(StringLiteralTest, UnicodeEscapesAcceptScalarBoundaries)
    {
      const std::vector<std::pair<std::string, char32_t>> Cases = {
          {"0", 0},
          {"7F", 0x7F},
          {"80", 0x80},
          {"000041", 0x41},
          {"D7FF", 0xD7FF},
          {"E000", 0xE000},
          {"10FFFF", 0x10FFFF},
      };
      for (const auto &Entry : Cases)
      {
        std::string Expected;
        appendScalar(Expected, Entry.second);
        expectString("\"\\u{" + Entry.first + "}\"", StringMode::EscapedSingleLine, Expected);
      }
    }

    // Verifies direct controls and invisible scalars are content unless they are a delimiter or physical line ending.
    TEST(StringLiteralTest, DirectControlsAndInvisibleScalarsAreContent)
    {
      const std::string Body = bytes({0, 1, 7, 9, 11, 12, 0x7F}) + utf8(u8"\uFEFF\u2028\u2029\u2066");
      expectString("\"" + Body + "\"", StringMode::EscapedSingleLine, Body);
    }

    // Verifies strings retain canonically equivalent but byte-distinct content instead of applying identifier normalization.
    TEST(StringLiteralTest, StringContentsAreNotNormalized)
    {
      expectString("\"" + utf8(u8"cafe\u0301") + "\"", StringMode::EscapedSingleLine, utf8(u8"cafe\u0301"));
      expectString("\"" + utf8(u8"café") + "\"", StringMode::EscapedSingleLine, utf8(u8"café"));
    }

    // Verifies comment and interpolation markers remain ordinary string content.
    TEST(StringLiteralTest, CommentAndInterpolationMarkersRemainText)
    {
      expectString("\"/* ${value} // */\"", StringMode::EscapedSingleLine, "/* ${value} // */");
    }

    // Verifies unknown escapes and the removed apostrophe escape produce lexical errors.
    TEST(StringLiteralTest, RejectsUnknownEscapes)
    {
      for (const std::string &Body : {"\\q", "\\'", "\\z"})
      {
        expectStringError("\"" + Body + "\"", DiagnosticKind::UnknownEscape);
      }
    }

    // Verifies incomplete and nonhex fixed-width escapes recover at a closing quote.
    TEST(StringLiteralTest, RejectsMalformedHexEscapes)
    {
      for (const std::string &Body : {"\\x", "\\x0", "\\xGG", "\\x1Z"})
      {
        const TokenizedBuffer File = tokenize("\"" + Body + "\" after");
        ASSERT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidHexEscape));
        EXPECT_EQ(File.raw(File.tokens()[File.tokens().size() - 2]), "after");
        expectStream(File);
      }
    }

    // Verifies missing braces, empty bodies, nonhex characters and more than six digits are invalid Unicode escapes.
    TEST(StringLiteralTest, RejectsMalformedUnicodeEscapes)
    {
      for (const std::string &Body : {"\\u41", "\\u{}", "\\u{xyz}", "\\u{0000000}", "\\u{123"})
      {
        const TokenizedBuffer File = tokenize("\"" + Body + "\" after");
        ASSERT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidUnicodeEscape));
        EXPECT_EQ(File.raw(File.tokens()[File.tokens().size() - 2]), "after");
        expectStream(File);
      }
    }

    // Verifies surrogate and out-of-range code points are rejected despite syntactically valid escape digits.
    TEST(StringLiteralTest, RejectsUnicodeValuesThatAreNotScalars)
    {
      for (const std::string &Body : {"D800", "DFFF", "110000", "FFFFFF"})
      {
        expectStringError("\"\\u{" + Body + "}\"", DiagnosticKind::InvalidUnicodeScalar);
      }
    }

    // Verifies CR, LF and CRLF end an unclosed ordinary string and preserve following source text.
    TEST(StringLiteralTest, PhysicalLineBreakTerminatesUnclosedSingleLineString)
    {
      for (const std::string &Ending : {std::string("\n"), std::string("\r"), std::string("\r\n")})
      {
        const TokenizedBuffer File = tokenize("\"broken" + Ending + "after", preservingTrivia());
        ASSERT_FALSE(File.succeeded());
        ASSERT_EQ(File.tokens().size(), 4U);
        EXPECT_EQ(File.raw(File.tokens()[0]), "\"broken");
        EXPECT_EQ(File.tokens()[1].Kind, TokenKind::LineBreak);
        EXPECT_EQ(File.raw(File.tokens()[2]), "after");
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnterminatedStringLiteral));
        expectStream(File, true);
      }
    }

    // Verifies a backslash does not continue a string across a physical line ending.
    TEST(StringLiteralTest, BackslashDoesNotContinueTheSourceLine)
    {
      const TokenizedBuffer File = tokenize("\"broken\\\nafter");
      EXPECT_FALSE(File.succeeded());
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnterminatedStringLiteral));
      EXPECT_EQ(File.raw(File.tokens()[File.tokens().size() - 2]), "after");
      expectStream(File);
    }

    // Verifies an unclosed quote or trailing backslash at EOF remains one bounded string error.
    TEST(StringLiteralTest, UnterminatedStringAtEofRetainsAllBytes)
    {
      for (const std::string &Source : {"\"", "\"broken", "\"broken\\"})
      {
        expectStringError(Source, DiagnosticKind::UnterminatedStringLiteral);
      }
    }

    // Verifies invalid UTF-8 in an escape is reported without hiding the closing quote or following identifier.
    TEST(StringLiteralTest, InvalidUtf8InEscapeRecoversAtClosingQuote)
    {
      const TokenizedBuffer File = tokenize("\"\\" + bytes({0x80}) + "\" after");
      EXPECT_FALSE(File.succeeded());
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidUtf8));
      EXPECT_EQ(File.raw(File.tokens()[File.tokens().size() - 2]), "after");
      expectStream(File);
    }
  } // namespace
} // namespace ink::tokenizer