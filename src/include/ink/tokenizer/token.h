#ifndef INK_TOKENIZER_TOKEN_H
#define INK_TOKENIZER_TOKEN_H

#include "ink/core/source_range.h"

#include <cstdint>
#include <string>
#include <variant>

namespace ink::tokenizer
{
  inline constexpr const char *UnicodeVersion = "15.1.0";

  enum class TokenKind
  {
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

  enum class KeywordKind
  {
    As,
    Async,
    Await,
    Break,
    Catch,
    Class,
    Comptime,
    Const,
    Continue,
    Decorator,
    Defer,
    Else,
    Enum,
    Extern,
    Final,
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
    Protected,
    Public,
    Return,
    Static,
    This,
    Throw,
    Try,
    Var,
    Virtual,
    While,
  };

  enum class BuiltinTypeKind
  {
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

  enum class NumericSuffix
  {
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

  enum class StringMode
  {
    EscapedSingleLine,
    RawSingleLine,
    EscapedMultiline,
    RawMultiline,
  };

  struct NumericInfo
  {
      unsigned Base = 10;
      NumericSuffix Suffix = NumericSuffix::None;
  };

  inline bool operator==(const NumericInfo &Left, const NumericInfo &Right) noexcept
  {
    return Left.Base == Right.Base && Left.Suffix == Right.Suffix;
  }

  inline bool operator!=(const NumericInfo &Left, const NumericInfo &Right) noexcept
  {
    return !(Left == Right);
  }

  struct StringInfo
  {
      StringMode Mode = StringMode::EscapedSingleLine;
      std::string Decoded;
  };

  inline bool operator==(const StringInfo &Left, const StringInfo &Right) noexcept
  {
    return Left.Mode == Right.Mode && Left.Decoded == Right.Decoded;
  }

  inline bool operator!=(const StringInfo &Left, const StringInfo &Right) noexcept
  {
    return !(Left == Right);
  }

  using TokenPayload = std::variant<std::monostate, KeywordKind, BuiltinTypeKind, bool, char, NumericInfo, char32_t, StringInfo>;

  struct Token
  {
      TokenKind Kind = TokenKind::InvalidCharacter;
      core::SourceRange Span;
      TokenPayload Payload;

      bool isTrivia() const noexcept;
      bool isError() const noexcept;
  };

  inline bool operator==(const Token &Left, const Token &Right)
  {
    return Left.Kind == Right.Kind && Left.Span == Right.Span && Left.Payload == Right.Payload;
  }

  inline bool operator!=(const Token &Left, const Token &Right)
  {
    return !(Left == Right);
  }

  bool isTrivia(TokenKind Kind) noexcept;
  bool isError(TokenKind Kind) noexcept;
  const char *tokenKindName(TokenKind Kind) noexcept;
} // namespace ink::tokenizer

#endif
