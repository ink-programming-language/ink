#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include <utility>

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
      auto *initializer(this auto &&Self) noexcept
      {
        return Self.Initializer ? &std::forward_like<decltype(Self) &>(*Self.Initializer) : nullptr;
      }
      auto *condition(this auto &&Self) noexcept
      {
        return Self.Condition ? &std::forward_like<decltype(Self) &>(*Self.Condition) : nullptr;
      }
      auto step(this auto &&Self) noexcept
      {
        return NodeArrayFor<decltype(Self), SimpleItem>(Self.Step);
      }
      auto *body(this auto &&Self) noexcept
      {
        return Self.Body ? &std::forward_like<decltype(Self) &>(*Self.Body) : nullptr;
      }

    private:
      Stmt *Initializer;
      Expr *Condition;
      ASTArray<SimpleItem *> Step;
      Stmt *Body;
  };
} // namespace ink::frontend::parser
