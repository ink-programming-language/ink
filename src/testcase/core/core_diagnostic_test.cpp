#include "ink/core/diagnostic.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace ink::core
{
  namespace
  {
    struct DiagnosticExpectation
    {
      DiagnosticKind Kind;
      std::uint32_t Number;
      DiagnosticDomain Domain;
      DiagnosticSeverity Severity;
      const char *Code;
      const char *Name;
      const char *DefaultMessage;
    };

    constexpr DiagnosticExpectation DiagnosticExpectations[] = {
        {DiagnosticKind::Unknown, 0x00000000U, DiagnosticDomain::Unknown, DiagnosticSeverity::Unknown, "INK-0000", "Unknown", "unknown diagnostic"},
        {DiagnosticKind::InvalidUtf8, 0x01000001U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0001", "InvalidUtf8", "invalid UTF-8"},
        {DiagnosticKind::UnexpectedBom, 0x01000002U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0002", "UnexpectedBom", "UTF-8 BOM is only allowed at the start of a file"},
        {DiagnosticKind::LoneCarriageReturn, 0x01000003U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0003", "LoneCarriageReturn", "carriage return must be followed by line feed"},
        {DiagnosticKind::NonAsciiWhitespace, 0x01000004U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0004", "NonAsciiWhitespace", "only ASCII space and tab are source whitespace"},
        {DiagnosticKind::ForbiddenControlCharacter, 0x01000005U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0005", "ForbiddenControlCharacter", "forbidden raw control character"},
        {DiagnosticKind::InvalidCharacter, 0x01000006U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0006", "InvalidCharacter", "character cannot start an Ink token"},
        {DiagnosticKind::IdentifierNotNfc, 0x01000007U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0007", "IdentifierNotNfc", "identifier is not in Unicode NFC"},
        {DiagnosticKind::InvisibleCharacter, 0x01000008U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0008", "InvisibleCharacter", "invisible format character must be written explicitly"},
        {DiagnosticKind::MissingBaseDigits, 0x01000009U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0009", "MissingBaseDigits", "base prefix must be followed by a digit"},
        {DiagnosticKind::DigitOutOfRange, 0x0100000AU, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0010", "DigitOutOfRange", "digit does not belong to the literal base"},
        {DiagnosticKind::MisplacedNumericSeparator, 0x0100000BU, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0011", "MisplacedNumericSeparator", "numeric separator must be between two digits"},
        {DiagnosticKind::MissingExponentDigits, 0x0100000CU, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0012", "MissingExponentDigits", "exponent must contain a decimal digit"},
        {DiagnosticKind::UnknownNumericSuffix, 0x0100000DU, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0013", "UnknownNumericSuffix", "unknown numeric literal suffix"},
        {DiagnosticKind::InvalidNumericSuffix, 0x0100000EU, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0014", "InvalidNumericSuffix", "numeric suffix is not valid for this literal"},
        {DiagnosticKind::UnsupportedNonDecimalFloat, 0x0100000FU, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0015", "UnsupportedNonDecimalFloat", "non-decimal floating-point literals are not supported"},
        {DiagnosticKind::EmptyScalarLiteral, 0x01000010U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0016", "EmptyScalarLiteral", "scalar literal is empty"},
        {DiagnosticKind::MultipleScalarValues, 0x01000011U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0017", "MultipleScalarValues", "scalar literal must contain exactly one Unicode scalar value"},
        {DiagnosticKind::UnterminatedScalarLiteral, 0x01000012U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0018", "UnterminatedScalarLiteral", "scalar literal is not terminated on this source line"},
        {DiagnosticKind::UnknownEscape, 0x01000013U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0019", "UnknownEscape", "unknown escape sequence"},
        {DiagnosticKind::InvalidHexEscape, 0x01000014U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0020", "InvalidHexEscape", "hex escape requires exactly two hexadecimal digits"},
        {DiagnosticKind::InvalidUnicodeEscape, 0x01000015U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0021", "InvalidUnicodeEscape", "Unicode escape requires one to six hexadecimal digits in braces"},
        {DiagnosticKind::InvalidUnicodeScalar, 0x01000016U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0022", "InvalidUnicodeScalar", "escape does not designate a Unicode scalar value"},
        {DiagnosticKind::UnterminatedStringLiteral, 0x01000017U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0023", "UnterminatedStringLiteral", "single-line string is not terminated on this source line"},
        {DiagnosticKind::MultilineOpeningLineBreakRequired, 0x01000018U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0024", "MultilineOpeningLineBreakRequired", "multiline string opening delimiter must be followed by a line break"},
        {DiagnosticKind::UnterminatedMultilineStringLiteral, 0x01000019U, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0025", "UnterminatedMultilineStringLiteral", "multiline string has no closing delimiter"},
        {DiagnosticKind::InvalidMultilineIndentation, 0x0100001AU, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0026", "InvalidMultilineIndentation", "multiline string line does not match the closing indentation"},
        {DiagnosticKind::UnterminatedBlockComment, 0x0100001BU, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0027", "UnterminatedBlockComment", "block comment is not terminated"},
        {DiagnosticKind::BlockCommentNestingLimit, 0x0100001CU, DiagnosticDomain::Tokenizer, DiagnosticSeverity::Error, "INK-T0028", "BlockCommentNestingLimit", "block comment nesting limit exceeded"},
        {DiagnosticKind::UnexpectedToken, 0x02000001U, DiagnosticDomain::Parser, DiagnosticSeverity::Error, "INK-P0001", "UnexpectedToken", "unexpected token"},
        {DiagnosticKind::ExpectedToken, 0x02000002U, DiagnosticDomain::Parser, DiagnosticSeverity::Error, "INK-P0002", "ExpectedToken", "expected token"},
        {DiagnosticKind::ExpectedSyntax, 0x02000003U, DiagnosticDomain::Parser, DiagnosticSeverity::Error, "INK-P0003", "ExpectedSyntax", "expected syntax"},
        {DiagnosticKind::ReservedSymbolSequence, 0x02000004U, DiagnosticDomain::Parser, DiagnosticSeverity::Error, "INK-P0004", "ReservedSymbolSequence", "reserved symbol sequence is not valid here"},
        {DiagnosticKind::TrailingComma, 0x02000005U, DiagnosticDomain::Parser, DiagnosticSeverity::Error, "INK-P0005", "TrailingComma", "trailing comma is not allowed here"},
        {DiagnosticKind::DeclarationRequiresBlock, 0x02000006U, DiagnosticDomain::Parser, DiagnosticSeverity::Error, "INK-P0006", "DeclarationRequiresBlock", "a declaration in this position must be enclosed in a statement block"},
        {DiagnosticKind::SyntaxNestingLimit, 0x02000007U, DiagnosticDomain::Parser, DiagnosticSeverity::Error, "INK-P0007", "SyntaxNestingLimit", "syntax nesting limit exceeded"},
    };

    constexpr DiagnosticKind RegisteredDiagnosticKinds[] = {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, DefaultSeverity, DefaultMessage) DiagnosticKind::Name,
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    };

    constexpr std::size_t DiagnosticExpectationCount = sizeof(DiagnosticExpectations) / sizeof(DiagnosticExpectations[0]);
    constexpr std::size_t RegisteredDiagnosticKindCount = sizeof(RegisteredDiagnosticKinds) / sizeof(RegisteredDiagnosticKinds[0]);

    static_assert(std::is_aggregate<Diagnostic>::value, "Diagnostic must remain an aggregate");

    // Verifies that SourceRange models a half-open source-byte range.
    TEST(SourceRangeTest, ReportsSizeAndEmptyState)
    {
      constexpr SourceRange Range{4, 9};
      constexpr SourceRange EmptyRange{7, 7};

      EXPECT_EQ(Range.size(), 5u);
      EXPECT_FALSE(Range.empty());
      EXPECT_EQ(EmptyRange.size(), 0u);
      EXPECT_TRUE(EmptyRange.empty());
    }

    // Verifies that a default diagnostic has unknown metadata, an empty range, and no structured payload.
    TEST(DiagnosticTest, DefaultsToUnknownWithoutStructuredPayload)
    {
      const Diagnostic Value;
      const DiagnosticArgument Argument;
      const DiagnosticRelatedInformation Related;

      EXPECT_EQ(Value.Kind, DiagnosticKind::Unknown);
      EXPECT_EQ(Value.Span, (SourceRange{}));
      EXPECT_TRUE(Value.Arguments.empty());
      EXPECT_TRUE(Value.Related.empty());
      EXPECT_EQ(Value.number(), 0U);
      EXPECT_STREQ(Value.code(), "INK-0000");
      EXPECT_EQ(Argument.Name, DiagnosticArgumentName::Unknown);
      EXPECT_TRUE(std::holds_alternative<bool>(Argument.Value));
      EXPECT_FALSE(std::get<bool>(Argument.Value));
      EXPECT_EQ(Related.Kind, DiagnosticRelatedKind::Unknown);
      EXPECT_EQ(Related.Span, (SourceRange{}));
      EXPECT_TRUE(Related.Arguments.empty());
    }

    // Verifies that diagnostic equality compares argument values and related information rather than a rendered message.
    TEST(DiagnosticTest, ComparesStructuredValues)
    {
      const Diagnostic Left = DiagnosticBuilder(DiagnosticKind::InvisibleCharacter, {2, 5}).argument(DiagnosticArgumentName::Character, U'\u200B').related(DiagnosticRelatedKind::PreviousVisibleCharacter, {1, 2}, {{DiagnosticArgumentName::Character, U'a'}}).build();
      const Diagnostic Equal = DiagnosticBuilder(DiagnosticKind::InvisibleCharacter, {2, 5}).argument(DiagnosticArgumentName::Character, U'\u200B').related(DiagnosticRelatedKind::PreviousVisibleCharacter, {1, 2}, {{DiagnosticArgumentName::Character, U'a'}}).build();
      const Diagnostic DifferentArgument = DiagnosticBuilder(DiagnosticKind::InvisibleCharacter, {2, 5}).argument(DiagnosticArgumentName::Character, U'\u200C').related(DiagnosticRelatedKind::PreviousVisibleCharacter, {1, 2}, {{DiagnosticArgumentName::Character, U'a'}}).build();
      const Diagnostic DifferentRelated = DiagnosticBuilder(DiagnosticKind::InvisibleCharacter, {2, 5}).argument(DiagnosticArgumentName::Character, U'\u200B').related(DiagnosticRelatedKind::NextVisibleCharacter, {5, 6}, {{DiagnosticArgumentName::Character, U'b'}}).build();

      EXPECT_EQ(Left, Equal);
      EXPECT_NE(Left, DifferentArgument);
      EXPECT_NE(Left, DifferentRelated);
    }

    // Verifies every registered diagnostic kind against its stable metadata and default severity.
    TEST(DiagnosticTest, ExposesEveryStableMapping)
    {
      ASSERT_EQ(RegisteredDiagnosticKindCount, DiagnosticExpectationCount);
      for (std::size_t Index = 0; Index < DiagnosticExpectationCount; ++Index)
      {
        const DiagnosticExpectation &Expectation = DiagnosticExpectations[Index];

        EXPECT_EQ(RegisteredDiagnosticKinds[Index], Expectation.Kind);
        EXPECT_EQ(static_cast<std::uint32_t>(Expectation.Kind), Expectation.Number);
        EXPECT_EQ(diagnosticNumber(Expectation.Kind), Expectation.Number);
        EXPECT_STREQ(diagnosticCode(Expectation.Kind), Expectation.Code);
        EXPECT_STREQ(diagnosticKindName(Expectation.Kind), Expectation.Name);
        EXPECT_STREQ(diagnosticDefaultMessage(Expectation.Kind), Expectation.DefaultMessage);
        EXPECT_EQ(diagnosticDomain(Expectation.Kind), Expectation.Domain);
        EXPECT_EQ(diagnosticDefaultSeverity(Expectation.Kind), Expectation.Severity);
        EXPECT_EQ(static_cast<std::uint32_t>(Expectation.Domain), Expectation.Number >> 24U);
      }
    }

    // Verifies that no registered diagnostic kind reuses another kind's stable global number or public code.
    TEST(DiagnosticTest, KeepsNumbersAndCodesUnique)
    {
      std::set<std::uint32_t> Numbers;
      std::set<std::string_view> Codes;
      for (const DiagnosticExpectation &Expectation : DiagnosticExpectations)
      {
        EXPECT_TRUE(Numbers.insert(Expectation.Number).second) << Expectation.Name;
        EXPECT_TRUE(Codes.insert(Expectation.Code).second) << Expectation.Name;
      }
    }

    // Verifies public names for every severity and the defensive fallback for an invalid severity value.
    TEST(DiagnosticTest, NamesEverySeverityAndFallsBackForInvalidValues)
    {
      EXPECT_STREQ(diagnosticSeverityName(DiagnosticSeverity::Unknown), "unknown");
      EXPECT_STREQ(diagnosticSeverityName(DiagnosticSeverity::Error), "error");
      EXPECT_STREQ(diagnosticSeverityName(DiagnosticSeverity::Warning), "warning");
      EXPECT_STREQ(diagnosticSeverityName(DiagnosticSeverity::Note), "note");
      EXPECT_STREQ(diagnosticSeverityName(static_cast<DiagnosticSeverity>(0xFF)), "unknown");
    }

    // Verifies that an invalid diagnostic kind consistently falls back to reserved unknown metadata and formatting.
    TEST(DiagnosticTest, FallsBackToUnknownMetadataForInvalidKinds)
    {
      constexpr DiagnosticKind InvalidKind = static_cast<DiagnosticKind>(0xFFFFFFFFU);
      const Diagnostic Value{InvalidKind, {1, 2}, {}, {}};
      const FormattedDiagnostic Formatted = DiagnosticFormatter().format(Value);

      EXPECT_EQ(diagnosticNumber(InvalidKind), 0U);
      EXPECT_STREQ(diagnosticCode(InvalidKind), "INK-0000");
      EXPECT_STREQ(diagnosticKindName(InvalidKind), "Unknown");
      EXPECT_STREQ(diagnosticDefaultMessage(InvalidKind), "unknown diagnostic");
      EXPECT_EQ(diagnosticDomain(InvalidKind), DiagnosticDomain::Unknown);
      EXPECT_EQ(diagnosticDefaultSeverity(InvalidKind), DiagnosticSeverity::Unknown);
      EXPECT_EQ(Value.number(), 0U);
      EXPECT_STREQ(Value.code(), "INK-0000");
      EXPECT_EQ(Formatted, (FormattedDiagnostic{DiagnosticSeverity::Unknown, "unknown diagnostic", {}}));
    }

    // Verifies that the builder appends typed arguments and related source information without rendering text.
    TEST(DiagnosticTest, BuilderCreatesStructuredDiagnostics)
    {
      DiagnosticBuilder Builder(DiagnosticKind::UnterminatedBlockComment, {4, 6});
      Builder.argument(DiagnosticArgumentName::RemainingNestingDepth, std::uint64_t{3});
      Builder.related(DiagnosticRelatedKind::MostRecentUnclosedBlockComment, {10, 12});
      const Diagnostic Value = std::move(Builder).build();

      ASSERT_EQ(Value.Arguments.size(), 1u);
      EXPECT_EQ(Value.Arguments[0], (DiagnosticArgument{DiagnosticArgumentName::RemainingNestingDepth, std::uint64_t{3}}));
      ASSERT_EQ(Value.Related.size(), 1u);
      EXPECT_EQ(Value.Related[0], (DiagnosticRelatedInformation{DiagnosticRelatedKind::MostRecentUnclosedBlockComment, {10, 12}, {}}));
    }

    // Verifies that ordinary diagnostics format their registered default severity and message without notes.
    TEST(DiagnosticFormatterTest, FormatsOrdinaryDiagnosticsFromRegisteredDefaults)
    {
      const Diagnostic Value{DiagnosticKind::InvalidUtf8, {2, 4}, {}, {}};

      EXPECT_EQ(DiagnosticFormatter().format(Value), (FormattedDiagnostic{DiagnosticSeverity::Error, "invalid UTF-8", {}}));
    }

    // Verifies that Parser diagnostics append a typed Expected string to their registered default messages.
    TEST(DiagnosticFormatterTest, FormatsParserExpectedStringArguments)
    {
      const Diagnostic ExpectedTokenValue = DiagnosticBuilder(DiagnosticKind::ExpectedToken, {8, 8}).argument(DiagnosticArgumentName::Expected, std::string(")")).build();
      const Diagnostic ExpectedSyntaxValue = DiagnosticBuilder(DiagnosticKind::ExpectedSyntax, {12, 12}).argument(DiagnosticArgumentName::Expected, std::string("expression")).build();

      EXPECT_EQ(DiagnosticFormatter().format(ExpectedTokenValue), (FormattedDiagnostic{DiagnosticSeverity::Error, "expected token ')'", {}}));
      EXPECT_EQ(DiagnosticFormatter().format(ExpectedSyntaxValue), (FormattedDiagnostic{DiagnosticSeverity::Error, "expected syntax 'expression'", {}}));
    }

    // Verifies that Parser diagnostics append a typed Actual string without allowing producers to replace the registered message.
    TEST(DiagnosticFormatterTest, FormatsParserActualStringArguments)
    {
      const Diagnostic UnexpectedTokenValue = DiagnosticBuilder(DiagnosticKind::UnexpectedToken, {3, 4}).argument(DiagnosticArgumentName::Actual, std::string("}")).build();
      const Diagnostic ReservedSymbolValue = DiagnosticBuilder(DiagnosticKind::ReservedSymbolSequence, {6, 8}).argument(DiagnosticArgumentName::Actual, std::string("++")).build();

      EXPECT_EQ(DiagnosticFormatter().format(UnexpectedTokenValue), (FormattedDiagnostic{DiagnosticSeverity::Error, "unexpected token '}'", {}}));
      EXPECT_EQ(DiagnosticFormatter().format(ReservedSymbolValue), (FormattedDiagnostic{DiagnosticSeverity::Error, "reserved symbol sequence is not valid here '++'", {}}));
    }

    // Verifies that an unterminated block comment formats its remaining depth and most recent opening as a located note.
    TEST(DiagnosticFormatterTest, FormatsUnterminatedBlockCommentWithRelatedOpening)
    {
      const Diagnostic Value = DiagnosticBuilder(DiagnosticKind::UnterminatedBlockComment, {0, 2}).argument(DiagnosticArgumentName::RemainingNestingDepth, std::uint64_t{2}).related(DiagnosticRelatedKind::MostRecentUnclosedBlockComment, {8, 10}).build();
      const FormattedDiagnostic Expected{DiagnosticSeverity::Error, "block comment is not terminated; remaining nesting depth: 2", {{SourceRange{8, 10}, "most recent unclosed block comment opening is here"}}};

      EXPECT_EQ(DiagnosticFormatter().format(Value), Expected);
    }

    // Verifies that unavailable block-comment opening history becomes an unlocated explanatory note.
    TEST(DiagnosticFormatterTest, FormatsUnavailableBlockCommentOpeningAsAnUnlocatedNote)
    {
      const Diagnostic Value = DiagnosticBuilder(DiagnosticKind::UnterminatedBlockComment, {0, 2}).argument(DiagnosticArgumentName::RemainingNestingDepth, std::uint64_t{4096}).argument(DiagnosticArgumentName::MostRecentOpeningUnavailable, true).build();
      const FormattedDiagnostic Expected{DiagnosticSeverity::Error, "block comment is not terminated; remaining nesting depth: 4096", {{std::nullopt, "most recent unclosed opening was not retained after the nesting limit was exceeded"}}};

      EXPECT_EQ(DiagnosticFormatter().format(Value), Expected);
    }

    // Verifies that invisible-character formatting uses structured context and renders adjacent characters as located notes.
    TEST(DiagnosticFormatterTest, FormatsInvisibleCharacterContextAndAdjacentCharacterNotes)
    {
      const Diagnostic Value = DiagnosticBuilder(DiagnosticKind::InvisibleCharacter, {4, 7}).argument(DiagnosticArgumentName::Character, U'\U000E0100').argument(DiagnosticArgumentName::Context, DiagnosticSourceContext::Identifier).related(DiagnosticRelatedKind::PreviousVisibleCharacter, {3, 4}, {{DiagnosticArgumentName::Character, U'a'}}).related(DiagnosticRelatedKind::NextVisibleCharacter, {7, 11}, {{DiagnosticArgumentName::Character, U'\U0001F600'}}).build();
      const FormattedDiagnostic Expected{DiagnosticSeverity::Error, "invisible format character U+E0100 appears in an identifier", {{SourceRange{3, 4}, "previous visible character is U+0061"}, {SourceRange{7, 11}, "next visible character is U+1F600"}}};

      EXPECT_EQ(DiagnosticFormatter().format(Value), Expected);
    }

    // Verifies that an invisible character outside an identifier uses the distinct source-text context wording.
    TEST(DiagnosticFormatterTest, FormatsInvisibleCharacterInSourceText)
    {
      const Diagnostic Value = DiagnosticBuilder(DiagnosticKind::InvisibleCharacter, {0, 3}).argument(DiagnosticArgumentName::Character, U'\u200B').argument(DiagnosticArgumentName::Context, DiagnosticSourceContext::SourceText).build();

      EXPECT_EQ(DiagnosticFormatter().format(Value), (FormattedDiagnostic{DiagnosticSeverity::Error, "invisible format character U+200B appears in source text", {}}));
    }

    // Verifies that a malformed typed argument cannot inject producer text and falls back to the registered default message.
    TEST(DiagnosticFormatterTest, FallsBackForMalformedStructuredArguments)
    {
      const Diagnostic Value = DiagnosticBuilder(DiagnosticKind::InvisibleCharacter, {0, 3}).argument(DiagnosticArgumentName::Character, std::string("producer supplied text")).build();

      EXPECT_EQ(DiagnosticFormatter().format(Value), (FormattedDiagnostic{DiagnosticSeverity::Error, "invisible format character must be written explicitly", {}}));
    }
  } // namespace
} // namespace ink::core
