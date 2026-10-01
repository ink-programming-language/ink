#ifndef INK_EXECUTION_SUPPORT_EXECUTION_DIAGNOSTIC_H
#define INK_EXECUTION_SUPPORT_EXECUTION_DIAGNOSTIC_H

#include "ink/core/diagnostic.h"
#include "ink/execution/support/execution_result.h"

#include <optional>
#include <string_view>

namespace ink::execution
{
  // Construct a diagnostic at the reporting boundary; success and cancellation are silent.
  std::optional<core::Diagnostic> makeExecutionDiagnostic(ExecutionStatus Status, core::SourceId Source = {}, core::SourceRange Span = {}, std::string_view Context = "execution failed");
} // namespace ink::execution

#endif
