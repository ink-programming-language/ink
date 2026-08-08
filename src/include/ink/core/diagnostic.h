#ifndef INK_CORE_DIAGNOSTIC_H
#define INK_CORE_DIAGNOSTIC_H

#include "ink/core/source_range.h"

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ink::core
{
  enum class DiagnosticDomain : std::uint8_t
  {
    Unknown = 0x00,
    Tokenizer = 0x01,
    Parser = 0x02,
    Semantic = 0x03,
  };

  enum class DiagnosticSeverity : std::uint8_t
  {
    Unknown,
    Error,
    Warning,
    Note,
  };

  enum class DiagnosticSourceContext : std::uint8_t
  {
    Unknown,
    SourceText,
    Identifier,
  };

  enum class DiagnosticArgumentName : std::uint8_t
  {
    Unknown,
    Character,
    Context,
    RemainingNestingDepth,
    MostRecentOpeningUnavailable,
  };

  enum class DiagnosticRelatedKind : std::uint8_t
  {
    Unknown,
    MostRecentUnclosedBlockComment,
    PreviousVisibleCharacter,
    NextVisibleCharacter,
  };

  enum class DiagnosticKind : std::uint32_t
  {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, DefaultSeverity, DefaultMessage) Name = Number,
#include "ink/core/diagnostic.def"
#undef INK_DIAGNOSTIC
  };

  std::uint32_t diagnosticNumber(DiagnosticKind Kind) noexcept;
  const char *diagnosticCode(DiagnosticKind Kind) noexcept;
  const char *diagnosticKindName(DiagnosticKind Kind) noexcept;
  const char *diagnosticDefaultMessage(DiagnosticKind Kind) noexcept;
  DiagnosticDomain diagnosticDomain(DiagnosticKind Kind) noexcept;
  DiagnosticSeverity diagnosticDefaultSeverity(DiagnosticKind Kind) noexcept;
  const char *diagnosticSeverityName(DiagnosticSeverity Severity) noexcept;

  using DiagnosticArgumentValue = std::variant<bool, std::int64_t, std::uint64_t, char32_t, std::string, DiagnosticSourceContext>;

  struct DiagnosticArgument
  {
    DiagnosticArgumentName Name = DiagnosticArgumentName::Unknown;
    DiagnosticArgumentValue Value;
  };

  bool operator==(const DiagnosticArgument &Left, const DiagnosticArgument &Right);
  bool operator!=(const DiagnosticArgument &Left, const DiagnosticArgument &Right);

  struct DiagnosticRelatedInformation
  {
    DiagnosticRelatedKind Kind = DiagnosticRelatedKind::Unknown;
    SourceRange Span;
    std::vector<DiagnosticArgument> Arguments;
  };

  bool operator==(const DiagnosticRelatedInformation &Left, const DiagnosticRelatedInformation &Right);
  bool operator!=(const DiagnosticRelatedInformation &Left, const DiagnosticRelatedInformation &Right);

  struct Diagnostic
  {
    DiagnosticKind Kind = DiagnosticKind::Unknown;
    SourceRange Span;
    std::vector<DiagnosticArgument> Arguments;
    std::vector<DiagnosticRelatedInformation> Related;

    std::uint32_t number() const noexcept;
    const char *code() const noexcept;
  };

  bool operator==(const Diagnostic &Left, const Diagnostic &Right);
  bool operator!=(const Diagnostic &Left, const Diagnostic &Right);

  class DiagnosticBuilder
  {
  public:
    DiagnosticBuilder(DiagnosticKind Kind, SourceRange Span);
    DiagnosticBuilder &argument(DiagnosticArgumentName Name, DiagnosticArgumentValue Value) &;
    DiagnosticBuilder &&argument(DiagnosticArgumentName Name, DiagnosticArgumentValue Value) &&;
    DiagnosticBuilder &related(DiagnosticRelatedKind Kind, SourceRange Span, std::vector<DiagnosticArgument> Arguments = {}) &;
    DiagnosticBuilder &&related(DiagnosticRelatedKind Kind, SourceRange Span, std::vector<DiagnosticArgument> Arguments = {}) &&;
    Diagnostic build() &&;

  private:
    Diagnostic Result;
  };

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
