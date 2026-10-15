#include "ink/core/diagnostic/diagnostic_builder.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/source/source_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <string_view>

namespace ink::core::test
{
  // Source-aware ICE output retains its location and excerpt before termination without allowing consumer interception.
  TEST(DiagnosticFormatterDeathTest, PrintsSourceBeforePanickingWithoutDispatching)
  {
    class SuppressingConsumer final : public DiagnosticConsumer
    {
      public:
        void consume(const Diagnostic &) override
        {
          std::_Exit(0);
        }
    };
    SourceManager Sources;
    const SourceId Source = Sources.addSource("fatal.ink", "ok\n!");
    DiagnosticEngine Engine(Sources);
    SuppressingConsumer Consumer;
    Engine.addConsumer(Consumer);
    auto Entry = makeDiagnostic<DiagnosticKind::SemanticUnsupported>(SourceRange::fromByteOffsets(3, 4), "SimpleStmt");
    Entry.Source = Source;

    EXPECT_DEATH(Engine.report(Entry), ::testing::AllOf(::testing::HasSubstr("fatal.ink:2:1: internal compiler error[INK-S0012]: initial semantic analyzer does not support SimpleStmt"), ::testing::HasSubstr("[bytes 3, 4)"), ::testing::HasSubstr("2 | !"), ::testing::HasSubstr("^")));
  }

  // Source identity, one-based byte coordinates and half-open offsets describe the diagnostic's registered file.
  TEST(DiagnosticFormatterTest, RendersRegisteredSourceAndByteRange)
  {
    SourceManager Sources;
    Sources.addSource("unrelated.ink", "not the diagnostic source");
    const SourceId Source = Sources.addSource("actual.ink", "first\nalpha beta\n");
    auto Entry = makeDiagnostic<DiagnosticKind::ParserExpectedToken>(SourceRange::fromByteOffsets(8, 11), ";");
    Entry.Source = Source;

    const std::string Text = DiagnosticFormatter{}.format(Entry, Sources);
    EXPECT_NE(Text.find("actual.ink:2:3: error[INK-P0001]: expected ;"), std::string::npos);
    EXPECT_NE(Text.find("[bytes 8, 11)"), std::string::npos);
    EXPECT_NE(Text.find("2 | alpha beta"), std::string::npos);
    EXPECT_NE(Text.find("^~~"), std::string::npos);
    EXPECT_EQ(Text.find("unrelated.ink"), std::string::npos);
    EXPECT_EQ(Text.find("1 | first"), std::string::npos);
  }

  // LF, CRLF and CR each advance one source line without leaking carriage returns into rendered excerpts.
  TEST(DiagnosticFormatterTest, RecognizesAllSupportedLineEndings)
  {
    constexpr std::array<std::string_view, 3> Endings = {"\n", "\r\n", "\r"};
    for (const std::string_view Ending : Endings)
    {
      SCOPED_TRACE(std::string(Ending));
      SourceManager Sources;
      const std::string SourceText = "before" + std::string(Ending) + "target" + std::string(Ending) + "after";
      const SourceId Source = Sources.addSource("lines.ink", SourceText);
      const std::size_t Begin = 6 + Ending.size();
      auto Entry = makeDiagnostic<DiagnosticKind::InvalidCharacter>(SourceRange::fromByteOffsets(Begin, Begin + 3));
      Entry.Source = Source;

      const std::string Text = DiagnosticFormatter{}.format(Entry, Sources);
      EXPECT_NE(Text.find("lines.ink:2:1:"), std::string::npos);
      EXPECT_NE(Text.find("2 | target"), std::string::npos);
      EXPECT_NE(Text.find("^~~"), std::string::npos);
      EXPECT_EQ(Text.find("3 | after"), std::string::npos);
      EXPECT_EQ(Text.find('\r'), std::string::npos);
    }
  }

  // A multiline range includes its partially covered last line but excludes a line beginning exactly at End.
  TEST(DiagnosticFormatterTest, RendersOnlyLinesCoveredByHalfOpenRange)
  {
    SourceManager Sources;
    const SourceId Source = Sources.addSource("range.ink", "abc\nmiddle\nlast\nuntouched");
    auto Entry = makeDiagnostic<DiagnosticKind::InvalidCharacter>(SourceRange::fromByteOffsets(1, 14));
    Entry.Source = Source;

    const std::string AcrossLines = DiagnosticFormatter{}.format(Entry, Sources);
    EXPECT_NE(AcrossLines.find("range.ink:1:2:"), std::string::npos);
    EXPECT_NE(AcrossLines.find("1 | abc"), std::string::npos);
    EXPECT_NE(AcrossLines.find("2 | middle"), std::string::npos);
    EXPECT_NE(AcrossLines.find("3 | last"), std::string::npos);
    EXPECT_EQ(AcrossLines.find("4 | untouched"), std::string::npos);

    Entry.Span = SourceRange::fromByteOffsets(1, 11);
    const std::string AtLineBoundary = DiagnosticFormatter{}.format(Entry, Sources);
    EXPECT_NE(AtLineBoundary.find("[bytes 1, 11)"), std::string::npos);
    EXPECT_NE(AtLineBoundary.find("2 | middle"), std::string::npos);
    EXPECT_EQ(AtLineBoundary.find("3 | last"), std::string::npos);
  }

