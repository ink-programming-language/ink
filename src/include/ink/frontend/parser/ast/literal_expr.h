#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class LiteralExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::LiteralExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      LiteralExpr(SourceRange Range, TokenId Token, TokenKind LiteralKind)
          : Expr(Kind, Range),
            Token(Token),
            LiteralKind(LiteralKind)
      {
      }
      TokenId token() const noexcept
      {
        return Token;
      }
      TokenKind literalKind() const noexcept
      {
        return LiteralKind;
      }

    private:
      TokenId Token;
      TokenKind LiteralKind;
  };
} // namespace ink::frontend::parser
