#include "ink/tokenizer/token.h"

#include <cassert>

#include <llvm/ADT/StringSwitch.h>

namespace ink::tokenizer
{
  namespace
  {
    struct SymbolEntry
    {
        std::string_view Spelling;
        SymbolKind Kind;
    };

    constexpr SymbolEntry Symbols[] = {
#define INK_SYMBOL(Name, Spelling) {Spelling, SymbolKind::Name},
#include "ink/tokenizer/token.def"
#undef INK_SYMBOL
    };
  } // namespace

  bool isTrivia(TokenKind Kind) noexcept
  {
    return Kind == TokenKind::SpacesAndTabs || Kind == TokenKind::LineBreak || Kind == TokenKind::LineComment || Kind == TokenKind::BlockComment;
  }

  bool isError(TokenKind Kind) noexcept
  {
    return Kind == TokenKind::InvalidEncoding || Kind == TokenKind::InvalidCharacter || Kind == TokenKind::InvalidIdentifier || Kind == TokenKind::InvalidStringLiteral || Kind == TokenKind::UnterminatedBlockComment;
  }

  bool Token::isTrivia() const noexcept
  {
    return ink::tokenizer::isTrivia(Kind);
  }

  KeywordKind Token::keyword() const noexcept
  {
    const KeywordKind *Value = std::get_if<KeywordKind>(&Payload);
    assert(is(TokenKind::Keyword) && Value != nullptr);
    return *Value;
  }

  SymbolKind Token::symbol() const noexcept
  {
    const SymbolKind *Value = std::get_if<SymbolKind>(&Payload);
    assert(is(TokenKind::Symbol) && Value != nullptr);
    return *Value;
  }

  bool Token::isError() const noexcept
  {
    return ink::tokenizer::isError(Kind);
  }

  const char *tokenKindName(TokenKind Kind) noexcept
  {
    switch (Kind)
    {
    case TokenKind::SpacesAndTabs:
      return "SpacesAndTabs";
    case TokenKind::LineBreak:
      return "LineBreak";
    case TokenKind::LineComment:
      return "LineComment";
    case TokenKind::BlockComment:
      return "BlockComment";
    case TokenKind::Identifier:
      return "Identifier";
    case TokenKind::Keyword:
      return "Keyword";
    case TokenKind::IntegerLiteral:
      return "IntegerLiteral";
    case TokenKind::FloatLiteral:
      return "FloatLiteral";
    case TokenKind::StringLiteral:
      return "StringLiteral";
    case TokenKind::Symbol:
      return "Symbol";
    case TokenKind::InvalidEncoding:
      return "InvalidEncoding";
    case TokenKind::InvalidCharacter:
      return "InvalidCharacter";
    case TokenKind::InvalidIdentifier:
      return "InvalidIdentifier";
    case TokenKind::InvalidStringLiteral:
      return "InvalidStringLiteral";
    case TokenKind::UnterminatedBlockComment:
      return "UnterminatedBlockComment";
    case TokenKind::EndOfFile:
      return "EndOfFile";
    }
    return "Unknown";
  }

  std::optional<KeywordKind> lookupKeyword(std::string_view Spelling) noexcept
  {
    return llvm::StringSwitch<std::optional<KeywordKind>>(Spelling)
#define INK_KEYWORD(Name, Spelling) .Case(Spelling, KeywordKind::Name)
#include "ink/tokenizer/token.def"
#undef INK_KEYWORD
        .Default(std::nullopt);
  }

  std::optional<SymbolKind> lookupSymbol(std::string_view Spelling) noexcept
  {
    return llvm::StringSwitch<std::optional<SymbolKind>>(Spelling)
#define INK_SYMBOL(Name, Spelling) .Case(Spelling, SymbolKind::Name)
#include "ink/tokenizer/token.def"
#undef INK_SYMBOL
        .Default(std::nullopt);
  }

  std::optional<SymbolKind> matchSymbolPrefix(std::string_view Source) noexcept
  {
    std::optional<SymbolKind> Result;
    std::size_t Length = 0;
    for (const SymbolEntry &Entry : Symbols)
    {
      if (Entry.Spelling.size() > Length && Source.size() >= Entry.Spelling.size() && Source.compare(0, Entry.Spelling.size(), Entry.Spelling) == 0)
      {
        Result = Entry.Kind;
        Length = Entry.Spelling.size();
      }
    }
    return Result;
  }

  std::string_view keywordSpelling(KeywordKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_KEYWORD(Name, Spelling) \
  case KeywordKind::Name:           \
    return Spelling;
#include "ink/tokenizer/token.def"
#undef INK_KEYWORD
    }
    return {};
  }

  std::string_view symbolSpelling(SymbolKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_SYMBOL(Name, Spelling) \
  case SymbolKind::Name:           \
    return Spelling;
#include "ink/tokenizer/token.def"
#undef INK_SYMBOL
    }
    return {};
  }

  std::string_view symbolSpelling(const Token &Token) noexcept
  {
    if (Token.Kind == TokenKind::Symbol)
    {
      if (const SymbolKind *Kind = std::get_if<SymbolKind>(&Token.Payload))
      {
        return symbolSpelling(*Kind);
      }
    }
    return {};
  }
} // namespace ink::tokenizer