  // Insertion points at an empty file, an ordinary EOF and the empty line after a final newline retain a caret.
  TEST(DiagnosticFormatterTest, RendersPointAndEofLocations)
  {
    struct Case
    {
        std::string_view Source;
        std::string_view Location;
    };
    constexpr Case Cases[] = {
        {"", "point.ink:1:1:"},
        {"abc", "point.ink:1:4:"},
        {"abc\n", "point.ink:2:1:"},
    };
    for (const Case &Value : Cases)
    {
      SCOPED_TRACE(std::string(Value.Location));
      SourceManager Sources;
      const SourceId Source = Sources.addSource("point.ink", std::string(Value.Source));
      auto Entry = makeDiagnostic<DiagnosticKind::ParserExpectedToken>(SourceRange::fromByteOffsets(Value.Source.size(), Value.Source.size()), ";");
      Entry.Source = Source;

      const std::string Text = DiagnosticFormatter{}.format(Entry, Sources);
      EXPECT_NE(Text.find(Value.Location), std::string::npos);
      EXPECT_NE(Text.find('^'), std::string::npos);
      EXPECT_EQ(Text.find('~'), std::string::npos);
    }
  }

  // A missing or unknown source preserves byte offsets without fabricating a filename, line or excerpt.
  TEST(DiagnosticFormatterTest, MissingSourceKeepsBytesWithoutCoordinates)
  {
    SourceManager Sources;
    Sources.addSource("unrelated.ink", "abcdef");
    const SourceId MissingSources[] = {SourceId{}, SourceId(std::numeric_limits<std::uint64_t>::max())};
    for (const SourceId Source : MissingSources)
    {
      auto Entry = makeDiagnostic<DiagnosticKind::ParserExpectedToken>(SourceRange::fromByteOffsets(2, 5), ";");
      Entry.Source = Source;

      const std::string Text = DiagnosticFormatter{}.format(Entry, Sources);
      EXPECT_NE(Text.find("<unknown>: error[INK-P0001]: expected ;"), std::string::npos);
      EXPECT_NE(Text.find("[bytes 2, 5)"), std::string::npos);
      EXPECT_EQ(Text.find("<unknown>:1:"), std::string::npos);
      EXPECT_EQ(Text.find("unrelated.ink"), std::string::npos);
      EXPECT_EQ(Text.find(" | "), std::string::npos);
    }
  }

  // Invalid, reversed and out-of-bounds ranges keep the diagnostic message without inventing valid coordinates.
  TEST(DiagnosticFormatterTest, RejectsUnusableRangesForSourceRendering)
  {
    SourceManager Sources;
    const SourceId Source = Sources.addSource("bounds.ink", "abc");
    const SourceRange Ranges[] = {
        {},
        SourceRange::fromByteOffsets(2, 1),
        SourceRange::fromByteOffsets(1, 9),
        SourceRange::fromByteOffsets(9, 9),
    };
    for (const SourceRange Span : Ranges)
    {
      auto Entry = makeDiagnostic<DiagnosticKind::ParserExpectedToken>(Span, ";");
      Entry.Source = Source;

      const std::string Text = DiagnosticFormatter{}.format(Entry, Sources);
      EXPECT_NE(Text.find("bounds.ink: error[INK-P0001]: expected ;"), std::string::npos);
      EXPECT_EQ(Text.find("bounds.ink:1:"), std::string::npos);
      EXPECT_EQ(Text.find(" | "), std::string::npos);
    }
  }

  // Tabs expand to eight-column stops while diagnostic coordinates continue to count original source bytes.
  TEST(DiagnosticFormatterTest, AlignsCaretAfterTabExpansion)
  {
    SourceManager Sources;
    const SourceId Source = Sources.addSource("tabs.ink", "a\tb");
    auto Entry = makeDiagnostic<DiagnosticKind::InvalidCharacter>(SourceRange::fromByteOffsets(2, 3));
    Entry.Source = Source;

    const std::string Text = DiagnosticFormatter{}.format(Entry, Sources);
    EXPECT_NE(Text.find("tabs.ink:1:3:"), std::string::npos);
    EXPECT_NE(Text.find("1 | a       b"), std::string::npos);
    EXPECT_NE(Text.find("|         ^"), std::string::npos);
    EXPECT_EQ(Text.find('\t'), std::string::npos);
  }

