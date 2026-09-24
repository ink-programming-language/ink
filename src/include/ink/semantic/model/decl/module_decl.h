#ifndef INK_SEMANTIC_MODEL_MODULE_DECL_H
#define INK_SEMANTIC_MODEL_MODULE_DECL_H

#include "ink/parser/ast.h"
#include "ink/semantic/model/decl/decl.h"

namespace ink::semantic
{
  // A module definition with its borrowed file AST and ordered child declarations.
  class ModuleDecl final : public Decl
  {
    public:
      const parser::ModuleAST &ast() const noexcept
      {
        return static_cast<const parser::ModuleAST &>(Decl::ast());
      }

      static bool classof(const Decl *Declaration) noexcept
      {
        return Declaration && parser::ModuleAST::classof(&Declaration->ast());
      }

    private:
      ModuleDecl(Name DeclName, const parser::ModuleAST &AST) noexcept
          : Decl(DeclName, AST)
      {
      }

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
