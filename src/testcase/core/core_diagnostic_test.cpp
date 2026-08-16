#include "ink/core/context.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
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
      DiagnosticClass Class;
      DiagnosticSeverity Severity;
      const char *Code;
      const char *Name;
      const char *DefaultMessage;
      const char *FormatPattern;
    };

    constexpr DiagnosticExpectation DiagnosticExpectations[] = {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) {DiagnosticKind::Name, Number, DiagnosticDomain::Domain, DiagnosticClass::Class, DiagnosticSeverity::DefaultSeverity, Code, #Name, DefaultMessage, FormatPattern},
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    };

    constexpr std::size_t DiagnosticExpectationCount = sizeof(DiagnosticExpectations) / sizeof(DiagnosticExpectations[0]);

    template <typename ValueType>
    ValueType sampleArgumentValue()
    {
      if constexpr (std::is_same_v<ValueType, bool>)
      {
        return true;
      }
      else if constexpr (std::is_same_v<ValueType, std::int64_t>)
      {
        return -3;
      }
      else if constexpr (std::is_same_v<ValueType, std::uint64_t>)
      {
        return 3;
      }
      else if constexpr (std::is_same_v<ValueType, char32_t>)
      {
        return U'\u200B';
      }
      else if constexpr (std::is_same_v<ValueType, std::string>)
      {
        return "sample";
      }
      else
      {
        return DiagnosticSourceContext::Identifier;
      }
    }

    template <DiagnosticKind Kind, typename... Specifications>
    Diagnostic makeSampleDiagnostic(DiagnosticArgumentSchema<Specifications...>)
    {
      return makeDiagnostic<Kind>({}, sampleArgumentValue<typename Specifications::ValueType>()...);
    }

    static_assert(std::is_aggregate_v<Diagnostic>, "Diagnostic must remain an aggregate");

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
      EXPECT_EQ(Value.Class, DiagnosticClass::Unknown);
      EXPECT_EQ(Value.classification(), DiagnosticClass::Unknown);
      EXPECT_EQ(Value.number(), 0U);
      EXPECT_STREQ(Value.code(), "INK-0000");
      EXPECT_EQ(Argument.Name, DiagnosticArgumentName::Unknown);
      EXPECT_TRUE(std::holds_alternative<bool>(Argument.Value));
      EXPECT_FALSE(std::get<bool>(Argument.Value));
      EXPECT_EQ(Related.Kind, DiagnosticRelatedKind::Unknown);
      EXPECT_EQ(Related.Span, (SourceRange{}));
      EXPECT_TRUE(Related.Arguments.empty());
    }

    // Verifies every registered diagnostic kind against its generated name, message, format schema, and stable number layout.
    TEST(DiagnosticTest, ExposesEveryRegisteredMapping)
    {
      for (const DiagnosticExpectation &Expectation : DiagnosticExpectations)
      {
        EXPECT_EQ(static_cast<std::uint32_t>(Expectation.Kind), Expectation.Number);
        EXPECT_EQ(diagnosticNumber(Expectation.Kind), Expectation.Number);
        EXPECT_STREQ(diagnosticCode(Expectation.Kind), Expectation.Code);
        EXPECT_STREQ(diagnosticKindName(Expectation.Kind), Expectation.Name);
        EXPECT_STREQ(diagnosticDefaultMessage(Expectation.Kind), Expectation.DefaultMessage);
        EXPECT_STREQ(diagnosticFormatPattern(Expectation.Kind), Expectation.FormatPattern);
        EXPECT_EQ(diagnosticDomain(Expectation.Kind), Expectation.Domain);
        EXPECT_EQ(diagnosticClass(Expectation.Kind), Expectation.Class);
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

    // Verifies public names for diagnostic classes and severities, including defensive invalid-value fallbacks.
    TEST(DiagnosticTest, NamesClassesAndSeverities)
    {
      EXPECT_STREQ(diagnosticClassName(DiagnosticClass::Unknown), "unknown");
      EXPECT_STREQ(diagnosticClassName(DiagnosticClass::User), "user");
      EXPECT_STREQ(diagnosticClassName(DiagnosticClass::InternalCompilerError), "internal compiler error");
      EXPECT_STREQ(diagnosticClassName(static_cast<DiagnosticClass>(0xFF)), "unknown");
      EXPECT_STREQ(diagnosticSeverityName(DiagnosticSeverity::Unknown), "unknown");
      EXPECT_STREQ(diagnosticSeverityName(DiagnosticSeverity::Error), "error");
      EXPECT_STREQ(diagnosticSeverityName(DiagnosticSeverity::Warning), "warning");
      EXPECT_STREQ(diagnosticSeverityName(DiagnosticSeverity::Note), "note");
      EXPECT_STREQ(diagnosticSeverityName(static_cast<DiagnosticSeverity>(0xFF)), "unknown");
    }

    // Verifies that a diagnostic's User or ICE class is independent from severity and can be overridden for context-sensitive verification.
    TEST(DiagnosticTest, DistinguishesUserErrorsFromInternalCompilerErrors)
    {
      const Diagnostic UserError = makeDiagnostic<DiagnosticKind::InvalidUtf8>({1, 2});
      const Diagnostic InternalError = makeDiagnostic<DiagnosticKind::SsaValueUnavailableDuringExecution>({}, std::uint64_t{7});
      const Diagnostic Reclassified = makeDiagnosticBuilder<DiagnosticKind::IrFunctionUnknownKind>({}, std::string("main")).classification(DiagnosticClass::User).build();

      EXPECT_EQ(UserError.classification(), DiagnosticClass::User);
      EXPECT_EQ(InternalError.classification(), DiagnosticClass::InternalCompilerError);
      EXPECT_EQ(Reclassified.classification(), DiagnosticClass::User);
      EXPECT_EQ(diagnosticDefaultSeverity(UserError.Kind), DiagnosticSeverity::Error);
      EXPECT_EQ(diagnosticDefaultSeverity(InternalError.Kind), DiagnosticSeverity::Error);
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
      EXPECT_STREQ(diagnosticFormatPattern(InvalidKind), "unknown diagnostic");
      EXPECT_EQ(diagnosticDomain(InvalidKind), DiagnosticDomain::Unknown);
      EXPECT_EQ(diagnosticClass(InvalidKind), DiagnosticClass::Unknown);
      EXPECT_EQ(diagnosticDefaultSeverity(InvalidKind), DiagnosticSeverity::Unknown);
      EXPECT_EQ(Formatted, (FormattedDiagnostic{DiagnosticSeverity::Unknown, "unknown diagnostic", {}}));
    }

    // Verifies zero-, one-, two-, and three-argument schemas format without producer-side message assembly.
    TEST(DiagnosticFormatterTest, FormatsDifferentRegisteredArgumentCounts)
    {
      const Diagnostic Zero = makeDiagnostic<DiagnosticKind::InvalidUtf8>({2, 4});
      const Diagnostic One = makeDiagnostic<DiagnosticKind::ExpectedToken>({8, 8}, std::string_view(")"));
      const Diagnostic Two = makeDiagnostic<DiagnosticKind::InvisibleCharacterInContext>({4, 7}, U'\U000E0100', DiagnosticSourceContext::Identifier);
      const Diagnostic Three = makeDiagnostic<DiagnosticKind::EntryArgumentCountMismatch>({}, std::string("main"), std::uint64_t{2}, std::uint64_t{3});

      EXPECT_EQ(DiagnosticFormatter().format(Zero).Message, "invalid UTF-8");
      EXPECT_EQ(DiagnosticFormatter().format(One).Message, "expected token ')'");
      EXPECT_EQ(DiagnosticFormatter().format(Two).Message, "invisible format character U+E0100 appears in an identifier");
      EXPECT_EQ(DiagnosticFormatter().format(Three).Message, "entry function @main received 3 arguments; expected 2");
    }

    // Verifies every registered format pattern accepts a representative value for each declared schema entry.
    TEST(DiagnosticFormatterTest, FormatsEveryRegisteredSchema)
    {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema)                         \
  {                                                                                                                                              \
    const Diagnostic Value = makeSampleDiagnostic<DiagnosticKind::Name>(typename DiagnosticTraits<DiagnosticKind::Name>::Arguments{});           \
    const FormattedDiagnostic Formatted = DiagnosticFormatter().format(Value);                                                                    \
    if (std::strcmp(DefaultMessage, FormatPattern) != 0)                                                                                           \
    {                                                                                                                                              \
      EXPECT_NE(Formatted.Message, DefaultMessage) << #Name;                                                                                       \
    }                                                                                                                                              \
  }
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
    }

    // Verifies missing, extra, duplicate, and wrongly typed arguments all use the registered safe fallback.
    TEST(DiagnosticFormatterTest, RejectsMalformedStructuredArguments)
    {
      const Diagnostic Missing{DiagnosticKind::InvisibleCharacterInContext, {0, 3}, {{DiagnosticArgumentName::Character, U'\u200B'}}, {}};
      const Diagnostic Extra{DiagnosticKind::InvisibleCharacterInContext, {0, 3}, {{DiagnosticArgumentName::Character, U'\u200B'}, {DiagnosticArgumentName::Context, DiagnosticSourceContext::Identifier}, {DiagnosticArgumentName::Actual, std::string("injected")}}, {}};
      const Diagnostic Duplicate{DiagnosticKind::InvisibleCharacterInContext, {0, 3}, {{DiagnosticArgumentName::Character, U'\u200B'}, {DiagnosticArgumentName::Character, U'\u200C'}}, {}};
      const Diagnostic WrongType{DiagnosticKind::InvisibleCharacterInContext, {0, 3}, {{DiagnosticArgumentName::Character, std::string("producer supplied text")}, {DiagnosticArgumentName::Context, DiagnosticSourceContext::Identifier}}, {}};

      EXPECT_EQ(DiagnosticFormatter().format(Missing).Message, "invisible format character must be written explicitly");
      EXPECT_EQ(DiagnosticFormatter().format(Extra).Message, "invisible format character must be written explicitly");
      EXPECT_EQ(DiagnosticFormatter().format(Duplicate).Message, "invisible format character must be written explicitly");
      EXPECT_EQ(DiagnosticFormatter().format(WrongType).Message, "invisible format character must be written explicitly");
    }

    // Verifies block-comment related IDs produce located and unlocated notes without producer-created note text.
    TEST(DiagnosticFormatterTest, FormatsBlockCommentRelatedInformation)
    {
      const Diagnostic Located = makeDiagnosticBuilder<DiagnosticKind::UnterminatedBlockComment>({0, 2}, std::uint64_t{2}).related(DiagnosticRelatedKind::MostRecentUnclosedBlockComment, {8, 10}).build();
      const Diagnostic Unavailable = makeDiagnosticBuilder<DiagnosticKind::UnterminatedBlockComment>({0, 2}, std::uint64_t{4096}).related(DiagnosticRelatedKind::MostRecentBlockCommentOpeningUnavailable, {}).build();

      EXPECT_EQ(DiagnosticFormatter().format(Located), (FormattedDiagnostic{DiagnosticSeverity::Error, "block comment is not terminated; remaining nesting depth: 2", {{SourceRange{8, 10}, "most recent unclosed block comment opening is here"}}}));
      EXPECT_EQ(DiagnosticFormatter().format(Unavailable), (FormattedDiagnostic{DiagnosticSeverity::Error, "block comment is not terminated; remaining nesting depth: 4096", {{std::nullopt, "most recent unclosed opening was not retained after the nesting limit was exceeded"}}}));
    }

    // Verifies invisible-character related IDs format adjacent code points as located notes.
    TEST(DiagnosticFormatterTest, FormatsInvisibleCharacterRelatedInformation)
    {
      const Diagnostic Value = makeDiagnosticBuilder<DiagnosticKind::InvisibleCharacterInContext>({4, 7}, U'\U000E0100', DiagnosticSourceContext::Identifier).related(DiagnosticRelatedKind::PreviousVisibleCharacter, {3, 4}, {{DiagnosticArgumentName::Character, U'a'}}).related(DiagnosticRelatedKind::NextVisibleCharacter, {7, 11}, {{DiagnosticArgumentName::Character, U'\U0001F600'}}).build();
      const FormattedDiagnostic Expected{DiagnosticSeverity::Error, "invisible format character U+E0100 appears in an identifier", {{SourceRange{3, 4}, "previous visible character is U+0061"}, {SourceRange{7, 11}, "next visible character is U+1F600"}}};

      EXPECT_EQ(DiagnosticFormatter().format(Value), Expected);
    }

    // Verifies diagnostic equality includes arguments, related information, and the instance-level class override.
    TEST(DiagnosticTest, ComparesStructuredValuesAndClassification)
    {
      const Diagnostic Left = makeDiagnosticBuilder<DiagnosticKind::InvisibleCharacterInContext>({2, 5}, U'\u200B', DiagnosticSourceContext::Identifier).related(DiagnosticRelatedKind::PreviousVisibleCharacter, {1, 2}, {{DiagnosticArgumentName::Character, U'a'}}).build();
      const Diagnostic Equal = makeDiagnosticBuilder<DiagnosticKind::InvisibleCharacterInContext>({2, 5}, U'\u200B', DiagnosticSourceContext::Identifier).related(DiagnosticRelatedKind::PreviousVisibleCharacter, {1, 2}, {{DiagnosticArgumentName::Character, U'a'}}).build();
      const Diagnostic DifferentArgument = makeDiagnosticBuilder<DiagnosticKind::InvisibleCharacterInContext>({2, 5}, U'\u200C', DiagnosticSourceContext::Identifier).related(DiagnosticRelatedKind::PreviousVisibleCharacter, {1, 2}, {{DiagnosticArgumentName::Character, U'a'}}).build();
      const Diagnostic DifferentClass = makeDiagnosticBuilder<DiagnosticKind::InvisibleCharacterInContext>({2, 5}, U'\u200B', DiagnosticSourceContext::Identifier).classification(DiagnosticClass::InternalCompilerError).related(DiagnosticRelatedKind::PreviousVisibleCharacter, {1, 2}, {{DiagnosticArgumentName::Character, U'a'}}).build();

      EXPECT_EQ(Left, Equal);
      EXPECT_NE(Left, DifferentArgument);
      EXPECT_NE(Left, DifferentClass);
    }

    // Verifies one diagnostic engine broadcasts typed reports to multiple consumers and supports removal.
    TEST(DiagnosticEngineTest, BroadcastsTypedReportsToRegisteredConsumers)
    {
      DiagnosticEngine Engine;
      CollectingDiagnosticConsumer First;
      CollectingDiagnosticConsumer Second;
      Engine.addConsumer(First);
      Engine.addConsumer(First);
      Engine.addConsumer(Second);

      Engine.report<DiagnosticKind::ExpectedToken>({2, 3}, std::string(")"));
      Engine.removeConsumer(First);
      Engine.report<DiagnosticKind::InvalidUtf8>({4, 5});

      ASSERT_EQ(First.diagnostics().size(), 1u);
      ASSERT_EQ(Second.diagnostics().size(), 2u);
      EXPECT_EQ(First.diagnostics()[0].Kind, DiagnosticKind::ExpectedToken);
      EXPECT_EQ(Second.diagnostics()[0], First.diagnostics()[0]);
      EXPECT_EQ(Second.diagnostics()[1].Kind, DiagnosticKind::InvalidUtf8);
    }

    // Verifies phase contexts compose around one compilation context and expose the same diagnostic engine.
    TEST(ContextTest, SharesCompilationDiagnosticEngineWithFrontend)
    {
      CompilationContext Compilation;
      FrontendContext Frontend(Compilation);

      EXPECT_EQ(&Frontend.compilationContext(), &Compilation);
      EXPECT_EQ(&Frontend.diagnosticEngine(), &Compilation.diagnosticEngine());
    }
  } // namespace
} // namespace ink::core
