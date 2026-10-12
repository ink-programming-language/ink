#ifndef INK_CORE_DIAGNOSTIC_FORMATTER_H
#define INK_CORE_DIAGNOSTIC_FORMATTER_H

#include "ink/core/diagnostic/diagnostic.h"

#include <optional>
#include <string>
#include <vector>

namespace ink::core
{
  struct FormattedDiagnosticNote
  {
      std::optional<SourceRange> Span;
      std::string Message;
  };

  bool operator==(const FormattedDiagnosticNote &Left, const FormattedDiagnosticNote &Right);
  bool operator!=(const FormattedDiagnosticNote &Left, const FormattedDiagnosticNote &Right);

  struct FormattedDiagnostic
  {
      DiagnosticSeverity Severity = DiagnosticSeverity::Unknown;
      std::string Message;
      std::vector<FormattedDiagnosticNote> Notes;
  };

  bool operator==(const FormattedDiagnostic &Left, const FormattedDiagnostic &Right);
  bool operator!=(const FormattedDiagnostic &Left, const FormattedDiagnostic &Right);

  class DiagnosticFormatter
  {
    public:
      FormattedDiagnostic format(const Diagnostic &DiagnosticEntry) const;
  };
} // namespace ink::core

#endif