  // Non-ASCII and invalid UTF-8 bytes are escaped individually and highlights use their expanded display widths.
  TEST(DiagnosticFormatterTest, EscapesUtf8BytesAndPreservesByteColumns)
  {
    SourceManager Sources;
    std::string SourceText = "a";
    SourceText.push_back(static_cast<char>(0xC3));
    SourceText.push_back(static_cast<char>(0xA9));
    SourceText.push_back(static_cast<char>(0xFF));
    SourceText.push_back('b');
    const SourceId Source = Sources.addSource("bytes.ink", SourceText);
    auto Entry = makeDiagnostic<DiagnosticKind::InvalidUtf8>(SourceRange::fromByteOffsets(3, 4));
    Entry.Source = Source;

    const std::string Text = DiagnosticFormatter{}.format(Entry, Sources);
    EXPECT_NE(Text.find("bytes.ink:1:4:"), std::string::npos);
    EXPECT_NE(Text.find("[bytes 3, 4)"), std::string::npos);
    EXPECT_NE(Text.find("a\\xC3\\xA9\\xFFb"), std::string::npos);
    EXPECT_NE(Text.find("|          ^~~~"), std::string::npos);
    EXPECT_EQ(Text.find(static_cast<char>(0xC3)), std::string::npos);
    EXPECT_EQ(Text.find(static_cast<char>(0xFF)), std::string::npos);
  }

  // Related comment and visible-character records become explanatory notes while retaining their original spans.
  TEST(DiagnosticFormatterTest, PreservesRelatedNotesAndTheirSourceLocations)
  {
    SourceManager Sources;
    const SourceId Source = Sources.addSource("notes.ink", "a /* open\n!");
    const SourceRange CommentSpan = SourceRange::fromByteOffsets(2, 4);
    const SourceRange CharacterSpan = SourceRange::fromByteOffsets(0, 1);
    auto Entry = makeDiagnosticBuilder<DiagnosticKind::UnterminatedBlockComment>(SourceRange::fromByteOffsets(10, 11)).related(DiagnosticRelatedKind::MostRecentUnclosedBlockComment, CommentSpan).related(DiagnosticRelatedKind::PreviousVisibleCharacter, CharacterSpan, {{DiagnosticArgumentName::Character, U'a'}}).build();
    Entry.Source = Source;

    const DiagnosticFormatter Formatter;
    const FormattedDiagnostic Formatted = Formatter.format(Entry);
    ASSERT_EQ(Formatted.Notes.size(), 2U);
    EXPECT_EQ(Formatted.Notes[0].Span, CommentSpan);
    EXPECT_EQ(Formatted.Notes[1].Span, CharacterSpan);
    EXPECT_FALSE(Formatted.Notes[0].Message.empty());
    EXPECT_FALSE(Formatted.Notes[1].Message.empty());
    EXPECT_NE(Formatted.Notes[0].Message, Formatted.Notes[1].Message);

    const std::string Text = Formatter.format(Entry, Sources);
    EXPECT_NE(Text.find("notes.ink:2:1: error[INK-T0019]"), std::string::npos);
    EXPECT_NE(Text.find("notes.ink:1:3: note:"), std::string::npos);
    EXPECT_NE(Text.find("notes.ink:1:1: note:"), std::string::npos);
    EXPECT_NE(Text.find(Formatted.Notes[0].Message), std::string::npos);
    EXPECT_NE(Text.find(Formatted.Notes[1].Message), std::string::npos);
    EXPECT_NE(Text.find("[bytes 2, 4)"), std::string::npos);
    EXPECT_NE(Text.find("[bytes 0, 1)"), std::string::npos);
  }

  // Related information without a usable location remains visible without borrowing the primary diagnostic's span.
  TEST(DiagnosticFormatterTest, RendersRelatedNoteWithoutInventingItsLocation)
  {
    SourceManager Sources;
    const SourceId Source = Sources.addSource("notes.ink", "/*");
    auto Entry = makeDiagnosticBuilder<DiagnosticKind::UnterminatedBlockComment>(SourceRange::fromByteOffsets(0, 2)).related(DiagnosticRelatedKind::MostRecentBlockCommentOpeningUnavailable, {}).build();
    Entry.Source = Source;

    const DiagnosticFormatter Formatter;
    const FormattedDiagnostic Formatted = Formatter.format(Entry);
    ASSERT_EQ(Formatted.Notes.size(), 1U);
    EXPECT_TRUE(!Formatted.Notes[0].Span || Formatted.Notes[0].Span->isInvalid());
    EXPECT_FALSE(Formatted.Notes[0].Message.empty());
    const std::string Text = Formatter.format(Entry, Sources);
    EXPECT_NE(Text.find(Formatted.Notes[0].Message), std::string::npos);
    EXPECT_EQ(Text.find("notes.ink:1:1: note:"), std::string::npos);
  }

  // An explicitly promoted diagnostic retains its effective ICE classification in the source-aware text overload.
  TEST(DiagnosticFormatterTest, RendersEffectiveInternalErrorClassification)
  {
    SourceManager Sources;
    auto Entry = makeDiagnosticBuilder<DiagnosticKind::ParserExpectedToken>(SourceRange::fromByteOffsets(0, 1), ";").classification(DiagnosticClass::InternalCompilerError).build();
    Entry.Source = Sources.addSource("ice.ink", "!");

    const std::string Text = DiagnosticFormatter{}.format(Entry, Sources);
    EXPECT_NE(Text.find("ice.ink:1:1: internal compiler error[INK-P0001]: expected ;"), std::string::npos);
    EXPECT_NE(Text.find("1 | !"), std::string::npos);
    EXPECT_NE(Text.find('^'), std::string::npos);
  }
} // namespace ink::core::test
