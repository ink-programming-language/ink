#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

namespace ink::frontend::parser
{
  class PostfixUpdateExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::PostfixUpdateExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      PostfixUpdateExpr(SourceRange Range, Expr *Operand, TokenKind Op, SourceRange OperatorRange)
          : Expr(Kind, Range),
            Operand(Operand),
            Op(Op),
            OperatorRange(OperatorRange)
      {
      }
      auto *operand(this auto &&Self) noexcept
      {
        return Self.Operand ? &std::forward_like<decltype(Self) &>(*Self.Operand) : nullptr;
      }
      TokenKind op() const noexcept
      {
        return Op;
      }
      SourceRange operatorRange() const noexcept
      {
        return OperatorRange;
      }

    private:
      Expr *Operand;
      TokenKind Op;
      SourceRange OperatorRange;
  };
} // namespace ink::frontend::parser
