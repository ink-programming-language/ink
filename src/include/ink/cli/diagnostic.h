#ifndef INK_CLI_DIAGNOSTIC_H
#define INK_CLI_DIAGNOSTIC_H

#include "ink/core/diagnostic.h"
#include "ink/core/source_manager.h"

#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ink::cli
{
  struct DiagnosticLocation
  {
    std::string Path;
    std::optional<core::SourceRange> Range;
  };

  struct DiagnosticNote
  {
    std::string Message;
    std::optional<DiagnosticLocation> Location;
  };

  struct DiagnosticMessage
  {
    core::DiagnosticSeverity Severity = core::DiagnosticSeverity::Error;
    std::optional<core::DiagnosticKind> Kind;
    std::string Message;
    std::optional<DiagnosticLocation> Location;
    std::vector<DiagnosticNote> Notes;
  };

  class DiagnosticConsumer
  {
  public:
    DiagnosticConsumer(std::string ProgramName, std::ostream &ErrorOutput);

    void report(const DiagnosticMessage &DiagnosticEntry);
    void report(const core::Diagnostic &DiagnosticEntry, const core::SourceManager &Sources);
    void reportError(std::string_view Message);
    void flush();
    bool good() const noexcept;

  private:
    std::string ProgramName;
    std::ostream &ErrorOutput;
  };

  class InternalCompilerError final : public std::runtime_error
  {
  public:
    explicit InternalCompilerError(std::string Message);
  };

  [[noreturn]] void internalCompilerError(std::string Message);
} // namespace ink::cli

#endif
