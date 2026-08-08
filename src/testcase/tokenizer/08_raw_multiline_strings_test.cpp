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

    void expectPartition(const LexedFile &File)
    {
      ASSERT_FALSE(File.tokens().empty());
      std::size_t Cursor = 0;
      std::size_t EofCount = 0;
      std::string Rebuilt;
      for (const Token &CurrentToken : File.tokens())
      {
        if (CurrentToken.Kind == TokenKind::EndOfFile)
        {
          ++EofCount;
          EXPECT_EQ(CurrentToken.Span, (SourceRange{File.source().size(), File.source().size()}));
          EXPECT_TRUE(File.raw(CurrentToken).empty());
          continue;
        }
        EXPECT_EQ(CurrentToken.Span.Start, Cursor);
        EXPECT_EQ(CurrentToken.Span.size(), File.raw(CurrentToken).size());
        Rebuilt.append(File.raw(CurrentToken).data(), File.raw(CurrentToken).size());
        Cursor = CurrentToken.Span.End;
      }
      EXPECT_EQ(Cursor, File.source().size());
      EXPECT_EQ(Rebuilt, File.source());
      EXPECT_EQ(EofCount, 1u);
      EXPECT_EQ(File.tokens().back().Kind, TokenKind::EndOfFile);
    }

    void expectToken(const LexedFile &File, std::size_t Index, TokenKind Kind, std::string_view Raw)
    {
      ASSERT_LT(Index, File.tokens().size());
      EXPECT_EQ(File.tokens()[Index].Kind, Kind);
      EXPECT_EQ(std::string(File.raw(File.tokens()[Index])), std::string(Raw));
    }

    bool hasDiagnostic(const LexedFile &File, DiagnosticKind Kind)
    {
      return std::any_of(File.diagnostics().begin(), File.diagnostics().end(), [Kind](const Diagnostic &CurrentDiagnostic)
                         {
                           return CurrentDiagnostic.Kind == Kind;
                         });
    }

    const StringInfo &stringPayload(const Token &CurrentToken)
    {
      return std::get<StringInfo>(CurrentToken.Payload);
    }

    // Verifies that escaped/raw and single-line/multiline strings select distinct modes and decoded values.
    TEST(RawAndMultilineStringTest, DistinguishesAllFourStringModes)
    {
      const std::string Source = R"ink("line\n" r"line\n" """
  line\nnext
  """ r"""
  line\nnext
  """)ink";
      const LexedFile File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 8u);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Mode, StringMode::EscapedSingleLine);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, "line\n");
      EXPECT_EQ(stringPayload(File.tokens()[2]).Mode, StringMode::RawSingleLine);
      EXPECT_EQ(stringPayload(File.tokens()[2]).Decoded, "line\\n");
      EXPECT_EQ(stringPayload(File.tokens()[4]).Mode, StringMode::EscapedMultiline);
      EXPECT_EQ(stringPayload(File.tokens()[4]).Decoded, "line\nnext");
      EXPECT_EQ(stringPayload(File.tokens()[6]).Mode, StringMode::RawMultiline);
      EXPECT_EQ(stringPayload(File.tokens()[6]).Decoded, "line\\nnext");
      expectPartition(File);
    }

    // Verifies that only a lowercase adjacent r prefix enables raw-string tokenization.
    TEST(RawAndMultilineStringTest, RawPrefixMustBeLowercaseAndAdjacent)
    {
      const std::string Source = R"ink(r"raw" r "ordinary" raw"ordinary" R"ordinary")ink";
      const LexedFile File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 12u);
      expectToken(File, 0, TokenKind::StringLiteral, "r\"raw\"");
      EXPECT_EQ(stringPayload(File.tokens()[0]).Mode, StringMode::RawSingleLine);
      expectToken(File, 2, TokenKind::Identifier, "r");
      expectToken(File, 4, TokenKind::StringLiteral, "\"ordinary\"");
      expectToken(File, 6, TokenKind::Identifier, "raw");
      expectToken(File, 7, TokenKind::StringLiteral, "\"ordinary\"");
      expectToken(File, 9, TokenKind::Identifier, "R");
      expectToken(File, 10, TokenKind::StringLiteral, "\"ordinary\"");
      expectPartition(File);
    }

    // Verifies that raw single-line strings preserve backslashes and interpolation markers literally.
    TEST(RawAndMultilineStringTest, RawSingleLineDoesNotInterpretBackslashes)
    {
      const std::string Source = R"ink(r"C:\Users\Hello\file.txt" r"\d+\.\d+" r"\n" r"${value}")ink";
      const LexedFile File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 8u);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, R"ink(C:\Users\Hello\file.txt)ink");
      EXPECT_EQ(stringPayload(File.tokens()[2]).Decoded, R"ink(\d+\.\d+)ink");
      EXPECT_EQ(stringPayload(File.tokens()[4]).Decoded, R"ink(\n)ink");
      EXPECT_EQ(stringPayload(File.tokens()[6]).Decoded, "${value}");
      expectPartition(File);
    }

    // Verifies that a backslash has no escaping power before a raw string's closing quote.
    TEST(RawAndMultilineStringTest, BackslashCannotEscapeRawClosingQuote)
    {
      const std::string Source = R"ink(r"a\")ink";
      const LexedFile File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2u);
      expectToken(File, 0, TokenKind::StringLiteral, Source);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, "a\\");
      expectPartition(File);
    }

    // Verifies that multiline decoding removes the exact indentation of the closing delimiter.
    TEST(RawAndMultilineStringTest, TrimsExactMultilineIndentation)
    {
      const std::string Source = "\"\"\"\n\t first\n\t   second\n\t third\n\t \"\"\"";
      const LexedFile File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2u);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Mode, StringMode::EscapedMultiline);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, "first\n  second\nthird");
      expectPartition(File);
    }

    // Verifies that a column-zero closing delimiter preserves all body indentation.
    TEST(RawAndMultilineStringTest, EmptyPrefixPreservesBodyIndentation)
    {
      const std::string Source = "\"\"\"\n  first\n    second\n\"\"\"";
      const LexedFile File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2u);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, "  first\n    second");
      expectPartition(File);
    }

    // Verifies boundary-line exclusion for empty, populated, and trailing-line-break multiline strings.
    TEST(RawAndMultilineStringTest, ExcludesBoundaryLineBreaksAndSupportsTrailingLineBreak)
    {
      const std::string EmptySource = "\"\"\"\n    \"\"\"";
      const std::string TextSource = "\"\"\"\n    first\n    second\n    \"\"\"";
      const std::string TrailingSource = "\"\"\"\n    first\n    \n    \"\"\"";
      const LexedFile EmptyFile = tokenize(EmptySource);
      const LexedFile TextFile = tokenize(TextSource);
      const LexedFile TrailingFile = tokenize(TrailingSource);

      ASSERT_TRUE(EmptyFile.succeeded());
      ASSERT_TRUE(TextFile.succeeded());
      ASSERT_TRUE(TrailingFile.succeeded());
      EXPECT_EQ(stringPayload(EmptyFile.tokens()[0]).Decoded, "");
      EXPECT_EQ(stringPayload(TextFile.tokens()[0]).Decoded, "first\nsecond");
      EXPECT_EQ(stringPayload(TrailingFile.tokens()[0]).Decoded, "first\n");
      expectPartition(EmptyFile);
      expectPartition(TextFile);
      expectPartition(TrailingFile);
    }

    // Verifies how whitespace-only body lines are trimmed against the closing indentation.
    TEST(RawAndMultilineStringTest, HandlesWhitespaceOnlyLinesRelativeToClosingIndentation)
    {
      const std::string Source = "\"\"\"\n    first\n      \n   \n    \"\"\"";
      const LexedFile File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2u);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, "first\n  \n");
      expectPartition(File);
    }

    // Verifies that CRLF is normalized only in decoded multiline content while raw bytes remain intact.
    TEST(RawAndMultilineStringTest, NormalizesCrlfToLfOnlyInDecodedValue)
    {
      const std::string LfSource = "\"\"\"\n  first\n  second\n  \"\"\"";
      const std::string CrlfSource = "\"\"\"\r\n  first\r\n  second\r\n  \"\"\"";
      const LexedFile LfFile = tokenize(LfSource);
      const LexedFile CrlfFile = tokenize(CrlfSource);

      ASSERT_TRUE(LfFile.succeeded());
      ASSERT_TRUE(CrlfFile.succeeded());
      EXPECT_EQ(stringPayload(LfFile.tokens()[0]).Decoded, "first\nsecond");
      EXPECT_EQ(stringPayload(CrlfFile.tokens()[0]).Decoded, "first\nsecond");
      EXPECT_EQ(std::string(CrlfFile.raw(CrlfFile.tokens()[0])), CrlfSource);
      EXPECT_NE(std::string(LfFile.raw(LfFile.tokens()[0])), std::string(CrlfFile.raw(CrlfFile.tokens()[0])));
      expectPartition(LfFile);
      expectPartition(CrlfFile);
    }

    // Verifies that triple quotes in the middle of a body line remain raw multiline text.
    TEST(RawAndMultilineStringTest, MidlineTripleQuotesAreBodyText)
    {
      const std::string Source = R"ink(r"""
  before """ after
  """)ink";
      const LexedFile File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2u);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Mode, StringMode::RawMultiline);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, "before \"\"\" after");
      expectPartition(File);
    }

    // Verifies that syntax may begin immediately after a valid multiline closing delimiter.
    TEST(RawAndMultilineStringTest, ClosingDelimiterMayBeFollowedBySyntaxOnSameLine)
    {
      const std::string Source = "\"\"\"\n  value\n  \"\"\";";
      const LexedFile File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 3u);
      expectToken(File, 0, TokenKind::StringLiteral, "\"\"\"\n  value\n  \"\"\"");
      expectToken(File, 1, TokenKind::Symbol, ";");
      EXPECT_EQ(std::get<char>(File.tokens()[1].Payload), ';');
      expectPartition(File);
    }

    // Verifies that escaped multiline strings decode escapes after indentation removal.
    TEST(RawAndMultilineStringTest, EscapedMultilineProcessesEscapesAfterIndentation)
    {
      const std::string Source = R"ink("""
    tab:\tvalue
    quote:\"
    triple:\"\"\"
    emoji:\u{1F600}
    """)ink";
      const std::string Expected = std::string("tab:\tvalue\nquote:\"\ntriple:\"\"\"\nemoji:") + "\xF0\x9F\x98\x80";
      const LexedFile File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2u);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, Expected);
      expectPartition(File);
    }

    // Verifies that raw multiline strings retain escape, interpolation, and comment-looking text.
    TEST(RawAndMultilineStringTest, RawMultilineKeepsEscapesInterpolationAndCommentMarkersAsText)
    {
      const std::string Source = R"ink(r"""
  \n \u{200B} ${value} // /* text */
  """)ink";
      const LexedFile File = tokenize(Source);

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 2u);
      EXPECT_EQ(stringPayload(File.tokens()[0]).Decoded, R"ink(\n \u{200B} ${value} // /* text */)ink");
      expectPartition(File);
    }

    // Verifies that both escaped and raw triple-quoted strings require a line break after opening.
    TEST(RawAndMultilineStringTest, RejectsInlineTripleQuotedForms)
    {
      const std::vector<std::string> Sources = {
          R"ink("""inline""")ink",
          R"ink(r"""inline""")ink",
      };
      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const LexedFile File = tokenize(Source);
        EXPECT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::MultilineOpeningLineBreakRequired));
        expectPartition(File);
      }
    }

    // Verifies recovery from an inline opening by selecting a later delimiter on its own line.
    TEST(RawAndMultilineStringTest, InlineOpeningRecoveryPrefersANextLineClosingCandidate)
    {
      const std::string Source = "\"\"\"inline\nbody \"\"\" middle\n  \"\"\"; next";
      const LexedFile File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      ASSERT_GE(File.tokens().size(), 5u);
      expectToken(File, 0, TokenKind::InvalidStringLiteral, "\"\"\"inline\nbody \"\"\" middle\n  \"\"\"");
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::MultilineOpeningLineBreakRequired));
      EXPECT_FALSE(hasDiagnostic(File, DiagnosticKind::UnterminatedMultilineStringLiteral));
      expectToken(File, 1, TokenKind::Symbol, ";");
      expectPartition(File);
    }

    // Verifies that an unterminated raw single-line string stops at the physical line break and recovers.
    TEST(RawAndMultilineStringTest, StopsRawSingleLineAtPhysicalLineBreakAndRecovers)
    {
      const std::string Source = "r\"oops\nnext";
      const LexedFile File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 4u);
      expectToken(File, 0, TokenKind::InvalidStringLiteral, "r\"oops");
      expectToken(File, 1, TokenKind::LineBreak, "\n");
      expectToken(File, 2, TokenKind::Identifier, "next");
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnterminatedStringLiteral));
      expectPartition(File);
    }

    // Verifies diagnostics and exact partitioning when no multiline closing delimiter exists.
    TEST(RawAndMultilineStringTest, RejectsMissingMultilineClosingDelimiter)
    {
      const std::vector<std::string> Sources = {
          "\"\"\"\n  text",
          "r\"\"\"\n  text",
          "r\"\"\"\n  before \"\"\" after",
      };
      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const LexedFile File = tokenize(Source);
        EXPECT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnterminatedMultilineStringLiteral));
        expectPartition(File);
      }
    }

    // Verifies rejection when body indentation does not match the closing delimiter's exact prefix.
    TEST(RawAndMultilineStringTest, RejectsMismatchedBodyIndentation)
    {
      const std::vector<std::string> Sources = {
          "\"\"\"\n  too shallow\n    \"\"\"",
          "\"\"\"\n    spaces\n\t\"\"\"",
          "\"\"\"\n  \n\t \"\"\"",
      };
      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const LexedFile File = tokenize(Source);
        EXPECT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvalidMultilineIndentation));
        expectPartition(File);
      }
    }

    // Verifies that an escape cannot continue an escaped multiline string across a physical line break.
    TEST(RawAndMultilineStringTest, RejectsEscapeFollowedByPhysicalLineBreak)
    {
      const std::string Source = "\"\"\"\n  first\\\n  second\n  \"\"\"";
      const LexedFile File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnknownEscape));
      expectPartition(File);
    }

    // Verifies that unknown escapes are diagnosed inside escaped multiline strings.
    TEST(RawAndMultilineStringTest, RejectsUnknownEscapeInEscapedMultilineMode)
    {
      const std::string Source = R"ink("""
  bad:\q
  """)ink";
      const LexedFile File = tokenize(Source);

      ASSERT_FALSE(File.succeeded());
      EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::UnknownEscape));
      expectPartition(File);
    }

    // Verifies that directly embedded invisible Unicode characters remain forbidden in raw string modes.
    TEST(RawAndMultilineStringTest, RejectsDirectInvisibleCharacterInRawModes)
    {
      const std::string Invisible = "\xE2\x80\x8B";
      const std::vector<std::string> Sources = {
          std::string("r\"a") + Invisible + "b\"",
          std::string("r\"\"\"\n  a") + Invisible + "b\n  \"\"\"",
      };
      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const LexedFile File = tokenize(Source);
        EXPECT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, DiagnosticKind::InvisibleCharacter));
        expectPartition(File);
      }
    }

    // Verifies that raw string modes cannot conceal forbidden source control bytes.
    TEST(RawAndMultilineStringTest, RawModeCannotBypassForbiddenSourceControls)
    {
      std::string SingleSource = "r\"before";
      SingleSource.push_back('\0');
      SingleSource += "after\"";
      std::string MultilineSource = "r\"\"\"\n  before";
      MultilineSource.push_back('\x1B');
      MultilineSource += "after\n  \"\"\"";
      const LexedFile SingleFile = tokenize(SingleSource);
      const LexedFile MultilineFile = tokenize(MultilineSource);

      EXPECT_FALSE(SingleFile.succeeded());
      EXPECT_FALSE(MultilineFile.succeeded());
      EXPECT_TRUE(hasDiagnostic(SingleFile, DiagnosticKind::ForbiddenControlCharacter));
      EXPECT_TRUE(hasDiagnostic(MultilineFile, DiagnosticKind::ForbiddenControlCharacter));
      expectPartition(SingleFile);
      expectPartition(MultilineFile);
    }
  } // namespace
} // namespace ink::tokenizer
