#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

namespace ink::frontend::parser
{
  class DeferStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::DeferStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      DeferStmt(SourceRange Range, Stmt *Body)
          : Stmt(Kind, Range),
            Body(Body)
      {
      }
      auto *body(this auto &&Self) noexcept
      {
        return Self.Body ? &std::forward_like<decltype(Self) &>(*Self.Body) : nullptr;
      }

    private:
      Stmt *Body;
  };
} // namespace ink::frontend::parser
