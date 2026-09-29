#ifndef INK_IR_FUNCTION_DECL_H
#define INK_IR_FUNCTION_DECL_H

#include "ink/parser/ast.h"
#include "ink/ir/decl/decl.h"

namespace ink::ir
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
      FunctionDecl(Decl &Parent, Name DeclName, const parser::FunctionDecl &AST) noexcept
          : Decl(Parent.module(), &Parent, DeclName, AST)
      {
      }

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
