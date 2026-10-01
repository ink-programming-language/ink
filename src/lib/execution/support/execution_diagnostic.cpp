#include "ink/execution/support/execution_diagnostic.h"

#include <cstdint>

namespace ink::execution
{
  std::optional<core::Diagnostic> makeExecutionDiagnostic(ExecutionStatus Status, core::SourceId Source, core::SourceRange Span, std::string_view Context)
  {
    core::Diagnostic Entry;
    switch (Status)
    {
    case ExecutionStatus::Success:
    case ExecutionStatus::Cancelled:
      return std::nullopt;
    case ExecutionStatus::InvalidFrame:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionInvalidFrame>(Span, Context);
      break;
    case ExecutionStatus::InvalidBinding:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionInvalidBinding>(Span, Context);
      break;
    case ExecutionStatus::DuplicateBinding:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionDuplicateBinding>(Span, Context);
      break;
    case ExecutionStatus::UnknownBinding:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionUnknownBinding>(Span, Context);
      break;
    case ExecutionStatus::InvalidPlace:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionInvalidPlace>(Span, Context);
      break;
    case ExecutionStatus::ExpiredPlace:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionExpiredPlace>(Span, Context);
      break;
    case ExecutionStatus::Uninitialized:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionUninitialized>(Span, Context);
      break;
    case ExecutionStatus::RuntimeValue:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionRuntimeValue>(Span, Context);
      break;
    case ExecutionStatus::ReadOnly:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionReadOnly>(Span, Context);
      break;
    case ExecutionStatus::TypeMismatch:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionTypeMismatch>(Span, Context);
      break;
    case ExecutionStatus::ForeignContext:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionForeignContext>(Span, Context);
      break;
    case ExecutionStatus::InvalidArguments:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionInvalidArguments>(Span, Context);
      break;
    case ExecutionStatus::MissingBody:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionMissingBody>(Span, Context);
      break;
    case ExecutionStatus::SymbolNotFound:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionSymbolNotFound>(Span, Context);
      break;
    case ExecutionStatus::UnsupportedExternalSignature:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionUnsupportedExternalSignature>(Span, Context);
      break;
    case ExecutionStatus::HostAbiMismatch:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionHostAbiMismatch>(Span, Context);
      break;
    case ExecutionStatus::UnsupportedOperation:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionUnsupportedOperation>(Span, Context);
      break;
    case ExecutionStatus::DivisionByZero:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionDivisionByZero>(Span, Context);
      break;
    case ExecutionStatus::InvalidShift:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionInvalidShift>(Span, Context);
      break;
    case ExecutionStatus::Overflow:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionOverflow>(Span, Context);
      break;
    case ExecutionStatus::BudgetExceeded:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionBudgetExceeded>(Span, Context);
      break;
    default:
      Entry = core::makeDiagnostic<core::DiagnosticKind::ExecutionInvalidStatus>(Span, Context, static_cast<std::int64_t>(Status));
      break;
    }
    Entry.Source = Source;
    return Entry;
  }
} // namespace ink::execution
