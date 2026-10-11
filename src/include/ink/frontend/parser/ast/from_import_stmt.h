#pragma once

#include "ink/frontend/parser/ast/stmt.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/import_entry.h"

namespace ink::frontend::parser
{
  class FromImportStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::FromImportStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      FromImportStmt(SourceRange Range, std::size_t RelativeLevel, ASTArray<TokenId> RelativeTokens, ASTArray<NameToken> Path, ASTArray<ImportEntry> Imports)
          : Stmt(Kind, Range),
            RelativeLevel(RelativeLevel),
            RelativeTokens(RelativeTokens),
            Path(Path),
            Imports(Imports)
      {
      }
      std::size_t relativeLevel() const noexcept
      {
        return RelativeLevel;
      }
      ASTArray<TokenId> relativeTokens() const noexcept
      {
        return RelativeTokens;
      }
      ASTArray<NameToken> path() const noexcept
      {
        return Path;
      }
      ASTArray<ImportEntry> imports() const noexcept
      {
        return Imports;
      }

    private:
      std::size_t RelativeLevel;
      ASTArray<TokenId> RelativeTokens;
      ASTArray<NameToken> Path;
      ASTArray<ImportEntry> Imports;
  };
} // namespace ink::frontend::parser
