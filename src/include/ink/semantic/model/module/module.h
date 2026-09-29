#ifndef INK_SEMANTIC_MODEL_MODULE_H
#define INK_SEMANTIC_MODEL_MODULE_H

#include "ink/semantic/model/function/basic_block.h"
#include "ink/semantic/model/name/name.h"

namespace ink::semantic
{
  class ModuleDecl;

  // A named module owns independent declaration and IR trees.
  class Module final : public Value
  {
    public:
      ~Module() override;

      Name name() const noexcept
      {
        return ModuleName;
      }

      BasicBlock &entryBlock() noexcept
      {
        return *EntryBlock;
      }

      const BasicBlock &entryBlock() const noexcept
      {
        return *EntryBlock;
      }

      // Null until IRBuilder attaches the module's borrowed-AST declaration root.
      ModuleDecl *declarationRoot() noexcept
      {
        return DeclarationRoot.get();
      }

      const ModuleDecl *declarationRoot() const noexcept
      {
        return DeclarationRoot.get();
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::Module;
      }

    private:
      Module(const SemanticContext &Context, Name ModuleName, std::unique_ptr<BasicBlock> EntryBlock) noexcept;

      Name ModuleName;
      // IR is destroyed before declarations it may reference. Both trees borrow shared context objects.
      std::unique_ptr<ModuleDecl> DeclarationRoot;
      std::unique_ptr<BasicBlock> EntryBlock;

      friend class IRBuilder;
  };
} // namespace ink::semantic

#endif
