#pragma once

#include "ink/core/diagnostic/diagnostic_engine.h"

#include <string_view>

namespace ink::ir
{
  class Function;
  class Value;
} // namespace ink::ir

namespace ink::ir::detail
{
  [[noreturn]] inline void failVerification(const core::DiagnosticEngine &Diagnostics, std::string_view Context, std::string_view Description)
  {
    Diagnostics.reportICE<core::DiagnosticKind::IRVerificationFailed>({}, Context, Description);
  }

  void verifyTypeValue(const Value &TypeValue, const core::DiagnosticEngine &Diagnostics, std::string_view Context);
  void verifyConstant(const Value &ConstantValue, const core::DiagnosticEngine &Diagnostics, std::string_view Context);
  void verifyFunctionBody(const Function &FunctionValue, const core::DiagnosticEngine &Diagnostics, std::string_view Context);
} // namespace ink::ir::detail
