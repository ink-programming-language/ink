#ifndef INK_SEMANTIC_MODEL_DECL_H
#define INK_SEMANTIC_MODEL_DECL_H

#include "ink/semantic/model/name/name.h"

#include <memory>
#include <vector>

namespace ink::parser
{
  class ASTNodeBase;
} // namespace ink::parser

namespace ink::semantic
{
  class Module;

  // A module or uninstantiated generic definition in a module-owned declaration tree.
  // Its AST is borrowed and must outlive the owning module.
  // Resolved types, values, bindings and instance state are stored separately.
  class Decl
  {
    public:
      virtual ~Decl();
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

      Module &module() noexcept
      {
        return Owner;
      }

      const Module &module() const noexcept
      {
        return Owner;
      }

      // The module declaration root has no declaration parent.
      Decl *parent() noexcept
      {
        return Parent;
      }

      const Decl *parent() const noexcept
      {
        return Parent;
      }

      // Owned children in insertion order. IRBuilder creates and attaches declarations.
      const std::vector<std::unique_ptr<Decl>> &children() const noexcept
      {
        return Children;
      }

    protected:
      Decl(Module &Owner, Decl *Parent, Name DeclName, const parser::ASTNodeBase &AST) noexcept
          : Owner(Owner),
            Parent(Parent),
            DeclName(DeclName),
            AST(AST)
      {
      }

    private:
      Module &Owner;
      Decl *Parent;
      Name DeclName;
      const parser::ASTNodeBase &AST;
      std::vector<std::unique_ptr<Decl>> Children;

      friend class IRBuilder;
  };
} // namespace ink::semantic

#endif
