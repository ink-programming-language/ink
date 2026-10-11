#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/argument.h"

namespace ink::frontend::parser
{
  class GenericApplyExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::GenericApplyExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      GenericApplyExpr(SourceRange Range, Expr *Object, ASTArray<Argument> Arguments)
          : Expr(Kind, Range),
            Object(Object),
            Arguments(Arguments)
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
      ASTArray<Argument> arguments() const noexcept
      {
        return Arguments;
      }

    private:
      Expr *Object;
      ASTArray<Argument> Arguments;
  };
} // namespace ink::frontend::parser
