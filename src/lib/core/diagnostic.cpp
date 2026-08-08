#include "ink/core/diagnostic.h"

namespace ink::core
{
  bool operator==(const Diagnostic &Left, const Diagnostic &Right)
  {
    return Left.Kind == Right.Kind && Left.Span == Right.Span && Left.Message == Right.Message;
  }

  bool operator!=(const Diagnostic &Left, const Diagnostic &Right)
  {
    return !(Left == Right);
  }

  const char *diagnosticKindName(DiagnosticKind Kind) noexcept
  {
    switch (Kind)
    {
    case DiagnosticKind::Unknown:
      return "unknown diagnostic";
    case DiagnosticKind::InvalidUtf8:
      return "invalid UTF-8";
    case DiagnosticKind::UnexpectedBom:
      return "UTF-8 BOM is only allowed at the start of a file";
    case DiagnosticKind::LoneCarriageReturn:
      return "carriage return must be followed by line feed";
    case DiagnosticKind::NonAsciiWhitespace:
      return "only ASCII space and tab are source whitespace";
    case DiagnosticKind::ForbiddenControlCharacter:
      return "forbidden raw control character";
    case DiagnosticKind::InvalidCharacter:
      return "character cannot start an Ink token";
    case DiagnosticKind::IdentifierNotNfc:
      return "identifier is not in Unicode NFC";
    case DiagnosticKind::InvisibleCharacter:
      return "invisible format character must be written explicitly";
    case DiagnosticKind::MissingBaseDigits:
      return "base prefix must be followed by a digit";
    case DiagnosticKind::DigitOutOfRange:
      return "digit does not belong to the literal base";
    case DiagnosticKind::MisplacedNumericSeparator:
      return "numeric separator must be between two digits";
    case DiagnosticKind::MissingExponentDigits:
      return "exponent must contain a decimal digit";
    case DiagnosticKind::UnknownNumericSuffix:
      return "unknown numeric literal suffix";
    case DiagnosticKind::InvalidNumericSuffix:
      return "numeric suffix is not valid for this literal";
    case DiagnosticKind::UnsupportedNonDecimalFloat:
      return "non-decimal floating-point literals are not supported";
    case DiagnosticKind::EmptyScalarLiteral:
      return "scalar literal is empty";
    case DiagnosticKind::MultipleScalarValues:
      return "scalar literal must contain exactly one Unicode scalar value";
    case DiagnosticKind::UnterminatedScalarLiteral:
      return "scalar literal is not terminated on this source line";
    case DiagnosticKind::UnknownEscape:
      return "unknown escape sequence";
    case DiagnosticKind::InvalidHexEscape:
      return "hex escape requires exactly two hexadecimal digits";
    case DiagnosticKind::InvalidUnicodeEscape:
      return "Unicode escape requires one to six hexadecimal digits in braces";
    case DiagnosticKind::InvalidUnicodeScalar:
      return "escape does not designate a Unicode scalar value";
    case DiagnosticKind::UnterminatedStringLiteral:
      return "single-line string is not terminated on this source line";
    case DiagnosticKind::MultilineOpeningLineBreakRequired:
      return "multiline string opening delimiter must be followed by a line break";
    case DiagnosticKind::UnterminatedMultilineStringLiteral:
      return "multiline string has no closing delimiter";
    case DiagnosticKind::InvalidMultilineIndentation:
      return "multiline string line does not match the closing indentation";
    case DiagnosticKind::UnterminatedBlockComment:
      return "block comment is not terminated";
    case DiagnosticKind::BlockCommentNestingLimit:
      return "block comment nesting limit exceeded";
    }
    return "unknown diagnostic";
  }
} // namespace ink::core
