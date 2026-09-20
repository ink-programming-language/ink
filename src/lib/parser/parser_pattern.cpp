#include "parser_internal.h"
namespace ink::parser
{
  BindingPattern *Parser::parseBindingPattern()
  {
    DepthScope Nesting(*this);
    if (!Nesting)
    {
      return make<MissingBindingPattern>(point());
    }
    const std::size_t Start = start();
    if (at(TokenKind::Identifier))
    {
      const NameToken Name = name();
      return make<NameBindingPattern>(range(Start), Name);
    }
    if (take(TokenKind::Underscore))
    {
      return make<WildcardBindingPattern>(range(Start));
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
      std::vector<BindingPattern *> Elements;
      std::optional<RestBinding> Rest;
      bool HadComma = false;
      while (!listEnd(End) && active())
      {
        const TokenId Before = Cursor.position();
        if (!Tuple && (at(TokenKind::Identifier) || at(TokenKind::Underscore)) && kind(1) == TokenKind::Ellipsis)
        {
          const bool Wildcard = at(TokenKind::Underscore);
          const TokenId Id = bump();
          const SourceRange Ellipsis = Input.token(bump()).Span;
          Rest = RestBinding{{Id, Input.spelling(Id), Input.token(Id).Span}, Wildcard, Ellipsis};
          break;
        }
        Elements.push_back(parseBindingPattern());
        HadComma = HadComma || at(TokenKind::Comma);
        if (Before == Cursor.position() && !at(TokenKind::Comma) && !listEnd(End))
        {
          ensureProgress(Before);
        }
        if (!nextElement(End, Tuple && Elements.size() == 1))
        {
          break;
        }
      }
      if (Tuple && (!HadComma || Elements.empty()))
      {
        invalid(range(Start));
      }
      expect(End);
      if (Tuple)
      {
        return make<TupleBindingPattern>(range(Start), array(Elements));
      }
      return make<ArrayBindingPattern>(range(Start), array(Elements), Rest);
    }
    invalid(point());
    if (boundary(kind()) || at(TokenKind::Comma) || at(TokenKind::Assign) || at(TokenKind::Colon) || at(TokenKind::KwIn) || strongStart(kind()))
    {
      return make<MissingBindingPattern>(point());
    }
    bump();
    return make<ErrorBindingPattern>(range(Start));
  }
  MatchPattern *Parser::parseMatchPattern()
  {
    DepthScope Nesting(*this);
    if (!Nesting)
    {
      return make<MissingMatchPattern>(point());
    }
    const std::size_t Start = start();
    std::vector<MatchPattern *> Alternatives;
    Alternatives.push_back(parseMatchAtom());
    while (take(TokenKind::Pipe) && active())
    {
      Alternatives.push_back(parseMatchAtom());
    }
    if (Alternatives.size() == 1)
    {
      return Alternatives.front();
    }
    return make<OrMatchPattern>(range(Start), array(Alternatives));
  }
  MatchPattern *Parser::parseMatchAtom()
  {
    const std::size_t Start = start();
    if (take(TokenKind::Underscore))
    {
      return make<WildcardMatchPattern>(range(Start));
    }
    if (at(TokenKind::Identifier))
    {
      std::vector<PathSegment> Path;
      do
      {
        const std::size_t SegmentStart = start();
        const NameToken Name = name();
        const bool Generic = take(TokenKind::DoubleColon);
        ASTArray<Argument> Arguments;
        if (Generic)
        {
          expect(TokenKind::LBracket);
          Arguments = parseArguments(TokenKind::RBracket);
        }
        Path.emplace_back(Name, Generic, Arguments, range(SegmentStart));
      } while (take(TokenKind::Dot) && active());
      const bool HasArguments = take(TokenKind::LParen);
      std::vector<MatchPattern *> Arguments;
      if (HasArguments)
      {
        RecoveryScope Scope(*this, TokenKind::RParen, true);
        while (!listEnd(TokenKind::RParen) && active())
        {
          const TokenId Before = Cursor.position();
          Arguments.push_back(parseMatchPattern());
          if (Before == Cursor.position() && !at(TokenKind::Comma) && !listEnd(TokenKind::RParen))
          {
            ensureProgress(Before);
          }
          if (!nextElement(TokenKind::RParen))
          {
            break;
          }
        }
        expect(TokenKind::RParen);
      }
      return make<NameMatchPattern>(range(Start), array(Path), HasArguments, array(Arguments));
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
      std::vector<MatchPattern *> Elements;
      std::optional<RestBinding> Rest;
      bool HadComma = false;
      while (!listEnd(End) && active())
      {
        const TokenId Before = Cursor.position();
        if (!Tuple && (at(TokenKind::Identifier) || at(TokenKind::Underscore)) && kind(1) == TokenKind::Ellipsis)
        {
          const bool Wildcard = at(TokenKind::Underscore);
          const TokenId Id = bump();
          const SourceRange Ellipsis = Input.token(bump()).Span;
          Rest = RestBinding{{Id, Input.spelling(Id), Input.token(Id).Span}, Wildcard, Ellipsis};
          break;
        }
        Elements.push_back(parseMatchPattern());
        HadComma = HadComma || at(TokenKind::Comma);
        if (Before == Cursor.position() && !at(TokenKind::Comma) && !listEnd(End))
        {
          ensureProgress(Before);
        }
        if (!nextElement(End, Tuple && Elements.size() == 1))
        {
          break;
        }
      }
      expect(End);
      if (Tuple && Elements.size() == 1 && !HadComma)
      {
        return make<GroupedMatchPattern>(range(Start), Elements.front());
      }
      if (Tuple)
      {
        return make<TupleMatchPattern>(range(Start), array(Elements));
      }
      return make<ArrayMatchPattern>(range(Start), array(Elements), Rest);
    }
    const bool Negative = take(TokenKind::Minus);
    if (at(TokenKind::IntegerLiteral) || at(TokenKind::FloatLiteral) || (!Negative && (at(TokenKind::StringLiteral) || at(TokenKind::CharLiteral))))
    {
      const TokenId Id = bump();
      LiteralExpr *Value = make<LiteralExpr>(Input.token(Id).Span, Id, Input.token(Id).Kind);
      return make<LiteralMatchPattern>(range(Start), Value, Negative);
    }
    invalid(point());
    if (boundary(kind()) || at(TokenKind::Comma) || at(TokenKind::FatArrow) || at(TokenKind::KwIf) || at(TokenKind::Pipe))
    {
      return make<MissingMatchPattern>(point());
    }
    bump();
    return make<ErrorMatchPattern>(range(Start));
  }
} // namespace ink::parser
