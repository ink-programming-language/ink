#pragma once

#include "ink/frontend/parser/ast/simple_item.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class AssignmentItem final : public SimpleItem
  {
    public:
      static constexpr ASTKind Kind = ASTKind::AssignmentItem;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      AssignmentItem(SourceRange Range, Expr *Left, TokenKind Op, SourceRange OperatorRange, SimpleItem *Right)
          : SimpleItem(Kind, Range),
            Left(Left),
            Op(Op),
            OperatorRange(OperatorRange),
            Right(Right)
      {
      }
      Expr *left() noexcept
      {
        return Left;
      }
      const Expr *left() const noexcept
      {
        return Left;
      }
      TokenKind op() const noexcept
      {
        return Op;
      }
      SourceRange operatorRange() const noexcept
      {
        return OperatorRange;
      }
      SimpleItem *right() noexcept
      {
        return Right;
      }
      const SimpleItem *right() const noexcept
      {
        return Right;
      }

    private:
      Expr *Left;
      TokenKind Op;
      SourceRange OperatorRange;
      SimpleItem *Right;
  };
} // namespace ink::frontend::parser
