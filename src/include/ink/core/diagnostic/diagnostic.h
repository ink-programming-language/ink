#ifndef INK_CORE_DIAGNOSTIC_H
#define INK_CORE_DIAGNOSTIC_H

#include "ink/core/source/source_id.h"
#include "ink/core/source/source_range.h"

#include <cstdint>
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
    Core = 0x08,
  };

  enum class DiagnosticClass : std::uint8_t
  {
    Unknown,
    User,                  // Incorrect user source code.
    InternalCompilerError, // Every other failure, including limits and invalid artifacts.
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
    Expected,
    Actual,
    Description,
    TypeName,
    SymbolName,
    FunctionName,
    ModuleName,
    ExpectedCount,
    ActualCount,
    Size,
    ActualValue,
  };

  enum class DiagnosticArgumentType : std::uint8_t
  {
    Boolean,
    SignedInteger,
    UnsignedInteger,
    Character,
    String,
    SourceContext,
  };

  enum class DiagnosticRelatedKind : std::uint8_t
  {
    Unknown,
    MostRecentUnclosedBlockComment,
    MostRecentBlockCommentOpeningUnavailable,
    PreviousVisibleCharacter,
    NextVisibleCharacter,
  };

  enum class DiagnosticKind : std::uint32_t
  {
#define INK_DIAGNOSTIC(Name, Number, Domain, Code, Class, DefaultSeverity, DefaultMessage, FormatPattern, ArgumentSchema) Name = Number,
#include "ink/core/diagnostic/diagnostic.def"
#undef INK_DIAGNOSTIC
  };

  std::uint32_t diagnosticNumber(DiagnosticKind Kind) noexcept;
  const char *diagnosticCode(DiagnosticKind Kind) noexcept;
  const char *diagnosticKindName(DiagnosticKind Kind) noexcept;
  const char *diagnosticDefaultMessage(DiagnosticKind Kind) noexcept;
  const char *diagnosticFormatPattern(DiagnosticKind Kind) noexcept;
  DiagnosticDomain diagnosticDomain(DiagnosticKind Kind) noexcept;
  DiagnosticClass diagnosticClass(DiagnosticKind Kind) noexcept;
  DiagnosticSeverity diagnosticDefaultSeverity(DiagnosticKind Kind) noexcept;
  const char *diagnosticClassName(DiagnosticClass Class) noexcept;
  const char *diagnosticSeverityName(DiagnosticSeverity Severity) noexcept;
  const char *diagnosticArgumentName(DiagnosticArgumentName Name) noexcept;

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
      SourceId Source;
      SourceRange Span;
      std::vector<DiagnosticArgument> Arguments;
      std::vector<DiagnosticRelatedInformation> Related;
      DiagnosticClass Class = DiagnosticClass::Unknown;

      std::uint32_t number() const noexcept;
      const char *code() const noexcept;
      DiagnosticClass classification() const noexcept;
  };

  bool operator==(const Diagnostic &Left, const Diagnostic &Right);
  bool operator!=(const Diagnostic &Left, const Diagnostic &Right);
} // namespace ink::core

#endif
