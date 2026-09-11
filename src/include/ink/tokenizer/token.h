#ifndef INK_TOKENIZER_TOKEN_H
#define INK_TOKENIZER_TOKEN_H

#include "ink/core/source_range.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace ink::tokenizer
{
  inline constexpr const char *UnicodeVersion = "15.1.0";

  enum class TokenKind
  {
    SpacesAndTabs,
    LineBreak,
    LineComment,
    BlockComment,
    Identifier,
    Keyword,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    Symbol,
    InvalidEncoding,
    InvalidCharacter,
    InvalidIdentifier,
    InvalidStringLiteral,
    UnterminatedBlockComment,
    EndOfFile,
  };

  enum class KeywordKind
  {
#define INK_KEYWORD(Name, Spelling) Name,
#include "ink/tokenizer/token.def"
#undef INK_KEYWORD
  };

  enum class SymbolKind
  {
#define INK_SYMBOL(Name, Spelling) Name,
#include "ink/tokenizer/token.def"
#undef INK_SYMBOL
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
  };

  inline bool operator==(const NumericInfo &Left, const NumericInfo &Right) noexcept
  {
    return Left.Base == Right.Base;
  }

  inline bool operator!=(const NumericInfo &Left, const NumericInfo &Right) noexcept
  {
    return !(Left == Right);
  }

  struct StringInfo
  {
      StringMode Mode = StringMode::EscapedSingleLine;
      // Escapes decode to Unicode scalars encoded as UTF-8; hexadecimal escapes designate U+00NN.
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

  using TokenPayload = std::variant<std::monostate, KeywordKind, SymbolKind, NumericInfo, StringInfo>;

  struct Token
  {
      TokenKind Kind = TokenKind::InvalidCharacter;
      core::SourceRange Span;
      TokenPayload Payload;

      bool is(TokenKind Expected) const noexcept
      {
        return Kind == Expected;
      }

      bool is(KeywordKind Expected) const noexcept
      {
        const KeywordKind *Value = std::get_if<KeywordKind>(&Payload);
        return is(TokenKind::Keyword) && Value != nullptr && *Value == Expected;
      }

      bool is(SymbolKind Expected) const noexcept
      {
        const SymbolKind *Value = std::get_if<SymbolKind>(&Payload);
        return is(TokenKind::Symbol) && Value != nullptr && *Value == Expected;
      }

      template <typename... Kinds>
      bool isOneOf(Kinds... Expected) const noexcept
      {
        static_assert(sizeof...(Kinds) != 0, "At least one token kind is required");
        return (is(Expected) || ...);
      }

      KeywordKind keyword() const noexcept;
      SymbolKind symbol() const noexcept;
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
  std::optional<KeywordKind> lookupKeyword(std::string_view Spelling) noexcept;
  std::optional<SymbolKind> lookupSymbol(std::string_view Spelling) noexcept;
  // Finds the longest symbol at the beginning of the remaining source bytes.
  std::optional<SymbolKind> matchSymbolPrefix(std::string_view Source) noexcept;
  std::string_view keywordSpelling(KeywordKind Kind) noexcept;
  std::string_view symbolSpelling(SymbolKind Kind) noexcept;
  std::string_view symbolSpelling(const Token &Token) noexcept;
} // namespace ink::tokenizer

#endif
