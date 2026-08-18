#ifndef INK_LIB_IR_TEXT_TOKEN_H
#define INK_LIB_IR_TEXT_TOKEN_H

#include "ink/core/source_range.h"

#include <cstdint>
#include <string>

namespace ink::ir::text
{
  enum class TokenKind : std::uint8_t
  {
    Identifier,
    GlobalName,
    TypeName,
    ValueName,
    Integer,
    Hexadecimal,
    String,
    LeftParenthesis,
    RightParenthesis,
    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    Comma,
    Colon,
    Equal,
    Star,
    End,
  };

  struct Token
  {
      TokenKind Kind = TokenKind::End;
      std::string Text;
      core::SourceRange Span;
  };
} // namespace ink::ir::text

#endif
