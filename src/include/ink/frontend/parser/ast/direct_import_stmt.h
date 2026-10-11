#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/import_entry.h"

namespace ink::frontend::parser
{
  class DirectImportStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::DirectImportStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      DirectImportStmt(SourceRange Range, ASTArray<ImportEntry> Imports)
          : Stmt(Kind, Range),
            Imports(Imports)
      {
      }
      ASTArray<ImportEntry> imports() const noexcept
      {
        return Imports;
      }

    private:
      ASTArray<ImportEntry> Imports;
  };
} // namespace ink::frontend::parser
