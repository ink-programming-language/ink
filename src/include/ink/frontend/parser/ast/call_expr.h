#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/argument.h"
#include <utility>

namespace ink::frontend::parser
{
  class CallExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::CallExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      CallExpr(SourceRange Range, Expr *Callee, ASTArray<Argument> Arguments, bool Optional)
          : Expr(Kind, Range),
            Callee(Callee),
            Arguments(Arguments),
            Optional(Optional)
      {
      }
      auto *callee(this auto &&Self) noexcept
      {
        return Self.Callee ? &std::forward_like<decltype(Self) &>(*Self.Callee) : nullptr;
      }
      ASTArray<Argument> arguments() const noexcept
      {
        return Arguments;
      }
      bool optional() const noexcept
      {
        return Optional;
      }

    private:
      Expr *Callee;
      ASTArray<Argument> Arguments;
      bool Optional;
  };
} // namespace ink::frontend::parser
