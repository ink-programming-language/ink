#ifndef INK_CORE_DIAGNOSTIC_FORMATTER_H
#define INK_CORE_DIAGNOSTIC_FORMATTER_H

#include "ink/core/diagnostic/diagnostic.h"

#include <optional>
#include <string>
#include <vector>

namespace ink::core
{
  class SourceManager;

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
      // Render a complete diagnostic without writing to a stream. Columns are one-based byte offsets.
      // Source previews expand tabs to eight-column stops and escape non-printable/non-ASCII bytes.
      std::string format(const Diagnostic &DiagnosticEntry, const SourceManager &Sources) const;
  };
} // namespace ink::core

#endif
