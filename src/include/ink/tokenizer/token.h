#ifndef INK_TOKENIZER_TOKEN_H
#define INK_TOKENIZER_TOKEN_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>

namespace ink::tokenizer {

inline constexpr const char* kUnicodeVersion = "15.1.0";

struct ByteSpan {
  std::size_t start = 0;
  std::size_t end = 0;

  constexpr std::size_t size() const noexcept { return end - start; }
  constexpr bool empty() const noexcept { return start == end; }
};

constexpr bool operator==(ByteSpan left, ByteSpan right) noexcept { return left.start == right.start && left.end == right.end; }
constexpr bool operator!=(ByteSpan left, ByteSpan right) noexcept { return !(left == right); }

enum class TokenKind {
  Utf8Bom,
  SpacesAndTabs,
  LineBreak,
  LineComment,
  BlockComment,
  Identifier,
  Keyword,
  BuiltinType,
  BoolLiteral,
  NullLiteral,
  IntegerLiteral,
  FloatLiteral,
  ScalarLiteral,
  StringLiteral,
  Symbol,
  InvalidEncoding,
  InvalidCharacter,
  InvalidIdentifier,
  InvalidNumber,
  InvalidScalarLiteral,
  InvalidStringLiteral,
  UnterminatedBlockComment,
  EndOfFile,
};

enum class KeywordKind {
  As,
  Async,
  Await,
  Break,
  Catch,
  Class,
  Comptime,
  Const,
  Continue,
  Constructor,
  Decorator,
  Defer,
  Destructor,
  Else,
  Enum,
  Extern,
  For,
  From,
  Func,
  If,
  Implicit,
  Import,
  In,
  Interface,
  Let,
  Match,
  Override,
  Private,
  Return,
  This,
  Throw,
  Try,
  Var,
  Virtual,
  While,
};

enum class BuiltinTypeKind {
  I8,
  I16,
  I32,
  I64,
  I128,
  U8,
  U16,
  U32,
  U64,
  U128,
  Int,
  UInt,
  PtrSize,
  F16,
  F32,
  F64,
  Bool,
  Byte,
  Void,
  Never,
  Type,
};

enum class NumericSuffix {
  None,
  I8,
  I16,
  I32,
  I64,
  I128,
  U8,
  U16,
  U32,
  U64,
  U128,
  Int,
  UInt,
  PtrSize,
  Byte,
  F16,
  F32,
  F64,
};

enum class StringMode {
  EscapedSingleLine,
  RawSingleLine,
  EscapedMultiline,
  RawMultiline,
};

struct NumericInfo {
  unsigned base = 10;
  NumericSuffix suffix = NumericSuffix::None;
};

inline bool operator==(const NumericInfo& left, const NumericInfo& right) noexcept { return left.base == right.base && left.suffix == right.suffix; }
inline bool operator!=(const NumericInfo& left, const NumericInfo& right) noexcept { return !(left == right); }

struct StringInfo {
  StringMode mode = StringMode::EscapedSingleLine;
  std::string decoded;
};

inline bool operator==(const StringInfo& left, const StringInfo& right) noexcept { return left.mode == right.mode && left.decoded == right.decoded; }
inline bool operator!=(const StringInfo& left, const StringInfo& right) noexcept { return !(left == right); }

using TokenPayload = std::variant<std::monostate, KeywordKind, BuiltinTypeKind, bool, char, NumericInfo, char32_t, StringInfo>;

struct Token {
  TokenKind kind = TokenKind::InvalidCharacter;
  ByteSpan span;
  TokenPayload payload;

  bool is_trivia() const noexcept;
  bool is_error() const noexcept;
};

inline bool operator==(const Token& left, const Token& right) { return left.kind == right.kind && left.span == right.span && left.payload == right.payload; }
inline bool operator!=(const Token& left, const Token& right) { return !(left == right); }

bool is_trivia(TokenKind kind) noexcept;
bool is_error(TokenKind kind) noexcept;
const char* token_kind_name(TokenKind kind) noexcept;

}  // namespace ink::tokenizer

#endif
