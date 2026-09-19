#include "ink/tokenizer/token.h"

namespace ink::tokenizer
{
  namespace
  {
    struct SpellingEntry
    {
        TokenKind Kind;
        std::string_view Spelling;
    };

    constexpr SpellingEntry Keywords[] = {
#define INK_KEYWORD(Name, DisplayName, Spelling) {TokenKind::Name, Spelling},
#include "ink/tokenizer/token.def"
#undef INK_KEYWORD
    };

    constexpr SpellingEntry Symbols[] = {
#define INK_SYMBOL(Name, DisplayName, Spelling) {TokenKind::Name, Spelling},
#include "ink/tokenizer/token.def"
#undef INK_SYMBOL
    };
  } // namespace

  const char *tokenKindName(TokenKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_TOKEN(Name, DisplayName) \
  case TokenKind::Name:              \
    return DisplayName;
#define INK_KEYWORD(Name, DisplayName, Spelling) INK_TOKEN(Name, DisplayName)
#define INK_SYMBOL(Name, DisplayName, Spelling) INK_TOKEN(Name, DisplayName)
#include "ink/tokenizer/token.def"
#undef INK_SYMBOL
#undef INK_KEYWORD
#undef INK_TOKEN
    }
    return "UNKNOWN";
  }

  std::string_view tokenSpelling(TokenKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_KEYWORD(Name, DisplayName, Spelling) \
  case TokenKind::Name:                          \
    return Spelling;
#define INK_SYMBOL(Name, DisplayName, Spelling) INK_KEYWORD(Name, DisplayName, Spelling)
#include "ink/tokenizer/token.def"
#undef INK_SYMBOL
#undef INK_KEYWORD
    default:
      return {};
    }
  }

  bool isKeyword(TokenKind Kind) noexcept
  {
    return Kind >= TokenKind::KwAs && Kind <= TokenKind::KwYield;
  }

  bool isSymbol(TokenKind Kind) noexcept
  {
    return Kind >= TokenKind::LParen && Kind <= TokenKind::FatArrow;
  }

  std::optional<TokenKind> lookupKeyword(std::string_view Spelling) noexcept
  {
    for (const SpellingEntry &Entry : Keywords)
    {
      if (Entry.Spelling == Spelling)
      {
        return Entry.Kind;
      }
    }
    return std::nullopt;
  }

  std::optional<TokenKind> lookupSymbol(std::string_view Spelling) noexcept
  {
    for (const SpellingEntry &Entry : Symbols)
    {
      if (Entry.Spelling == Spelling)
      {
        return Entry.Kind;
      }
    }
    return std::nullopt;
  }

  std::optional<TokenKind> matchSymbolPrefix(std::string_view Source) noexcept
  {
    std::optional<TokenKind> Result;
    std::size_t Length = 0;
    for (const SpellingEntry &Entry : Symbols)
    {
      if (Entry.Spelling.size() > Length && Source.size() >= Entry.Spelling.size() && Source.compare(0, Entry.Spelling.size(), Entry.Spelling) == 0)
      {
        Result = Entry.Kind;
        Length = Entry.Spelling.size();
      }
    }
    return Result;
  }
} // namespace ink::tokenizer
