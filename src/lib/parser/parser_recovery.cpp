#include "parser_internal.h"
namespace ink::parser
{
  bool Parser::declarationStart(std::size_t Ahead) const
  {
    const TokenKind Kind = kind(Ahead);
    return Kind == TokenKind::KwVar || Kind == TokenKind::KwConst || Kind == TokenKind::KwField || Kind == TokenKind::KwClass || Kind == TokenKind::KwEnum || Kind == TokenKind::KwInterface || (Kind == TokenKind::KwFunc && kind(Ahead + 1) == TokenKind::Identifier);
  }
  bool Parser::strongStart(TokenKind Kind) const
  {
    switch (Kind)
    {
    case TokenKind::KwVar:
    case TokenKind::KwConst:
    case TokenKind::KwField:
    case TokenKind::KwClass:
    case TokenKind::KwEnum:
    case TokenKind::KwInterface:
    case TokenKind::KwIf:
    case TokenKind::KwWhile:
    case TokenKind::KwFor:
    case TokenKind::KwSwitch:
    case TokenKind::KwReturn:
    case TokenKind::KwYield:
    case TokenKind::KwBreak:
    case TokenKind::KwContinue:
    case TokenKind::KwDefer:
    case TokenKind::KwImport:
    case TokenKind::KwFrom:
      return true;
    default:
      return false;
    }
  }
  bool Parser::boundary(TokenKind Kind) const
  {
    if (Kind == TokenKind::EndOfFile || Kind == TokenKind::RBrace || Kind == TokenKind::RParen || Kind == TokenKind::RBracket || Kind == TokenKind::Semicolon || Kind == TokenKind::KwElse || Kind == TokenKind::KwCase || Kind == TokenKind::KwDefault)
    {
      return true;
    }
    for (const RecoveryPolicy &Policy : Recovery.Scopes)
    {
      if (Kind == Policy.End || (Policy.Comma && Kind == TokenKind::Comma))
      {
        return true;
      }
    }
    return false;
  }
  bool Parser::listEnd(TokenKind End) const
  {
    return at(End) || (boundary(kind()) && !at(TokenKind::Comma)) || strongStart(kind()) || declarationStart();
  }
  bool Parser::expressionStart(TokenKind Kind) const
  {
    switch (Kind)
    {
    case TokenKind::Identifier:
    case TokenKind::IntegerLiteral:
    case TokenKind::FloatLiteral:
    case TokenKind::StringLiteral:
    case TokenKind::CharLiteral:
    case TokenKind::LParen:
    case TokenKind::LBracket:
    case TokenKind::KwFunc:
    case TokenKind::KwMatch:
    case TokenKind::KwDo:
    case TokenKind::KwComptime:
    case TokenKind::Plus:
    case TokenKind::Minus:
    case TokenKind::Bang:
    case TokenKind::Tilde:
    case TokenKind::PlusPlus:
    case TokenKind::MinusMinus:
    case TokenKind::Amp:
    case TokenKind::Star:
      return true;
    default:
      return false;
    }
  }
  SourceRange Parser::recover(TokenKind Expected)
  {
    const std::size_t Start = start();
    std::vector<TokenKind> Ends;
    while (!at(TokenKind::EndOfFile) && active())
    {
      const TokenKind Kind = kind();
      if (Ends.empty() && (Kind == Expected || boundary(Kind) || strongStart(Kind) || declarationStart() || Kind == TokenKind::LBrace))
      {
        break;
      }
      if (Kind == TokenKind::LParen)
      {
        Ends.push_back(TokenKind::RParen);
      }
      else if (Kind == TokenKind::LBracket)
      {
        Ends.push_back(TokenKind::RBracket);
      }
      else if (Kind == TokenKind::LBrace)
      {
        Ends.push_back(TokenKind::RBrace);
      }
      else if (Kind == TokenKind::RParen || Kind == TokenKind::RBracket || Kind == TokenKind::RBrace)
      {
        if (Ends.empty() || Ends.back() != Kind)
        {
          break;
        }
        Ends.pop_back();
      }
      if (!Ends.empty() && (strongStart(Kind) || Kind == TokenKind::KwElse || Kind == TokenKind::KwCase || Kind == TokenKind::KwDefault))
      {
        break;
      }
      bump();
    }
    return range(Start);
  }
  ExpectResult Parser::expect(TokenKind Kind)
  {
    if (at(Kind))
    {
      const TokenId Id = bump();
      return {Kind, Id, Input.token(Id).Span, ExpectStatus::Matched};
    }
    const SourceRange Insertion = point();
    if (Strict)
    {
      TrialFailed = true;
      return {Kind, {}, Insertion, ExpectStatus::Inserted};
    }
    if (Status == ParseStatus::Completed && (Input.lexedFile().succeeded() || !at(TokenKind::EndOfFile)))
    {
      Diagnostics.report(core::makeDiagnostic<core::DiagnosticKind::ParserExpectedToken>(Insertion, tokenizer::tokenKindName(Kind)));
    }
    ExpectResult Result{Kind, {}, Insertion, ExpectStatus::Inserted};
    std::optional<SourceRange> Skipped;
    // A caller's terminator or the start of the next structure always wins over
    // single-token deletion. Missing commas also preserve the next element.
    if (!boundary(kind()) && !strongStart(kind()) && !declarationStart() && kind() != TokenKind::LBrace && !(Kind == TokenKind::Comma && expressionStart(kind())))
    {
      if (kind(1) == Kind)
      {
        Skipped = Cursor.peek().Span;
        bump();
      }
      else if (Kind != TokenKind::Identifier && Kind != TokenKind::LParen && Kind != TokenKind::LBrace && Kind != TokenKind::Colon && Kind != TokenKind::Assign && Kind != TokenKind::FatArrow)
      {
        Skipped = recover(Kind);
      }
      if (at(Kind))
      {
        const TokenId Id = bump();
        Result = {Kind, Id, Input.token(Id).Span, ExpectStatus::Recovered};
      }
    }
    LastEnd = std::max(LastEnd, Insertion.getEnd().getByteOffset());
    RecoveryInfo.Entries.push_back({nullptr, Result, Skipped});
    return Result;
  }
  bool Parser::nextElement(TokenKind End, bool AllowSingleTrailing)
  {
    if (take(TokenKind::Comma))
    {
      if (at(End))
      {
        if (!AllowSingleTrailing)
        {
          invalid(point());
        }
        return false;
      }
      return !listEnd(End);
    }
    if (listEnd(End))
    {
      return false;
    }
    expect(TokenKind::Comma);
    return !listEnd(End) && active();
  }
} // namespace ink::parser
