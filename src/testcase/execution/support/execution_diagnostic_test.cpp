#include "ink/execution/support/execution_diagnostic.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

namespace ink::execution::test
{
  // Success and cancellation are control results and never construct an error for a reporting boundary.
  TEST(ExecutionDiagnosticTest, SuccessAndCancellationAreSilent)
  {
    EXPECT_FALSE(makeExecutionDiagnostic(ExecutionStatus::Success));
    EXPECT_FALSE(makeExecutionDiagnostic(ExecutionStatus::Cancelled));
  }

  // Every execution failure retains a distinct stable execution code and its user-error or invariant classification.
  TEST(ExecutionDiagnosticTest, MapsFailuresToDistinctKindsAndClassifications)
  {
    struct Case
    {
        ExecutionStatus Status;
        core::DiagnosticKind Kind;
        core::DiagnosticClass Class;
        const char *Code;
    };
    using core::DiagnosticClass;
    using core::DiagnosticKind;
    constexpr Case Cases[] = {
        {ExecutionStatus::InvalidFrame, DiagnosticKind::ExecutionInvalidFrame, DiagnosticClass::InternalCompilerError, "INK-E0001"},
        {ExecutionStatus::InvalidBinding, DiagnosticKind::ExecutionInvalidBinding, DiagnosticClass::InternalCompilerError, "INK-E0002"},
        {ExecutionStatus::DuplicateBinding, DiagnosticKind::ExecutionDuplicateBinding, DiagnosticClass::InternalCompilerError, "INK-E0003"},
        {ExecutionStatus::UnknownBinding, DiagnosticKind::ExecutionUnknownBinding, DiagnosticClass::InternalCompilerError, "INK-E0004"},
        {ExecutionStatus::InvalidPlace, DiagnosticKind::ExecutionInvalidPlace, DiagnosticClass::InternalCompilerError, "INK-E0005"},
        {ExecutionStatus::ExpiredPlace, DiagnosticKind::ExecutionExpiredPlace, DiagnosticClass::User, "INK-E0006"},
        {ExecutionStatus::Uninitialized, DiagnosticKind::ExecutionUninitialized, DiagnosticClass::User, "INK-E0007"},
        {ExecutionStatus::RuntimeValue, DiagnosticKind::ExecutionRuntimeValue, DiagnosticClass::User, "INK-E0008"},
        {ExecutionStatus::ReadOnly, DiagnosticKind::ExecutionReadOnly, DiagnosticClass::User, "INK-E0009"},
        {ExecutionStatus::TypeMismatch, DiagnosticKind::ExecutionTypeMismatch, DiagnosticClass::User, "INK-E0010"},
        {ExecutionStatus::ForeignContext, DiagnosticKind::ExecutionForeignContext, DiagnosticClass::InternalCompilerError, "INK-E0011"},
        {ExecutionStatus::InvalidArguments, DiagnosticKind::ExecutionInvalidArguments, DiagnosticClass::User, "INK-E0012"},
        {ExecutionStatus::MissingBody, DiagnosticKind::ExecutionMissingBody, DiagnosticClass::User, "INK-E0013"},
        {ExecutionStatus::SymbolNotFound, DiagnosticKind::ExecutionSymbolNotFound, DiagnosticClass::User, "INK-E0014"},
        {ExecutionStatus::UnsupportedExternalSignature, DiagnosticKind::ExecutionUnsupportedExternalSignature, DiagnosticClass::User, "INK-E0015"},
        {ExecutionStatus::HostAbiMismatch, DiagnosticKind::ExecutionHostAbiMismatch, DiagnosticClass::InternalCompilerError, "INK-E0016"},
        {ExecutionStatus::UnsupportedOperation, DiagnosticKind::ExecutionUnsupportedOperation, DiagnosticClass::InternalCompilerError, "INK-E0018"},
        {ExecutionStatus::DivisionByZero, DiagnosticKind::ExecutionDivisionByZero, DiagnosticClass::User, "INK-E0019"},
        {ExecutionStatus::InvalidShift, DiagnosticKind::ExecutionInvalidShift, DiagnosticClass::User, "INK-E0020"},
        {ExecutionStatus::Overflow, DiagnosticKind::ExecutionOverflow, DiagnosticClass::User, "INK-E0021"},
        {ExecutionStatus::BudgetExceeded, DiagnosticKind::ExecutionBudgetExceeded, DiagnosticClass::InternalCompilerError, "INK-E0022"},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Code);
      const auto Diagnostic = makeExecutionDiagnostic(Entry.Status);
      ASSERT_TRUE(Diagnostic);
      EXPECT_EQ(Diagnostic->Kind, Entry.Kind);
      EXPECT_EQ(Diagnostic->classification(), Entry.Class);
      EXPECT_STREQ(Diagnostic->code(), Entry.Code);
      EXPECT_EQ(core::diagnosticDomain(Diagnostic->Kind), core::DiagnosticDomain::Execution);
      EXPECT_EQ(core::DiagnosticFormatter{}.format(*Diagnostic).Severity, core::DiagnosticSeverity::Error);
      EXPECT_EQ(core::DiagnosticFormatter{}.format(*Diagnostic).Message.find("compile-time"), std::string::npos);
    }
  }

  // Reporting owns the caller context and preserves the exact source and half-open span for a user error.
  TEST(ExecutionDiagnosticTest, PreservesSourceSpanAndOwnedContextWhenReported)
  {
    const core::SourceId Source(17);
    const core::SourceRange Span = core::SourceRange::fromByteOffsets(4, 9);
    std::string Context = "execution of entry 'main' failed";
    const auto Diagnostic = makeExecutionDiagnostic(ExecutionStatus::DivisionByZero, Source, Span, Context);
    ASSERT_TRUE(Diagnostic);
    Context.clear();
    core::DiagnosticEngine Engine;
    core::CollectingDiagnosticConsumer Consumer;
    Engine.addConsumer(Consumer);
    EXPECT_TRUE(Consumer.diagnostics().empty());
    Engine.report(*Diagnostic);
    ASSERT_EQ(Consumer.diagnostics().size(), 1U);
    const auto &Reported = Consumer.diagnostics().front();
    EXPECT_EQ(Reported.Source, Source);
    EXPECT_EQ(Reported.Span, Span);
    EXPECT_EQ(Reported.Kind, core::DiagnosticKind::ExecutionDivisionByZero);
    EXPECT_EQ(core::DiagnosticFormatter{}.format(Reported).Message, "execution of entry 'main' failed: division by zero");
  }

  // An invalid enum value remains inspectable as an ICE and records the unexpected numeric status.
  TEST(ExecutionDiagnosticTest, UnknownStatusProducesAnInvariantDiagnostic)
  {
    const auto Diagnostic = makeExecutionDiagnostic(static_cast<ExecutionStatus>(999));
    ASSERT_TRUE(Diagnostic);
    EXPECT_EQ(Diagnostic->Kind, core::DiagnosticKind::ExecutionInvalidStatus);
    EXPECT_EQ(Diagnostic->classification(), core::DiagnosticClass::InternalCompilerError);
    EXPECT_NE(core::DiagnosticFormatter{}.format(*Diagnostic).Message.find("999"), std::string::npos);
  }

  // Creating an ICE is inert, while submitting an execution resource failure uses the engine's normal panic path.
  TEST(ExecutionDiagnosticDeathTest, ReportsBudgetFailureThroughDiagnosticEngine)
  {
    const auto Diagnostic = makeExecutionDiagnostic(ExecutionStatus::BudgetExceeded, {}, {}, "compile-time execution failed");
    ASSERT_TRUE(Diagnostic);
    core::DiagnosticEngine Engine;
    EXPECT_DEATH(Engine.report(*Diagnostic), "internal compiler error\\[INK-E0022\\]: compile-time execution failed: execution budget exceeded");
  }
} // namespace ink::execution::test
