#include "grammar_test_support.h"

namespace ink::tokenizer
{
  namespace
  {
    using namespace grammar_test;
    using core::DiagnosticKind;

    // Verifies an empty source has one EOF and a line-one byte origin.
    TEST(SourceEncodingTest, EmptyFileHasExactlyOneEofToken)
    {
      const TokenizedBuffer File = tokenize("");
      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 1U);
      EXPECT_EQ(File.lineStarts(), (std::vector<std::size_t>{0}));
      expectStream(File);
    }

    // Verifies UTF-8 BOM is an invalid source scalar even at byte zero and does not consume the following identifier.
    TEST(SourceEncodingTest, ByteOrderMarkIsNotLexicalTrivia)
    {
      for (const std::string &Prefix : {std::string(), std::string("before ")})
      {
        const TokenizedBuffer File = tokenize(Prefix + bytes({0xEF, 0xBB, 0xBF}) + "after");
        ASSERT_FALSE(File.succeeded());
        const std::size_t Index = Prefix.empty() ? 0 : 1;
        ASSERT_GT(File.tokens().size(), Index + 1);
        EXPECT_EQ(File.tokens()[Index].Kind, TokenKind::InvalidCharacter);
        EXPECT_EQ(File.tokens()[Index].Span, (core::SourceRange{Prefix.size(), Prefix.size() + 3}));
        EXPECT_EQ(File.raw(File.tokens()[Index + 1]), "after");
        expectStream(File);
      }
    }

