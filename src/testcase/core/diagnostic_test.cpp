#include "ink/core/diagnostic.h"
#include <gtest/gtest.h>
#include <set>

namespace ink::core::test
{
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

  // Both classifications use the existing engine and formatter, with archive values retained as typed arguments.
  TEST(DiagnosticTest, EngineDeliversUserErrorsAndParameterizedICE)
  {
    DiagnosticEngine Engine;
    CollectingDiagnosticConsumer Consumer;
    Engine.addConsumer(Consumer);
    Engine.report<DiagnosticKind::ParserExpectedToken>(SourceRange::fromByteOffsets(2, 2), ";");
    Engine.report<DiagnosticKind::ASTArchiveSizeLimitExceeded>({}, 300U, 256U);
    Engine.report<DiagnosticKind::ASTArchiveUnsupportedVersion>({}, 9U, 1U);
    Engine.removeConsumer(Consumer);
    ASSERT_EQ(Consumer.diagnostics().size(), 3U);
    const auto &User = Consumer.diagnostics()[0];
    EXPECT_EQ(User.classification(), DiagnosticClass::User);
    EXPECT_EQ(DiagnosticFormatter{}.format(User).Message, "expected ;");
    const auto &Limit = Consumer.diagnostics()[1];
    EXPECT_EQ(Limit.classification(), DiagnosticClass::InternalCompilerError);
    EXPECT_TRUE(Limit.Span.isInvalid());
    EXPECT_EQ(DiagnosticFormatter{}.format(Limit).Severity, DiagnosticSeverity::Error);
    EXPECT_EQ(DiagnosticFormatter{}.format(Limit).Message, "AST archive size 300 bytes exceeds limit 256 bytes");
    ASSERT_EQ(Limit.Arguments.size(), 2U);
    EXPECT_EQ(Limit.Arguments[0].Name, DiagnosticArgumentName::Size);
    EXPECT_EQ(std::get<std::uint64_t>(Limit.Arguments[0].Value), 300U);
    EXPECT_EQ(Limit.Arguments[1].Name, DiagnosticArgumentName::MaximumSize);
    EXPECT_EQ(std::get<std::uint64_t>(Limit.Arguments[1].Value), 256U);
    const auto &Version = Consumer.diagnostics()[2];
    EXPECT_EQ(Version.classification(), DiagnosticClass::InternalCompilerError);
    EXPECT_EQ(DiagnosticFormatter{}.format(Version).Message, "unsupported AST archive version 9; supported version is 1");
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
