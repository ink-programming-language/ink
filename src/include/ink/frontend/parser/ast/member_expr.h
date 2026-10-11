#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class MemberExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MemberExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MemberExpr(SourceRange Range, Expr *Object, TokenKind Access, NameToken Member)
          : Expr(Kind, Range),
            Object(Object),
            Access(Access),
            Member(Member)
      {
      }
      Expr *object() noexcept
      {
        return Object;
      }
      const Expr *object() const noexcept
      {
        return Object;
      }
      TokenKind access() const noexcept
      {
        return Access;
      }
      NameToken member() const noexcept
      {
        return Member;
      }

    private:
      Expr *Object;
      TokenKind Access;
      NameToken Member;
  };
} // namespace ink::frontend::parser
