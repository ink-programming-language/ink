#pragma once

#include "ink/frontend/parser/ast/ast_node_base.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"

namespace ink::frontend::parser
{
  class ModuleAST final : public ASTNodeBase
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ModuleAST;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ModuleAST(SourceRange Range, ASTArray<Stmt *> Statements)
          : ASTNodeBase(Kind, Range),
            Statements(Statements)
      {
      }
      auto statements(this auto &&Self) noexcept
      {
        return NodeArrayFor<decltype(Self), Stmt>(Self.Statements);
      }

    private:
      ASTArray<Stmt *> Statements;
  };
} // namespace ink::frontend::parser