    // Verifies every Unicode scalar is valid comment content, including NUL, invisible characters and noncharacters.
    TEST(SourceEncodingTest, StrictUtf8AcceptsEveryUnicodeScalarInComments)
    {
      std::string Source = "/*";
      for (char32_t Scalar = 0; Scalar <= 0x10FFFF; ++Scalar)
      {
        if (Scalar < 0xD800 || Scalar > 0xDFFF)
        {
          appendScalar(Source, Scalar);
        }
      }
      Source += "*/";
      const TokenizedBuffer File = tokenize(Source, preservingTrivia());
      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2U);
      EXPECT_EQ(File.tokens()[0].Kind, TokenKind::BlockComment);
      EXPECT_TRUE(testDiagnostics(File).empty());
      expectStream(File, true);
    }

    // Verifies UTF-8 boundary encodings retain their exact byte representation in raw strings.
    TEST(SourceEncodingTest, StrictUtf8AcceptsExactScalarBoundaryEncodings)
    {
      const std::vector<char32_t> Scalars = {
          0,
          0x7F,
          0x80,
          0x7FF,
          0x800,
          0xD7FF,
          0xE000,
          0xFFFF,
          0x10000,
          0x10FFFF,
      };
      for (char32_t Scalar : Scalars)
      {
        SCOPED_TRACE(static_cast<unsigned>(Scalar));
        std::string Body;
        appendScalar(Body, Scalar);
        expectString("r\"" + Body + "\"", StringMode::RawSingleLine, Body);
      }
    }

    // Verifies invalid leading bytes are rejected individually without swallowing an ASCII suffix.
    TEST(SourceEncodingTest, StrictUtf8RejectsEveryInvalidLeadingBytePrecisely)
    {
      for (unsigned Byte = 0x80; Byte <= 0xFF; ++Byte)
      {
        if (Byte >= 0xC2 && Byte <= 0xF4)
        {
          continue;
        }
        SCOPED_TRACE(Byte);
        const TokenizedBuffer File = tokenize(bytes({Byte}) + "after");
        ASSERT_FALSE(File.succeeded());
        ASSERT_EQ(File.tokens().size(), 3U);
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::InvalidEncoding);
        EXPECT_EQ(File.tokens()[0].Span, (core::SourceRange{0, 1}));
        EXPECT_EQ(File.raw(File.tokens()[1]), "after");
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidUtf8));
        expectStream(File);
      }
    }

    // Verifies overlong, surrogate, out-of-range, malformed and truncated UTF-8 sequences recover before a following identifier.
    TEST(SourceEncodingTest, RejectsInvalidSequenceClassesAndPreservesFollowingIdentifier)
    {
      const std::vector<std::string> Cases = {
          bytes({0xC0, 0xAF}),
          bytes({0xC1, 0xBF}),
          bytes({0xE0, 0x80, 0x80}),
          bytes({0xED, 0xA0, 0x80}),
          bytes({0xF0, 0x80, 0x80, 0x80}),
          bytes({0xF4, 0x90, 0x80, 0x80}),
          bytes({0xF5, 0x80, 0x80, 0x80}),
          bytes({0xC2}),
          bytes({0xE2}),
          bytes({0xE2, 0x82}),
          bytes({0xF0}),
          bytes({0xF0, 0x90}),
          bytes({0xF0, 0x90, 0x80}),
      };
      for (const std::string &Invalid : Cases)
      {
        const TokenizedBuffer File = tokenize(Invalid + "after", preservingTrivia());
        ASSERT_FALSE(File.succeeded());
        ASSERT_GE(File.tokens().size(), 3U);
        EXPECT_EQ(File.tokens()[File.tokens().size() - 2].Kind, TokenKind::Identifier);
        EXPECT_EQ(File.raw(File.tokens()[File.tokens().size() - 2]), "after");
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidUtf8));
        expectStream(File, true);
      }
    }

    // Verifies invalid restricted second-byte ranges never enter the scalar decoder as legal characters.
    TEST(SourceEncodingTest, RejectsRestrictedSecondByteBoundaries)
    {
      const std::vector<std::pair<unsigned, unsigned>> Cases = {
          {0xE0, 0x80},
          {0xE0, 0x9F},
          {0xED, 0xA0},
          {0xED, 0xBF},
          {0xF0, 0x80},
          {0xF0, 0x8F},
          {0xF4, 0x90},
          {0xF4, 0xBF},
      };
      for (const auto &Entry : Cases)
      {
        std::string Source = bytes({Entry.first, Entry.second, 0x80});
        if (Entry.first >= 0xF0)
        {
          Source += bytes({0x80});
        }
        const TokenizedBuffer File = tokenize(Source + " end");
        EXPECT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidUtf8));
        expectStream(File);
      }
    }

    // Verifies UTF-8 validation is active in every opaque lexical state.
    TEST(SourceEncodingTest, InvalidUtf8IsDiagnosedInsideCommentsAndAllStringModes)
    {
      const std::vector<std::pair<std::string, std::string>> Delimiters = {
          {"//", "\n"},
          {"/*", "*/"},
          {"\"", "\""},
          {"r\"", "\""},
          {"\"\"\"", "\"\"\""},
          {"r\"\"\"", "\"\"\""},
      };
      for (const auto &Entry : Delimiters)
      {
        const TokenizedBuffer File = tokenize(Entry.first + bytes({0x80}) + Entry.second + " after", preservingTrivia());
        EXPECT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidUtf8));
        EXPECT_EQ(File.raw(File.tokens()[File.tokens().size() - 2]), "after");
        expectStream(File, true);
      }
    }

    // Verifies CR, LF and CRLF are accepted trivia and line maps preserve physical byte boundaries.
    TEST(SourceEncodingTest, AllDefinedLineEndingsAreAccepted)
    {
      const TokenizedBuffer File = tokenize("a\rb\nc\r\nd\r", preservingTrivia());
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.lineStarts(), (std::vector<std::size_t>{0, 2, 4, 7, 9}));
      EXPECT_EQ(File.lineNumber(5), 3U);
      EXPECT_EQ(File.lineNumber(6), 3U);
      EXPECT_EQ(File.lineNumber(7), 4U);
      expectStream(File, true);
    }

    // Verifies Unicode spacing and newline lookalikes are invalid outside tokens and do not create source lines.
    TEST(SourceEncodingTest, UnicodeWhitespaceIsNotLexicalTrivia)
    {
      const std::vector<char32_t> Scalars = {
          0x85,
          0xA0,
          0x1680,
          0x2000,
          0x2028,
          0x2029,
          0x202F,
          0x205F,
          0x3000,
      };
      for (char32_t Scalar : Scalars)
      {
        std::string Source;
        appendScalar(Source, Scalar);
        const TokenizedBuffer File = tokenize(Source + "after");
        EXPECT_FALSE(File.succeeded());
        ASSERT_EQ(File.tokens().size(), 3U);
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::InvalidCharacter);
        EXPECT_EQ(File.lineStarts(), (std::vector<std::size_t>{0}));
        EXPECT_EQ(File.raw(File.tokens()[1]), "after");
        expectStream(File);
      }
    }

    // Verifies NUL and non-whitespace ASCII controls outside strings or comments are errors with forward progress.
    TEST(SourceEncodingTest, ControlsOutsideTokensAreNotWhitespace)
    {
      for (unsigned Byte = 0; Byte <= 0x1F; ++Byte)
      {
        if (Byte == 9 || Byte == 10 || Byte == 13)
        {
          continue;
        }
        const TokenizedBuffer File = tokenize(bytes({Byte}) + "after");
        EXPECT_FALSE(File.succeeded());
        ASSERT_EQ(File.tokens().size(), 3U);
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::InvalidCharacter);
        EXPECT_EQ(File.raw(File.tokens()[1]), "after");
        expectStream(File);
      }
    }
  } // namespace
} // namespace ink::tokenizer