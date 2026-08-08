#include "ink/tokenizer/token.h"

namespace ink::tokenizer
{
  bool isTrivia(TokenKind Kind) noexcept
  {
    switch (Kind)
    {
    case TokenKind::Utf8Bom:
    case TokenKind::SpacesAndTabs:
    case TokenKind::LineBreak:
    case TokenKind::LineComment:
    case TokenKind::BlockComment:
      return true;
    default:
      return false;
    }
  }

  bool isError(TokenKind Kind) noexcept
  {
    switch (Kind)
    {
    case TokenKind::InvalidEncoding:
    case TokenKind::InvalidCharacter:
    case TokenKind::InvalidIdentifier:
    case TokenKind::InvalidNumber:
    case TokenKind::InvalidScalarLiteral:
    case TokenKind::InvalidStringLiteral:
    case TokenKind::UnterminatedBlockComment:
      return true;
    default:
      return false;
    }
  }

  bool Token::isTrivia() const noexcept
  {
    return ink::tokenizer::isTrivia(Kind);
  }

  bool Token::isError() const noexcept
  {
    return ink::tokenizer::isError(Kind);
  }

  const char *tokenKindName(TokenKind Kind) noexcept
  {
    switch (Kind)
    {
    case TokenKind::Utf8Bom:
      return "Utf8Bom";
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
    case TokenKind::BuiltinType:
      return "BuiltinType";
    case TokenKind::BoolLiteral:
      return "BoolLiteral";
    case TokenKind::NullLiteral:
      return "NullLiteral";
    case TokenKind::IntegerLiteral:
      return "IntegerLiteral";
    case TokenKind::FloatLiteral:
      return "FloatLiteral";
    case TokenKind::ScalarLiteral:
      return "ScalarLiteral";
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
    case TokenKind::InvalidNumber:
      return "InvalidNumber";
    case TokenKind::InvalidScalarLiteral:
      return "InvalidScalarLiteral";
    case TokenKind::InvalidStringLiteral:
      return "InvalidStringLiteral";
    case TokenKind::UnterminatedBlockComment:
      return "UnterminatedBlockComment";
    case TokenKind::EndOfFile:
      return "EndOfFile";
    }
    return "Unknown";
  }
} // namespace ink::tokenizer
