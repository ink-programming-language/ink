#include "ink/core/diagnostic.h"
#include "ink/core/core_define.h"
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <set>

namespace ink::core::test
{
  // PANIC flushes its message and call-site location before terminating, even when normal logging is disabled.
  TEST(DiagnosticDeathTest, PanicWritesBeforeTerminating)
  {
    const auto Fail = []()
    {
      spdlog::set_level(spdlog::level::off);
      PANIC("fatal {message}");
    };
    EXPECT_DEATH(Fail(), "fatal \\{message\\}");
    EXPECT_DEATH(PANIC("fatal"), "panic at .*diagnostic_test.cpp:");
  }

  // An ICE is printed and aborts even without a diagnostic consumer.
  TEST(DiagnosticDeathTest, PrintsParameterizedICEWithoutConsumers)
  {
    DiagnosticEngine Engine;
    EXPECT_DEATH(Engine.report<DiagnosticKind::ASTArchiveSizeLimitExceeded>({}, 300U, 256U), "internal compiler error\\[INK-P0013\\]: AST archive size 300 bytes exceeds limit 256 bytes");
    EXPECT_DEATH(Engine.report<DiagnosticKind::ASTArchiveUnsupportedVersion>({}, 9U, 1U), "unsupported AST archive version 9; supported version is 1");
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
    EXPECT_DEATH(Engine.report<DiagnosticKind::SemanticConstructionFailed>({}), "internal compiler error\\[INK-S0013\\]");
  }

  // The effective classification controls termination, including an explicitly promoted user diagnostic.
  TEST(DiagnosticDeathTest, HonorsExplicitICEClassificationAndFormattingFallback)
  {
    DiagnosticEngine Engine;
    auto Promoted = makeDiagnosticBuilder<DiagnosticKind::ParserExpectedToken>({}, ";").classification(DiagnosticClass::InternalCompilerError).build();
    EXPECT_DEATH(Engine.report(Promoted), "internal compiler error\\[INK-P0001\\]: expected ;");
    auto Malformed = makeDiagnostic<DiagnosticKind::ASTArchiveInvalidTree>({}, "missing required child");
    Malformed.Arguments.clear();
    EXPECT_DEATH(Engine.report(Malformed), "internal compiler error\\[INK-P0009\\]: invalid AST archive tree");
  }

  // The two definition files stay disjoint, retain unique stable identities and enforce their declared classifications.
  TEST(DiagnosticTest, DefinitionTablesHaveDistinctClassesAndIdentities)
  {
    constexpr DiagnosticKind UserKinds[] = {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, Schema) DiagnosticKind::Name,
#include "ink/core/diagnostic_user.def"
#undef INK_DIAGNOSTIC
    };
    constexpr DiagnosticKind ICEKinds[] = {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, Schema) DiagnosticKind::Name,
#include "ink/core/diagnostic_ice.def"
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
        EXPECT_EQ(diagnosticDefaultSeverity(Kind), DiagnosticSeverity::Error);
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
    const auto Limit = makeDiagnostic<DiagnosticKind::ASTArchiveSizeLimitExceeded>({}, 300U, 256U);
    EXPECT_EQ(Limit.classification(), DiagnosticClass::InternalCompilerError);
    EXPECT_TRUE(Limit.Span.isInvalid());
    EXPECT_EQ(DiagnosticFormatter{}.format(Limit).Severity, DiagnosticSeverity::Error);
    EXPECT_EQ(DiagnosticFormatter{}.format(Limit).Message, "AST archive size 300 bytes exceeds limit 256 bytes");
    ASSERT_EQ(Limit.Arguments.size(), 2U);
    EXPECT_EQ(Limit.Arguments[0].Name, DiagnosticArgumentName::Size);
    EXPECT_EQ(std::get<std::uint64_t>(Limit.Arguments[0].Value), 300U);
    EXPECT_EQ(Limit.Arguments[1].Name, DiagnosticArgumentName::MaximumSize);
    EXPECT_EQ(std::get<std::uint64_t>(Limit.Arguments[1].Value), 256U);
    const auto Version = makeDiagnostic<DiagnosticKind::ASTArchiveUnsupportedVersion>({}, 9U, 1U);
    EXPECT_EQ(Version.classification(), DiagnosticClass::InternalCompilerError);
    EXPECT_EQ(DiagnosticFormatter{}.format(Version).Message, "unsupported AST archive version 9; supported version is 1");
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

  // An owned verifier reason survives its producer, and malformed diagnostic arguments use the registered fallback.
  TEST(DiagnosticTest, ArchiveICEOwnsReasonAndHasFormattingFallback)
  {
    auto Diagnostic = makeDiagnostic<DiagnosticKind::ASTArchiveInvalidTree>({}, std::string("missing required child"));
    EXPECT_EQ(Diagnostic.classification(), DiagnosticClass::InternalCompilerError);
    EXPECT_EQ(DiagnosticFormatter{}.format(Diagnostic).Message, "invalid AST archive tree: missing required child");
    Diagnostic.Arguments.clear();
    EXPECT_EQ(DiagnosticFormatter{}.format(Diagnostic).Message, "invalid AST archive tree");
  }
} // namespace ink::core::test
