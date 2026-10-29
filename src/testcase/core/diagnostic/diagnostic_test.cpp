#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/diagnostic/diagnostic_builder.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/source/source_manager.h"
#include "ink/core/support/core_define.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <set>

namespace ink::core::test
{
  // panic flushes its message and call-site location before terminating, even when normal logging is disabled.
  TEST(DiagnosticDeathTest, PanicWritesBeforeTerminating)
  {
    const auto Fail = []()
    {
      spdlog::set_level(spdlog::level::off);
      panic("fatal {message}");
    };
    EXPECT_DEATH(Fail(), "fatal \\{message\\}");
    EXPECT_DEATH(panic("fatal"), "panic at .*diagnostic_test.cpp:");
  }

  // An ICE is printed and aborts even without a diagnostic consumer.
  TEST(DiagnosticDeathTest, PrintsParameterizedICEWithoutConsumers)
  {
    DiagnosticEngine Engine;
    EXPECT_DEATH(Engine.report<DiagnosticKind::ParserInvalidTokenReference>({}, 300U, 256U), "internal compiler error\\[INK-P0048\\]: token index 300 is out of bounds for input of 256 tokens");
  }

  // Explicit fatal reporting preserves parameter formatting without requiring a source or consumer.
  TEST(DiagnosticDeathTest, ExplicitICEEntryPrintsParameterizedDiagnostic)
  {
    const DiagnosticEngine Engine;
    EXPECT_DEATH(Engine.reportICE<DiagnosticKind::ParserInvalidTokenReference>({}, 300U, 256U), "internal compiler error\\[INK-P0048\\]: token index 300 is out of bounds for input of 256 tokens");
  }

  // Explicit fatal reporting retains the selected source and excerpt and bypasses a suppressing consumer.
  TEST(DiagnosticDeathTest, ExplicitICEEntryPreservesSourceAndBypassesConsumers)
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
    Sources.addSource("unrelated.ink", "unrelated source");
    const SourceId Source = Sources.addSource("fatal.ink", "ok\n!");
    DiagnosticEngine Engine(Sources);
    SuppressingConsumer Consumer;
    Engine.addConsumer(Consumer);
    const DiagnosticEngine &View = Engine;
    EXPECT_DEATH(View.reportICE<DiagnosticKind::ParserInvalidTokenReference>(Source, SourceRange::fromByteOffsets(3, 4), 300U, 256U), ::testing::AllOf(::testing::HasSubstr("fatal.ink:2:1: internal compiler error[INK-P0048]: token index 300 is out of bounds for input of 256 tokens"), ::testing::HasSubstr("[bytes 3, 4)"), ::testing::HasSubstr("2 | !"), ::testing::HasSubstr("^")));
  }

  // A consumer cannot intercept or suppress an ICE before the engine prints and panics.
  TEST(DiagnosticDeathTest, PanicsBeforeDispatchingToConsumers)
  {
    class SuppressingConsumer final : public DiagnosticConsumer
    {
      public:
        void consume(const Diagnostic &) override
        {
          std::_Exit(0);
        }
    };
    DiagnosticEngine Engine;
    SuppressingConsumer Consumer;
    Engine.addConsumer(Consumer);
    EXPECT_DEATH(Engine.report<DiagnosticKind::ParserLimitExceeded>({}), "internal compiler error\\[INK-P0004\\]");
  }

  // The effective classification controls termination, including an explicitly promoted user diagnostic.
  TEST(DiagnosticDeathTest, HonorsExplicitICEClassificationAndFormattingFallback)
  {
    DiagnosticEngine Engine;
    auto Promoted = makeDiagnosticBuilder<DiagnosticKind::ParserExpectedToken>({}, ";").classification(DiagnosticClass::InternalCompilerError).build();
    EXPECT_DEATH(Engine.report(Promoted), "internal compiler error\\[INK-P0001\\]: expected ;");
    auto Malformed = makeDiagnostic<DiagnosticKind::ParserInvalidTokenReference>({}, 300U, 256U);
    Malformed.Arguments.clear();
    EXPECT_DEATH(Engine.report(Malformed), "internal compiler error\\[INK-P0048\\]: invalid token reference");
  }

  // The two definition files stay disjoint, retain unique stable identities and enforce their declared classifications.
  TEST(DiagnosticTest, DefinitionTablesHaveDistinctClassesAndIdentities)
  {
    constexpr DiagnosticKind UserKinds[] = {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, Schema) DiagnosticKind::Name,
#include "ink/core/diagnostic/diagnostic_user.def"
#undef INK_DIAGNOSTIC
    };
    constexpr DiagnosticKind ICEKinds[] = {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, Schema) DiagnosticKind::Name,
