#ifndef INK_IR_CLASS_DECL_H
#define INK_IR_CLASS_DECL_H

#include "ink/parser/ast.h"
#include "ink/ir/decl/decl.h"

namespace ink::ir
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
      ClassDecl(Decl &Parent, Name DeclName, const parser::ClassDecl &AST) noexcept
          : Decl(Parent.module(), &Parent, DeclName, AST)
      {
      }

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
