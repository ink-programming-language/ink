#ifndef INK_IR_MODULE_DECL_H
#define INK_IR_MODULE_DECL_H

#include "ink/parser/ast.h"
#include "ink/ir/decl/decl.h"

namespace ink::ir
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
      ModuleDecl(Module &Owner, Name DeclName, const parser::ModuleAST &AST) noexcept
          : Decl(Owner, nullptr, DeclName, AST)
      {
      }

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
