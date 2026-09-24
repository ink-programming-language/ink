#ifndef INK_SEMANTIC_MODEL_DECL_H
#define INK_SEMANTIC_MODEL_DECL_H

#include "ink/semantic/model/name.h"

#include <vector>

namespace ink::parser
{
  class ASTNodeBase;
} // namespace ink::parser

namespace ink::semantic
{
  // A module or uninstantiated generic definition. Its AST is borrowed and must outlive the context.
  // Resolved types, values, bindings and instance state are stored separately.
  class Decl
  {
    public:
      virtual ~Decl() = default;
      Decl(const Decl &) = delete;
      Decl &operator=(const Decl &) = delete;
      Decl(Decl &&) = delete;
      Decl &operator=(Decl &&) = delete;

      Name name() const noexcept
      {
        return DeclName;
      }

      const parser::ASTNodeBase &ast() const noexcept
      {
        return AST;
      }

      // Non-owning child declarations in insertion order.
      std::vector<const Decl *> &child() noexcept
      {
        return Child;
      }

      const std::vector<const Decl *> &child() const noexcept
      {
        return Child;
      }

    protected:
      Decl(Name DeclName, const parser::ASTNodeBase &AST) noexcept
          : DeclName(DeclName),
            AST(AST)
      {
      }

    private:
      Name DeclName;
      const parser::ASTNodeBase &AST;
      std::vector<const Decl *> Child;
  };
} // namespace ink::semantic

#endif
