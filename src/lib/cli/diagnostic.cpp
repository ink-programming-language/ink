#include "ink/cli/diagnostic.h"

#include <ostream>
#include <utility>

namespace ink::cli
{
  namespace
  {
    void printLocation(std::ostream &Output, const DiagnosticLocation &Location)
    {
      Output << Location.Path << ": ";
    }

    void printRange(std::ostream &Output, const std::optional<core::SourceRange> &Range)
    {
      if (Range)
      {
        Output << " [" << Range->Start << ", " << Range->End << ')';
      }
    }
  } // namespace

  DiagnosticConsumer::DiagnosticConsumer(std::string ProgramName, std::ostream &ErrorOutput) : ProgramName(std::move(ProgramName)), ErrorOutput(ErrorOutput)
  {
  }

  void DiagnosticConsumer::report(const DiagnosticMessage &DiagnosticEntry)
  {
    if (DiagnosticEntry.Location)
    {
      printLocation(ErrorOutput, *DiagnosticEntry.Location);
    }
    else
    {
      ErrorOutput << ProgramName << ": ";
    }
    ErrorOutput << core::diagnosticSeverityName(DiagnosticEntry.Severity);
    if (DiagnosticEntry.Kind)
    {
      ErrorOutput << '[' << core::diagnosticCode(*DiagnosticEntry.Kind) << ']';
    }
    ErrorOutput << ": " << DiagnosticEntry.Message;
    if (DiagnosticEntry.Location)
    {
      printRange(ErrorOutput, DiagnosticEntry.Location->Range);
    }
    ErrorOutput << '\n';

    for (const DiagnosticNote &Note : DiagnosticEntry.Notes)
    {
      if (Note.Location)
      {
        printLocation(ErrorOutput, *Note.Location);
      }
      else
      {
        ErrorOutput << ProgramName << ": ";
      }
      ErrorOutput << "note: " << Note.Message;
      if (Note.Location)
      {
        printRange(ErrorOutput, Note.Location->Range);
      }
      ErrorOutput << '\n';
    }
  }

  void DiagnosticConsumer::report(const core::Diagnostic &DiagnosticEntry, const core::SourceManager &Sources)
  {
    if (!Sources.contains(DiagnosticEntry.File))
    {
      internalCompilerError("diagnostic references an unknown source file");
    }
    const core::FormattedDiagnostic Formatted = core::DiagnosticFormatter().format(DiagnosticEntry);
    DiagnosticMessage Message{Formatted.Severity, DiagnosticEntry.Kind, Formatted.Message, DiagnosticLocation{Sources.sourceFile(DiagnosticEntry.File).Path, DiagnosticEntry.Span}, {}};
    Message.Notes.reserve(Formatted.Notes.size());
    for (const core::FormattedDiagnosticNote &Note : Formatted.Notes)
    {
      std::optional<DiagnosticLocation> Location;
      if (Note.File.isValid())
      {
        if (!Sources.contains(Note.File))
        {
          internalCompilerError("diagnostic note references an unknown source file");
        }
        Location = DiagnosticLocation{Sources.sourceFile(Note.File).Path, Note.Span};
      }
      else if (Note.Span)
      {
        internalCompilerError("unlocated diagnostic note carries a source range");
      }
      Message.Notes.push_back({Note.Message, std::move(Location)});
    }
    report(Message);
  }

  void DiagnosticConsumer::reportError(std::string_view Message)
  {
    report({core::DiagnosticSeverity::Error, std::nullopt, std::string(Message), std::nullopt, {}});
  }

  void DiagnosticConsumer::flush()
  {
    ErrorOutput.flush();
  }

  bool DiagnosticConsumer::good() const noexcept
  {
    return static_cast<bool>(ErrorOutput);
  }

  InternalCompilerError::InternalCompilerError(std::string Message) : std::runtime_error(std::move(Message))
  {
  }

  [[noreturn]] void internalCompilerError(std::string Message)
  {
    throw InternalCompilerError(std::move(Message));
  }
} // namespace ink::cli
