#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"

namespace ink::frontend::parser
{
  class SimpleStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::SimpleStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      SimpleStmt(SourceRange Range, ASTArray<SimpleItem *> Items)
          : Stmt(Kind, Range),
            Items(Items)
      {
      }
      auto items(this auto &&Self) noexcept
      {
        return NodeArrayFor<decltype(Self), SimpleItem>(Self.Items);
      }

    private:
      ASTArray<SimpleItem *> Items;
  };
} // namespace ink::frontend::parser
