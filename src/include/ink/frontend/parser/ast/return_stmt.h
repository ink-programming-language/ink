#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

namespace ink::frontend::parser
{
  class ReturnStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ReturnStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ReturnStmt(SourceRange Range, Expr *Value)
          : Stmt(Kind, Range),
            Value(Value)
      {
      }
      auto *value(this auto &&Self) noexcept
      {
        return Self.Value ? &std::forward_like<decltype(Self) &>(*Self.Value) : nullptr;
      }

    private:
      Expr *Value;
  };
} // namespace ink::frontend::parser
