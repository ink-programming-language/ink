#ifndef INK_CORE_DIAGNOSTIC_H
#define INK_CORE_DIAGNOSTIC_H

#include "ink/core/source_range.h"

#include <string>

namespace ink::core
{
  enum class DiagnosticKind
  {
    Unknown,
    InvalidUtf8,
    UnexpectedBom,
    LoneCarriageReturn,
    NonAsciiWhitespace,
    ForbiddenControlCharacter,
    InvalidCharacter,
    IdentifierNotNfc,
    InvisibleCharacter,
    MissingBaseDigits,
    DigitOutOfRange,
    MisplacedNumericSeparator,
    MissingExponentDigits,
    UnknownNumericSuffix,
    InvalidNumericSuffix,
    UnsupportedNonDecimalFloat,
    EmptyScalarLiteral,
    MultipleScalarValues,
    UnterminatedScalarLiteral,
    UnknownEscape,
    InvalidHexEscape,
    InvalidUnicodeEscape,
    InvalidUnicodeScalar,
    UnterminatedStringLiteral,
    MultilineOpeningLineBreakRequired,
    UnterminatedMultilineStringLiteral,
    InvalidMultilineIndentation,
    UnterminatedBlockComment,
    BlockCommentNestingLimit,
  };

  struct Diagnostic
  {
    DiagnosticKind Kind = DiagnosticKind::Unknown;
    SourceRange Span;
    std::string Message;
  };

  bool operator==(const Diagnostic &Left, const Diagnostic &Right);
  bool operator!=(const Diagnostic &Left, const Diagnostic &Right);
  const char *diagnosticKindName(DiagnosticKind Kind) noexcept;
} // namespace ink::core

#endif