#include "ink/core/diagnostic/diagnostic_ice.def"
#undef INK_DIAGNOSTIC
    };
    std::set<std::uint32_t> Numbers;
    std::set<std::string> Codes;
    const auto Check = [&](const auto &Kinds, DiagnosticClass Class)
    {
      for (DiagnosticKind Kind : Kinds)
      {
        SCOPED_TRACE(diagnosticKindName(Kind));
        EXPECT_EQ(diagnosticClass(Kind), Class);
        if (Class == DiagnosticClass::InternalCompilerError)
        {
          EXPECT_EQ(diagnosticDefaultSeverity(Kind), DiagnosticSeverity::Error);
        }
        else
        {
          EXPECT_NE(diagnosticDefaultSeverity(Kind), DiagnosticSeverity::Unknown);
        }
        EXPECT_TRUE(Numbers.insert(diagnosticNumber(Kind)).second);
        EXPECT_TRUE(Codes.insert(diagnosticCode(Kind)).second);
        EXPECT_NE(diagnosticNumber(Kind), 0U);
        EXPECT_EQ(diagnosticNumber(Kind) >> 24, static_cast<std::uint32_t>(diagnosticDomain(Kind)));
        EXPECT_FALSE(std::string_view(diagnosticDefaultMessage(Kind)).empty());
      }
    };
    Check(UserKinds, DiagnosticClass::User);
    Check(ICEKinds, DiagnosticClass::InternalCompilerError);
    EXPECT_EQ(diagnosticClass(DiagnosticKind::Unknown), DiagnosticClass::Unknown);
    EXPECT_STREQ(diagnosticCode(DiagnosticKind::Unknown), "INK-0000");
  }

  // Reclassifying resource limits must preserve existing diagnostic numbers and displayed codes.
  TEST(DiagnosticTest, ResourceLimitsAreICEWithoutRenumbering)
  {
    EXPECT_EQ(diagnosticClass(DiagnosticKind::SourceTooLarge), DiagnosticClass::InternalCompilerError);
    EXPECT_EQ(diagnosticNumber(DiagnosticKind::SourceTooLarge), 0x01000016U);
    EXPECT_STREQ(diagnosticCode(DiagnosticKind::SourceTooLarge), "INK-T0022");
    EXPECT_EQ(diagnosticClass(DiagnosticKind::ParserLimitExceeded), DiagnosticClass::InternalCompilerError);
    EXPECT_EQ(diagnosticNumber(DiagnosticKind::ParserLimitExceeded), 0x02000004U);
    EXPECT_STREQ(diagnosticCode(DiagnosticKind::ParserLimitExceeded), "INK-P0004");
    EXPECT_EQ(diagnosticClass(DiagnosticKind::InvalidUtf8), DiagnosticClass::User);
    EXPECT_EQ(diagnosticClass(DiagnosticKind::ParserExpectedToken), DiagnosticClass::User);
  }

  // User errors return to their caller, while ICE values can be inspected and formatted before reporting.
  TEST(DiagnosticTest, EngineDeliversUserErrorsAndParameterizedICE)
  {
    DiagnosticEngine Engine;
    CollectingDiagnosticConsumer Consumer;
    Engine.addConsumer(Consumer);
    Engine.report<DiagnosticKind::ParserExpectedToken>(SourceRange::fromByteOffsets(2, 2), ";");
    Engine.removeConsumer(Consumer);
    ASSERT_EQ(Consumer.diagnostics().size(), 1U);
    const auto &User = Consumer.diagnostics()[0];
    EXPECT_EQ(User.classification(), DiagnosticClass::User);
    EXPECT_EQ(DiagnosticFormatter{}.format(User).Message, "expected ;");
    const auto InvalidReference = makeDiagnostic<DiagnosticKind::ParserInvalidTokenReference>({}, 300U, 256U);
    EXPECT_EQ(InvalidReference.classification(), DiagnosticClass::InternalCompilerError);
    EXPECT_TRUE(InvalidReference.Span.isInvalid());
    EXPECT_EQ(DiagnosticFormatter{}.format(InvalidReference).Severity, DiagnosticSeverity::Error);
    EXPECT_EQ(DiagnosticFormatter{}.format(InvalidReference).Message, "token index 300 is out of bounds for input of 256 tokens");
    ASSERT_EQ(InvalidReference.Arguments.size(), 2U);
    EXPECT_EQ(InvalidReference.Arguments[0].Name, DiagnosticArgumentName::ActualValue);
    EXPECT_EQ(std::get<std::uint64_t>(InvalidReference.Arguments[0].Value), 300U);
    EXPECT_EQ(InvalidReference.Arguments[1].Name, DiagnosticArgumentName::Size);
    EXPECT_EQ(std::get<std::uint64_t>(InvalidReference.Arguments[1].Value), 256U);
  }

  // Source-aware reporting preserves each source and span for both argument-free and parameterized diagnostics.
  TEST(DiagnosticTest, EngineReportsSourceIdentityAndSpan)
  {
    DiagnosticEngine Engine;
    CollectingDiagnosticConsumer Consumer;
    Engine.addConsumer(Consumer);
    const DiagnosticEngine &View = Engine;
    const SourceId FirstSource(17);
    const SourceId SecondSource(23);
    const SourceRange FirstSpan = SourceRange::fromByteOffsets(2, 8);
    const SourceRange SecondSpan = SourceRange::fromByteOffsets(11, 11);
    std::string ExpectedToken = ";";
    View.report<DiagnosticKind::InvalidUtf8>(FirstSource, FirstSpan);
    View.report<DiagnosticKind::ParserExpectedToken>(SecondSource, SecondSpan, ExpectedToken);
    ExpectedToken = ")";
    ASSERT_EQ(Consumer.diagnostics().size(), 2U);
    const auto &First = Consumer.diagnostics()[0];
    EXPECT_EQ(First.Kind, DiagnosticKind::InvalidUtf8);
    EXPECT_EQ(First.Source, FirstSource);
    EXPECT_EQ(First.Span, FirstSpan);
    EXPECT_TRUE(First.Arguments.empty());
    const auto &Second = Consumer.diagnostics()[1];
    EXPECT_EQ(Second.Kind, DiagnosticKind::ParserExpectedToken);
    EXPECT_EQ(Second.Source, SecondSource);
    EXPECT_EQ(Second.Span, SecondSpan);
    EXPECT_EQ(DiagnosticFormatter{}.format(Second).Message, "expected ;");
  }
} // namespace ink::core::test
