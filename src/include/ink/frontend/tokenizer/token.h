#ifndef INK_FRONTEND_TOKENIZER_TOKEN_H
#define INK_FRONTEND_TOKENIZER_TOKEN_H

#include "ink/core/config/numeric_literal.h"
#include "ink/core/source/source_range.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace ink::frontend::tokenizer
{
  inline constexpr const char *UnicodeVersion = "15.1.0";

  enum class TokenKind
  {
#define INK_TOKEN(Name, DisplayName) Name,
#define INK_KEYWORD(Name, DisplayName, Spelling) Name,
#define INK_SYMBOL(Name, DisplayName, Spelling) Name,
#include "ink/frontend/tokenizer/token.def"
#undef INK_SYMBOL
#undef INK_KEYWORD
#undef INK_TOKEN
  };

  struct IdentifierInfo
  {
      std::string Name;
  };

  struct NumericInfo
  {
      core::NumericBase Base = core::NumericBase::Decimal;

      constexpr bool hasValidBase() const noexcept
      {
        return Base == core::NumericBase::Binary || Base == core::NumericBase::Octal || Base == core::NumericBase::Decimal || Base == core::NumericBase::Hexadecimal;
      }
  };

  enum class StringMode
  {
    EscapedSingleLine,
    RawSingleLine,
    EscapedMultiline,
    RawMultiline,
  };

  struct StringInfo
  {
      StringMode Mode = StringMode::EscapedSingleLine;
      // Unicode scalar values encoded as UTF-8, including escaped NUL.
      std::string Decoded;
  };

  struct CharInfo
  {
      char32_t Value = 0;
      bool Raw = false;
  };

  using TokenPayload = std::variant<std::monostate, IdentifierInfo, NumericInfo, StringInfo, CharInfo>;

  struct Token
  {
      TokenKind Kind = TokenKind::EndOfFile;
      core::SourceRange Span;
      TokenPayload Payload;

      bool is(TokenKind Expected) const noexcept
      {
        return Kind == Expected;
      }

      template <typename... Kinds>
      bool isOneOf(Kinds... Expected) const noexcept
      {
        static_assert(sizeof...(Kinds) != 0, "At least one token kind is required");
        return (is(Expected) || ...);
      }

      core::SourceLocation getLocation() const noexcept
      {
        return Span.getBegin();
      }

      core::SourceRange getSourceRange() const noexcept
      {
        return Span;
      }

      std::size_t getLength() const noexcept
      {
        return Span.size();
      }
  };

  const char *tokenKindName(TokenKind Kind) noexcept;
  std::string_view tokenSpelling(TokenKind Kind) noexcept;
  bool isKeyword(TokenKind Kind) noexcept;
  bool isSymbol(TokenKind Kind) noexcept;
  std::optional<TokenKind> lookupKeyword(std::string_view Spelling) noexcept;
  std::optional<TokenKind> lookupSymbol(std::string_view Spelling) noexcept;
  std::optional<TokenKind> matchSymbolPrefix(std::string_view Source) noexcept;
} // namespace ink::frontend::tokenizer

#endif
