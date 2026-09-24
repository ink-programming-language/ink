#ifndef INK_SEMANTIC_MODEL_CLASS_DECL_H
#define INK_SEMANTIC_MODEL_CLASS_DECL_H

#include "ink/parser/ast.h"
#include "ink/semantic/model/decl/decl.h"

namespace ink::semantic
{
  // A generic class definition, separate from every instantiated ClassType.
  class ClassDecl final : public Decl
  {
    public:
      const parser::ClassDecl &ast() const noexcept
      {
        return static_cast<const parser::ClassDecl &>(Decl::ast());
      }

      static bool classof(const Decl *Declaration) noexcept
      {
        return Declaration && parser::ClassDecl::classof(&Declaration->ast());
      }

    private:
      ClassDecl(Name DeclName, const parser::ClassDecl &AST) noexcept
          : Decl(DeclName, AST)
      {
      }

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
