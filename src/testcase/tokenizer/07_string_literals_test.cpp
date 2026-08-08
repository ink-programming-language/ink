#include "ink/tokenizer/tokenizer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace ink::tokenizer
{
  namespace
  {
    using core::Diagnostic;
    using core::DiagnosticKind;
    using core::SourceRange;

    void expectPartition(const TokenizedBuffer &File)
    {
      ASSERT_FALSE(File.tokens().empty());
      std::size_t Cursor = 0;
      std::size_t EofCount = 0;
      std::string Rebuilt;
      for (const Token &Token : File.tokens())
      {
        if (Token.Kind == TokenKind::EndOfFile)
        {
          ++EofCount;
          EXPECT_EQ(Token.Span, (SourceRange{File.source().size(), File.source().size()}));
          EXPECT_TRUE(File.raw(Token).empty());
          continue;
        }
        EXPECT_EQ(Token.Span.Start, Cursor);
        EXPECT_EQ(Token.Span.size(), File.raw(Token).size());
        Rebuilt.append(File.raw(Token).data(), File.raw(Token).size());
        Cursor = Token.Span.End;
      }
      EXPECT_EQ(Cursor, File.source().size());
      EXPECT_EQ(Rebuilt, File.source());
      EXPECT_EQ(EofCount, 1U);
      EXPECT_EQ(File.tokens().back().Kind, TokenKind::EndOfFile);
    }

    void expectToken(const TokenizedBuffer &File, std::size_t Index, TokenKind Kind, std::string_view Raw)
    {
      ASSERT_LT(Index, File.tokens().size());
      EXPECT_EQ(File.tokens()[Index].Kind, Kind);
      EXPECT_EQ(std::string(File.raw(File.tokens()[Index])), std::string(Raw));
    }

    bool hasDiagnostic(const TokenizedBuffer &File, DiagnosticKind Kind)
    {
      return std::any_of(File.diagnostics().begin(), File.diagnostics().end(), [Kind](const Diagnostic &Diagnostic)
                         {
                           return Diagnostic.Kind == Kind;
                         });
    }

    const StringInfo &stringPayload(const Token &Token)
    {
      return std::get<StringInfo>(Token.Payload);
    }

    // Verifies empty, ASCII, non-ASCII, and supplementary-plane single-line strings.
    TEST(StringLiteralTest, LexesEmptyAsciiAndUtf8Strings)
    {
      const std::string Chinese = "\xE4\xBD\xA0\xE5\xA5\xBD";
      const std::string Emoji = "\xF0\x9F\x98\x80";
      const std::string Source = "\"\" \"hello\" \"" + Chinese + "\" \"" + Emoji + "\"";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 8U);
      expectToken(File, 0, TokenKind::StringLiteral, "\"\"");
      expectToken(File, 2, TokenKind::StringLiteral, "\"hello\"");
      expectToken(File, 4, TokenKind::StringLiteral, std::string("\"") + Chinese + "\"");
      expectToken(File, 6, TokenKind::StringLiteral, std::string("\"") + Emoji + "\"");
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, "");
      EXPECT_EQ(stringPayload(File.tokens()[2]).Decoded, "hello");
      EXPECT_EQ(stringPayload(File.tokens()[4]).Decoded, Chinese);
      EXPECT_EQ(stringPayload(File.tokens()[6]).Decoded, Emoji);
      EXPECT_EQ(stringPayload(File.tokens()[6]).Mode, StringMode::EscapedSingleLine);
      expectPartition(File);
    }

    // Verifies decoding of every supported simple escape in a string literal.
    TEST(StringLiteralTest, DecodesEverySimpleEscape)
    {
      const std::string Source = R"ink("\\\'\"\0\n\r\t")ink";
      std::string Expected;
      Expected.push_back('\\');
      Expected.push_back('\'');
      Expected.push_back('"');
      Expected.push_back('\0');
      Expected.push_back('\n');
      Expected.push_back('\r');
      Expected.push_back('\t');

      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2U);
      expectToken(File, 0, TokenKind::StringLiteral, Source);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, Expected);
      expectPartition(File);
    }

    // Verifies fixed-width hexadecimal and braced Unicode escape decoding in strings.
    TEST(StringLiteralTest, DecodesFixedWidthHexAndUnicodeEscapes)
    {
      const std::string Source = R"ink("\x00\xFF" "\u{0}\u{4E2D}\u{1F600}\u{10FFFF}")ink";
      const std::string ExpectedHex("\0\xC3\xBF", 3);
      const std::string ExpectedUnicode = std::string("\0", 1) + "\xE4\xB8\xAD\xF0\x9F\x98\x80\xF4\x8F\xBF\xBF";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 4U);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, ExpectedHex);
      EXPECT_EQ(stringPayload(File.tokens()[2]).Decoded, ExpectedUnicode);
      expectPartition(File);
    }

    // Verifies that a hexadecimal escape consumes exactly two hexadecimal digits.
    TEST(StringLiteralTest, HexEscapeConsumesExactlyTwoDigits)
    {
      const std::string Source = R"ink("\x123")ink";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2U);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, std::string("\x12") + "3");
      expectPartition(File);
    }

    // Verifies that decoded string contents retain distinct Unicode scalar sequences without NFC normalization.
    TEST(StringLiteralTest, PreservesDistinctUnicodeScalarSequencesWithoutNormalization)
    {
      const std::string Precomposed = "\xC3\xA9";
      const std::string Decomposed = "e\xCC\x81";
      const std::string Source = std::string("\"") + Precomposed + "\" \"e\\u{301}\"";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 4U);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, Precomposed);
      EXPECT_EQ(stringPayload(File.tokens()[2]).Decoded, Decomposed);
      EXPECT_NE(stringPayload(File.tokens()[0]).Decoded, stringPayload(File.tokens()[2]).Decoded);
      expectPartition(File);
    }

    // Verifies that interpolation markers and comment delimiters are ordinary string content.
    TEST(StringLiteralTest, TreatsInterpolationAndCommentDelimitersAsText)
    {
      const std::string Source = R"ink("${value} // /* not comments */")ink";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2U);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, "${value} // /* not comments */");
      expectPartition(File);
    }

    // Verifies that adjacent strings and a following identifier remain separate tokens.
    TEST(StringLiteralTest, KeepsAdjacentStringsAndSuffixIdentifierSeparate)
    {
      const std::string Source = R"ink("first""second"name)ink";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 4U);
      expectToken(File, 0, TokenKind::StringLiteral, "\"first\"");
      expectToken(File, 1, TokenKind::StringLiteral, "\"second\"");
      expectToken(File, 2, TokenKind::Identifier, "name");
      expectPartition(File);
    }

    // Verifies that an unsupported simple escape produces an invalid string and diagnostic.
    TEST(StringLiteralTest, RejectsUnknownSimpleEscape)
    {
      const std::string Source = R"ink("\q")ink";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      expectToken(File, 0, TokenKind::InvalidStringLiteral, Source);
      EXPECT_TRUE(File.tokens()[0].isError());
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnknownEscape));
      expectPartition(File);
    }

    // Verifies malformed fixed-width hexadecimal escapes are rejected.
    TEST(StringLiteralTest, RejectsMalformedHexEscapes)
    {
      const std::vector<std::string> Sources = {
          R"ink("\x")ink",
          R"ink("\x1")ink",
          R"ink("\xG0")ink",
      };

      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const TokenizedBuffer File = tokenize(Source);
        EXPECT_FALSE(File.succeeded());
        ASSERT_GE(File.tokens().size(), 2U);
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::InvalidStringLiteral);
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidHexEscape));
        expectPartition(File);
      }
    }

    // Verifies malformed braced Unicode escape syntax is rejected.
    TEST(StringLiteralTest, RejectsMalformedUnicodeEscapes)
    {
      const std::vector<std::string> Sources = {
          R"ink("\u0041")ink",
          R"ink("\u{}")ink",
          R"ink("\u{1234567}")ink",
          R"ink("\u{1_2}")ink",
          R"ink("\u{ 41}")ink",
          R"ink("\u{+41}")ink",
          R"ink("\u{0x41}")ink",
      };

      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const TokenizedBuffer File = tokenize(Source);
        EXPECT_FALSE(File.succeeded());
        ASSERT_GE(File.tokens().size(), 2U);
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::InvalidStringLiteral);
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidUnicodeEscape));
        expectPartition(File);
      }
    }

    // Verifies that surrogate code points and out-of-range Unicode escapes are rejected.
    TEST(StringLiteralTest, RejectsNonScalarUnicodeEscapes)
    {
      const std::vector<std::string> Sources = {
          R"ink("\u{D800}")ink",
          R"ink("\u{DFFF}")ink",
          R"ink("\u{110000}")ink",
      };

      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const TokenizedBuffer File = tokenize(Source);
        EXPECT_FALSE(File.succeeded());
        ASSERT_GE(File.tokens().size(), 2U);
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::InvalidStringLiteral);
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidUnicodeScalar));
        expectPartition(File);
      }
    }

    // Verifies that a Unicode escape missing its right brace stops at the string quote and scanning resumes afterward.
    TEST(StringLiteralTest, MissingUnicodeEscapeBraceRecoversAtClosingQuote)
    {
      const std::string Source = "\"\\u{41\"next";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 3U);
      expectToken(File, 0, TokenKind::InvalidStringLiteral, "\"\\u{41\"");
      EXPECT_EQ(File.tokens()[0].Span, (SourceRange{0, 7}));
      expectToken(File, 1, TokenKind::Identifier, "next");
      EXPECT_EQ(File.tokens()[1].Span, (SourceRange{7, 11}));
      EXPECT_EQ(File.tokens()[2].Kind, TokenKind::EndOfFile);
      ASSERT_EQ(File.diagnostics().size(), 1U);
      EXPECT_EQ(File.diagnostics().front().Kind, DiagnosticKind::InvalidUnicodeEscape);
      EXPECT_EQ(File.diagnostics().front().Span, (SourceRange{1, 6}));
      expectPartition(File);
    }

    // Verifies that a trailing backslash at end of file reports both the incomplete escape and unterminated string.
    TEST(StringLiteralTest, TrailingBackslashAtEofReportsBothRootDiagnostics)
    {
      const std::string Source = "\"\\";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2U);
      expectToken(File, 0, TokenKind::InvalidStringLiteral, Source);
      EXPECT_EQ(File.tokens()[0].Span, (SourceRange{0, 2}));
      EXPECT_EQ(File.tokens()[1].Kind, TokenKind::EndOfFile);
      ASSERT_EQ(File.diagnostics().size(), 2U);
      EXPECT_EQ(File.diagnostics()[0].Kind, DiagnosticKind::UnknownEscape);
      EXPECT_EQ(File.diagnostics()[0].Span, (SourceRange{1, 2}));
      EXPECT_EQ(File.diagnostics()[1].Kind, DiagnosticKind::UnterminatedStringLiteral);
      EXPECT_EQ(File.diagnostics()[1].Span, (SourceRange{0, 2}));
      expectPartition(File);
    }

    // Verifies that invalid UTF-8 immediately after a backslash is contained within the string and later tokens survive recovery.
    TEST(StringLiteralTest, InvalidUtf8AfterBackslashRecoversAtClosingQuote)
    {
      std::string Source = "\"\\";
      Source.push_back(static_cast<char>(0x80));
      Source += "\"next";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 3U);
      expectToken(File, 0, TokenKind::InvalidStringLiteral, std::string_view(Source.data(), 4));
      EXPECT_EQ(File.tokens()[0].Span, (SourceRange{0, 4}));
      expectToken(File, 1, TokenKind::Identifier, "next");
      EXPECT_EQ(File.tokens()[1].Span, (SourceRange{4, 8}));
      EXPECT_EQ(File.tokens()[2].Kind, TokenKind::EndOfFile);
      ASSERT_EQ(File.diagnostics().size(), 1U);
      EXPECT_EQ(File.diagnostics().front().Kind, DiagnosticKind::InvalidUtf8);
      EXPECT_EQ(File.diagnostics().front().Span, (SourceRange{2, 3}));
      expectPartition(File);
    }

    // Verifies that an unterminated string stops before LF and scanning resumes on the next line.
    TEST(StringLiteralTest, StopsInvalidSingleLineStringBeforeLfAndRecovers)
    {
      const std::string Source = "\"oops\nnext";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 4U);
      expectToken(File, 0, TokenKind::InvalidStringLiteral, "\"oops");
      expectToken(File, 1, TokenKind::LineBreak, "\n");
      expectToken(File, 2, TokenKind::Identifier, "next");
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnterminatedStringLiteral));
      expectPartition(File);
    }

    // Verifies that an unterminated string stops before CRLF and scanning resumes afterward.
    TEST(StringLiteralTest, StopsInvalidSingleLineStringBeforeCrlfAndRecovers)
    {
      const std::string Source = "\"oops\r\nnext";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 4U);
      expectToken(File, 0, TokenKind::InvalidStringLiteral, "\"oops");
      expectToken(File, 1, TokenKind::LineBreak, "\r\n");
      expectToken(File, 2, TokenKind::Identifier, "next");
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnterminatedStringLiteral));
      expectPartition(File);
    }

    // Verifies that a backslash does not continue a single-line string across LF or CRLF.
    TEST(StringLiteralTest, BackslashDoesNotContinueASingleLineStringAcrossLf)
    {
      const std::string Source = "\"oops\\\nnext";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 4U);
      expectToken(File, 0, TokenKind::InvalidStringLiteral, "\"oops\\");
      expectToken(File, 1, TokenKind::LineBreak, "\n");
      expectToken(File, 2, TokenKind::Identifier, "next");
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnknownEscape));
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnterminatedStringLiteral));
      expectPartition(File);

      const std::string CrlfSource = "\"oops\\\r\nnext";
      const TokenizedBuffer CrlfFile = tokenize(CrlfSource);
      ASSERT_FALSE(CrlfFile.succeeded());
      ASSERT_EQ(CrlfFile.tokens().size(), 4U);
      expectToken(CrlfFile, 0, TokenKind::InvalidStringLiteral, "\"oops\\");
      expectToken(CrlfFile, 1, TokenKind::LineBreak, "\r\n");
      expectToken(CrlfFile, 2, TokenKind::Identifier, "next");
      EXPECT_TRUE(hasDiagnostic(CrlfFile, DiagnosticKind::UnknownEscape));
      EXPECT_FALSE(hasDiagnostic(CrlfFile, DiagnosticKind::LoneCarriageReturn));
      expectPartition(CrlfFile);
    }

    // Verifies that end of file before a closing quote produces an unterminated string token.
    TEST(StringLiteralTest, RejectsEofBeforeClosingQuote)
    {
      const std::string Source = "\"unterminated";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      expectToken(File, 0, TokenKind::InvalidStringLiteral, Source);
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnterminatedStringLiteral));
      expectPartition(File);
    }

    // Verifies that directly encoded invisible format characters are diagnosed in strings.
    TEST(StringLiteralTest, RejectsDirectInvisibleFormatCharacters)
    {
      const std::vector<std::string> InvisibleCharacters = {
          "\xE2\x80\x8B",
          "\xE2\x80\xAE",
          "\xEF\xB8\x8F",
      };

      for (const std::string &Invisible : InvisibleCharacters)
      {
        const std::string Source = std::string("\"a") + Invisible + "b\"";
        SCOPED_TRACE(Source);
        const TokenizedBuffer File = tokenize(Source);
        EXPECT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvisibleCharacter));
        expectPartition(File);
      }
    }

    // Verifies that escape recovery still reports directly encoded invisible characters.
    TEST(StringLiteralTest, EscapeRecoveryStillDiagnosesRawInvisibleCharacters)
    {
      const std::string Invisible = "\xE2\x80\x8C";
      const std::vector<std::string> Sources = {
          std::string("\"\\") + Invisible + "\"",
          std::string("\"\\u{") + Invisible + "}\"",
      };

      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const TokenizedBuffer File = tokenize(Source);
        EXPECT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvisibleCharacter));
        expectPartition(File);
      }
    }

    // Verifies that invisible values are allowed when represented through Unicode escapes.
    TEST(StringLiteralTest, AllowsInvisibleValuesThroughUnicodeEscapes)
    {
      const std::string Source = R"ink("\u{200B}\u{202E}\u{FE0F}")ink";
      const std::string Expected = "\xE2\x80\x8B\xE2\x80\xAE\xEF\xB8\x8F";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2U);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, Expected);
      expectPartition(File);
    }

    // Verifies that a raw control byte inside string source text is diagnosed.
    TEST(StringLiteralTest, RejectsRawControlCharacterInsideSourceText)
    {
      std::string Source = "\"before";
      Source.push_back('\0');
      Source += "after\"";
      const TokenizedBuffer File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::ForbiddenControlCharacter));
      expectPartition(File);
    }
  } // namespace
} // namespace ink::tokenizer
