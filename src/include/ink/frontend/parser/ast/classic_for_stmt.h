#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"

namespace ink::frontend::parser
{
  class ClassicForStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ClassicForStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ClassicForStmt(SourceRange Range, Stmt *Initializer, Expr *Condition, ASTArray<SimpleItem *> Step, Stmt *Body)
          : Stmt(Kind, Range),
            Initializer(Initializer),
            Condition(Condition),
            Step(Step),
            Body(Body)
      {
      }
      Stmt *initializer() noexcept
      {
        return Initializer;
      }
      const Stmt *initializer() const noexcept
      {
        return Initializer;
      }
      Expr *condition() noexcept
      {
        return Condition;
      }
      const Expr *condition() const noexcept
      {
        return Condition;
      }
      ASTArray<SimpleItem *> step() noexcept
      {
        return Step;
      }
      ConstNodeArray<SimpleItem> step() const noexcept
      {
        return ConstNodeArray<SimpleItem>(Step);
      }
      Stmt *body() noexcept
      {
        return Body;
      }
      const Stmt *body() const noexcept
      {
        return Body;
      }

    private:
      Stmt *Initializer;
      Expr *Condition;
      ASTArray<SimpleItem *> Step;
      Stmt *Body;
  };
} // namespace ink::frontend::parser
