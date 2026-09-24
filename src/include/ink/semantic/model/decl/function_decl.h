#ifndef INK_SEMANTIC_MODEL_FUNCTION_DECL_H
#define INK_SEMANTIC_MODEL_FUNCTION_DECL_H

#include "ink/parser/ast.h"
#include "ink/semantic/model/decl/decl.h"

namespace ink::semantic
{
  // Only generic function definitions have a FunctionDecl; closed functions are Function values.
  class FunctionDecl final : public Decl
  {
    public:
      const parser::FunctionDecl &ast() const noexcept
      {
        return static_cast<const parser::FunctionDecl &>(Decl::ast());
      }

      static bool classof(const Decl *Declaration) noexcept
      {
        return Declaration && parser::FunctionDecl::classof(&Declaration->ast());
      }

    private:
      FunctionDecl(Name DeclName, const parser::FunctionDecl &AST) noexcept
          : Decl(DeclName, AST)
      {
      }

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
