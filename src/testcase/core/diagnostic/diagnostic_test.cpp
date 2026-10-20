#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/diagnostic/diagnostic_builder.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/support/core_define.h"
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

  // Invalid semantic AST diagnostics print their invariant description and stable code before termination.
  TEST(DiagnosticDeathTest, PrintsInvalidSemanticASTReason)
  {
    DiagnosticEngine Engine;
    EXPECT_DEATH(Engine.report<DiagnosticKind::SemanticInvalidAST>({}, "FunctionDecl definition requires a body"), "internal compiler error\\[INK-S0061\\]: invalid AST: FunctionDecl definition requires a body");
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
    auto Malformed = makeDiagnostic<DiagnosticKind::SemanticInvalidAST>({}, "missing required child");
    Malformed.Arguments.clear();
    EXPECT_DEATH(Engine.report(Malformed), "internal compiler error\\[INK-S0061\\]: invalid AST");
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

  // Initialization warnings preserve their identities and messages while allowing subsequent errors to reach consumers.
  TEST(DiagnosticTest, InitializationWarningsDoNotStopDiagnosticDispatch)
  {
    DiagnosticEngine Engine;
    CollectingDiagnosticConsumer Consumer;
    Engine.addConsumer(Consumer);
    const SourceRange ReadSpan = SourceRange::fromByteOffsets(2, 7);
    const SourceRange AddressSpan = SourceRange::fromByteOffsets(11, 17);
    Engine.report<DiagnosticKind::SemanticUninitializedRead>(ReadSpan, "Value");
    Engine.report<DiagnosticKind::SemanticUninitializedAddress>(AddressSpan, "Output");
    Engine.report<DiagnosticKind::SemanticInvalidAddressOperand>({});
    Engine.report<DiagnosticKind::SemanticInvalidDereference>({});
    ASSERT_EQ(Consumer.diagnostics().size(), 4U);
    const auto &Read = Consumer.diagnostics()[0];
    EXPECT_EQ(Read.Kind, DiagnosticKind::SemanticUninitializedRead);
    EXPECT_EQ(Read.number(), 0x03000006U);
    EXPECT_STREQ(Read.code(), "INK-S0006");
    EXPECT_EQ(Read.classification(), DiagnosticClass::User);
    EXPECT_EQ(Read.Span, ReadSpan);
    EXPECT_EQ(DiagnosticFormatter{}.format(Read).Severity, DiagnosticSeverity::Warning);
    EXPECT_EQ(DiagnosticFormatter{}.format(Read).Message, "'Value' is read before initialization");
    const auto &Address = Consumer.diagnostics()[1];
    EXPECT_EQ(Address.Kind, DiagnosticKind::SemanticUninitializedAddress);
    EXPECT_EQ(Address.number(), 0x0300003EU);
    EXPECT_STREQ(Address.code(), "INK-S0062");
    EXPECT_EQ(Address.classification(), DiagnosticClass::User);
    EXPECT_EQ(Address.Span, AddressSpan);
    EXPECT_EQ(DiagnosticFormatter{}.format(Address).Severity, DiagnosticSeverity::Warning);
    EXPECT_EQ(DiagnosticFormatter{}.format(Address).Message, "address of 'Output' is taken before initialization");
    const auto &InvalidAddress = Consumer.diagnostics()[2];
    EXPECT_EQ(InvalidAddress.Kind, DiagnosticKind::SemanticInvalidAddressOperand);
    EXPECT_EQ(DiagnosticFormatter{}.format(InvalidAddress).Severity, DiagnosticSeverity::Error);
    EXPECT_EQ(DiagnosticFormatter{}.format(InvalidAddress).Message, "address-of requires a local variable or a dereference expression");
    const auto &InvalidDereference = Consumer.diagnostics()[3];
    EXPECT_EQ(InvalidDereference.Kind, DiagnosticKind::SemanticInvalidDereference);
    EXPECT_EQ(DiagnosticFormatter{}.format(InvalidDereference).Severity, DiagnosticSeverity::Error);
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

  // Invalid semantic AST diagnostics own their description and preserve their registered identity, classification and fallback.
  TEST(DiagnosticTest, InvalidSemanticASTOwnsDescriptionAndHasStableIdentity)
  {
    const SourceId Source(29);
    const SourceRange Span = SourceRange::fromByteOffsets(3, 11);
    auto Diagnostic = makeDiagnostic<DiagnosticKind::SemanticInvalidAST>(Span, std::string("FunctionDecl definition requires a body"));
    Diagnostic.Source = Source;
    EXPECT_EQ(Diagnostic.Kind, DiagnosticKind::SemanticInvalidAST);
    EXPECT_EQ(diagnosticNumber(Diagnostic.Kind), 0x0300003DU);
    EXPECT_STREQ(Diagnostic.code(), "INK-S0061");
    EXPECT_EQ(Diagnostic.classification(), DiagnosticClass::InternalCompilerError);
    EXPECT_EQ(Diagnostic.Source, Source);
    EXPECT_EQ(Diagnostic.Span, Span);
    const auto Formatted = DiagnosticFormatter{}.format(Diagnostic);
    EXPECT_EQ(Formatted.Severity, DiagnosticSeverity::Error);
    EXPECT_EQ(Formatted.Message, "invalid AST: FunctionDecl definition requires a body");
    ASSERT_EQ(Diagnostic.Arguments.size(), 1U);
    EXPECT_EQ(Diagnostic.Arguments[0].Name, DiagnosticArgumentName::Description);
    EXPECT_EQ(std::get<std::string>(Diagnostic.Arguments[0].Value), "FunctionDecl definition requires a body");
    Diagnostic.Arguments.clear();
    EXPECT_EQ(DiagnosticFormatter{}.format(Diagnostic).Message, "invalid AST");
  }
} // namespace ink::core::test
