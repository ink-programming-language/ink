#include "parser_internal.h"
namespace ink::parser
{
  namespace
  {
    unsigned bindingPower(TokenKind Kind)
    {
      switch (Kind)
      {
      case TokenKind::NullCoalesce:
        return 2;
      case TokenKind::PipePipe:
        return 3;
      case TokenKind::AmpAmp:
        return 4;
      case TokenKind::Pipe:
        return 5;
      case TokenKind::Caret:
        return 6;
      case TokenKind::Amp:
        return 7;
      case TokenKind::EqualEqual:
      case TokenKind::BangEqual:
        return 8;
      case TokenKind::Less:
      case TokenKind::LessEqual:
      case TokenKind::Greater:
      case TokenKind::GreaterEqual:
        return 9;
      case TokenKind::ShiftLeft:
      case TokenKind::ShiftRight:
        return 10;
      case TokenKind::Plus:
      case TokenKind::Minus:
        return 11;
      case TokenKind::Star:
      case TokenKind::Slash:
      case TokenKind::Percent:
        return 12;
      default:
        return 0;
      }
    }
  } // namespace
  Expr *Parser::parseExpr(unsigned MinBP)
  {
    DepthScope Nesting(*this);
    if (!Nesting)
    {
      return missingExpr();
    }
    const std::size_t Start = start();
    Expr *Left = parseUnary();
    while (active())
    {
      const TokenKind Op = kind();
      if (Op == TokenKind::Question && MinBP <= 1)
      {
        bump();
        Expr *Then;
        {
          RecoveryScope Scope(*this, TokenKind::Colon);
          Then = parseExpr();
          expect(TokenKind::Colon);
        }
        Expr *Else = parseExpr(1);
        Left = make<ConditionalExpr>(range(Start), Left, Then, Else);
        continue;
      }
      const unsigned BP = bindingPower(Op);
      if (BP == 0 || BP < MinBP)
      {
        break;
      }
      const SourceRange OperatorRange = Input.token(bump()).Span;
      Expr *Right = parseExpr(BP + (Op == TokenKind::NullCoalesce ? 0 : 1));
      Left = make<BinaryExpr>(range(Start), Left, Op, OperatorRange, Right);
    }
    return Left;
  }
  Expr *Parser::parseUnary(bool TypeOnly)
  {
    DepthScope Nesting(*this);
    if (!Nesting)
    {
      return missingExpr();
    }
    const std::size_t Start = start();
    const TokenKind Op = kind();
    if (Op == TokenKind::KwFunc)
    {
      return parseFunction(TypeOnly);
    }
    if (Op == TokenKind::KwComptime)
    {
      bump();
      Expr *Operand = parseUnary(TypeOnly);
      return make<ComptimeExpr>(range(Start), Operand);
    }
    if (Op == TokenKind::Plus || Op == TokenKind::Minus || Op == TokenKind::Bang || Op == TokenKind::Tilde || Op == TokenKind::PlusPlus || Op == TokenKind::MinusMinus || Op == TokenKind::Amp || Op == TokenKind::Star)
    {
      const SourceRange OperatorRange = Input.token(bump()).Span;
      Expr *Operand = parseUnary(TypeOnly);
      return make<UnaryExpr>(range(Start), Op, OperatorRange, Operand);
    }
    return parsePostfix(parsePrimary(TypeOnly));
  }
  Expr *Parser::parsePrimary(bool TypeOnly)
  {
    (void)TypeOnly;
    const std::size_t Start = start();
    if (at(TokenKind::Identifier))
    {
      const NameToken Name = name();
      return make<NameExpr>(range(Start), Name);
    }
    if (at(TokenKind::IntegerLiteral) || at(TokenKind::FloatLiteral) || at(TokenKind::StringLiteral) || at(TokenKind::CharLiteral))
    {
      const TokenId Id = bump();
      return make<LiteralExpr>(range(Start), Id, Input.token(Id).Kind);
    }
    if (take(TokenKind::KwDo))
    {
      BlockStmt *Body = parseBlock();
      return make<BlockExpr>(range(Start), Body);
    }
    if (at(TokenKind::KwMatch))
    {
      return parseMatch();
    }
    if (at(TokenKind::LParen) || at(TokenKind::LBracket))
    {
      const bool Tuple = take(TokenKind::LParen);
      if (!Tuple)
      {
        bump();
      }
      const TokenKind End = Tuple ? TokenKind::RParen : TokenKind::RBracket;
      RecoveryScope Scope(*this, End, true);
      std::vector<Expr *> Elements;
      bool HadComma = false;
      if (!listEnd(End))
      {
        Elements.push_back(parseExpr());
        if (!Tuple && take(TokenKind::Semicolon))
        {
          Expr *Count = parseExpr();
          expect(End);
          return make<ArrayRepeatExpr>(range(Start), Elements.front(), Count);
        }
        while (active())
        {
          HadComma = HadComma || at(TokenKind::Comma);
          if (!nextElement(End, Tuple && Elements.size() == 1))
          {
            break;
          }
          const TokenId Before = Cursor.position();
          Elements.push_back(parseExpr());
          if (Cursor.position() == Before && !at(TokenKind::Comma) && !listEnd(End))
          {
            ensureProgress(Before);
          }
        }
      }
      expect(End);
      if (Tuple && Elements.size() == 1 && !HadComma)
      {
        return make<ParenExpr>(range(Start), Elements.front());
      }
      if (Tuple)
      {
        return make<TupleExpr>(range(Start), array(Elements));
      }
      return make<ArrayExpr>(range(Start), array(Elements));
    }
    if (boundary(kind()) || strongStart(kind()) || declarationStart() || at(TokenKind::Comma) || at(TokenKind::Colon) || at(TokenKind::FatArrow) || at(TokenKind::LBrace))
    {
      return missingExpr();
    }
    invalid(Cursor.peek().Span);
    bump();
    return make<ErrorExpr>(range(Start));
  }
  Expr *Parser::parsePostfix(Expr *Value)
  {
    const std::size_t Start = Value->getSourceRange().getBegin().getByteOffset();
    while (active())
    {
      bool Optional = false;
      TokenKind Access = kind();
      if (take(TokenKind::QuestionDot))
      {
        Optional = true;
      }
      if (take(TokenKind::LParen))
      {
        const auto Arguments = parseArguments(TokenKind::RParen);
        Value = make<CallExpr>(range(Start), Value, Arguments, Optional);
      }
      else if (take(TokenKind::LBracket))
      {
        RecoveryScope Scope(*this, TokenKind::RBracket);
        Expr *Index = parseExpr();
        expect(TokenKind::RBracket);
        Value = make<IndexExpr>(range(Start), Value, Index, Optional);
      }
      else if (!Optional && take(TokenKind::DoubleColon))
      {
        expect(TokenKind::LBracket);
        const auto Arguments = parseArguments(TokenKind::RBracket);
        Value = make<GenericApplyExpr>(range(Start), Value, Arguments);
      }
      else if (Optional || take(TokenKind::Dot) || take(TokenKind::Arrow))
      {
        const NameToken Member = name();
        Value = make<MemberExpr>(range(Start), Value, Access, Member);
      }
      else if (at(TokenKind::PlusPlus) || at(TokenKind::MinusMinus))
      {
        const TokenId Id = bump();
        Value = make<PostfixUpdateExpr>(range(Start), Value, Input.token(Id).Kind, Input.token(Id).Span);
      }
      else
      {
        break;
      }
    }
    return Value;
  }
  ASTArray<Argument> Parser::parseArguments(TokenKind End)
  {
    RecoveryScope Scope(*this, End, true);
    std::vector<Argument> Arguments;
    while (!listEnd(End) && active())
    {
      const TokenId Before = Cursor.position();
      const std::size_t Start = start();
      ArgumentKind Form = ArgumentKind::Positional;
      std::optional<NameToken> Name;
      if (at(TokenKind::Identifier) && kind(1) == TokenKind::Assign)
      {
        Name = name();
        bump();
        Form = ArgumentKind::Named;
      }
      Expr *Value = parseExpr();
      if (take(TokenKind::Ellipsis))
      {
        if (Form == ArgumentKind::Named)
        {
          invalid(range(Start));
        }
        else
        {
          Form = ArgumentKind::SpreadPositional;
        }
      }
      Arguments.emplace_back(Form, Name, Value, range(Start));
      if (Cursor.position() == Before && !at(TokenKind::Comma) && !listEnd(End))
      {
        ensureProgress(Before);
      }
      if (!nextElement(End))
      {
        break;
      }
    }
    expect(End);
    return array(Arguments);
  }
  Expr *Parser::parseMatch()
  {
    const std::size_t Start = start();
    bump();
    expect(TokenKind::LParen);
    Expr *Value;
    {
      RecoveryScope Scope(*this, TokenKind::RParen);
      Value = parseExpr();
      expect(TokenKind::RParen);
    }
    expect(TokenKind::LBrace);
    RecoveryScope Scope(*this, TokenKind::RBrace, true);
    std::vector<MatchArm> Arms;
    while (!listEnd(TokenKind::RBrace) && active())
    {
      const TokenId Before = Cursor.position();
      const std::size_t ArmStart = start();
      MatchPattern *Pattern;
      Expr *Guard = nullptr;
      {
        RecoveryScope ArrowScope(*this, TokenKind::FatArrow);
        Pattern = parseMatchPattern();
        if (take(TokenKind::KwIf))
        {
          expect(TokenKind::LParen);
          RecoveryScope GuardScope(*this, TokenKind::RParen);
          Guard = parseExpr();
          expect(TokenKind::RParen);
        }
        expect(TokenKind::FatArrow);
      }
      Expr *Body = parseExpr();
      Arms.emplace_back(Pattern, Guard, Body, range(ArmStart));
      if (Cursor.position() == Before && !at(TokenKind::Comma) && !listEnd(TokenKind::RBrace))
      {
        ensureProgress(Before);
      }
      if (!nextElement(TokenKind::RBrace))
      {
        break;
      }
    }
    expect(TokenKind::RBrace);
    return make<MatchExpr>(range(Start), Value, array(Arms));
  }
} // namespace ink::parser
