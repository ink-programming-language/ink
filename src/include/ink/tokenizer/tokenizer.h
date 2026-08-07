#ifndef INK_TOKENIZER_TOKENIZER_H
#define INK_TOKENIZER_TOKENIZER_H

#include "ink/tokenizer/token.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace ink::tokenizer {

enum class DiagnosticKind {
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

struct Diagnostic {
  DiagnosticKind kind = DiagnosticKind::InvalidCharacter;
  ByteSpan span;
  std::string message;
};

inline bool operator==(const Diagnostic& left, const Diagnostic& right) { return left.kind == right.kind && left.span == right.span && left.message == right.message; }
inline bool operator!=(const Diagnostic& left, const Diagnostic& right) { return !(left == right); }

struct TokenizerOptions {
  std::size_t max_block_comment_depth = 1024;
};

class LexedFile {
 public:
  const std::string& source() const noexcept { return source_; }
  const std::vector<Token>& tokens() const noexcept { return tokens_; }
  const std::vector<Diagnostic>& diagnostics() const noexcept { return diagnostics_; }
  const std::vector<std::size_t>& line_starts() const noexcept { return line_starts_; }
  std::string_view raw(const Token& token) const noexcept;
  std::size_t line_number(std::size_t byte_offset) const noexcept;
  bool succeeded() const noexcept;

 private:
  LexedFile() = default;

  std::string source_;
  std::vector<Token> tokens_;
  std::vector<Diagnostic> diagnostics_;
  std::vector<std::size_t> line_starts_;

  friend class Tokenizer;
};

class Tokenizer {
 public:
  explicit Tokenizer(TokenizerOptions options = {});
  LexedFile tokenize(std::string source) const;

 private:
  TokenizerOptions options_;
};

LexedFile tokenize(std::string source, TokenizerOptions options = {});
const char* diagnostic_kind_name(DiagnosticKind kind) noexcept;

}  // namespace ink::tokenizer

#endif
