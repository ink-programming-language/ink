#ifndef INK_IR_MODULE_H
#define INK_IR_MODULE_H

#include "ink/ir/function/basic_block.h"
#include "ink/ir/name/name.h"

#include <span>

namespace ink::parser
{
  struct ParseResult;
} // namespace ink::parser

namespace ink::ir
{
  class ModuleDecl;
  class ClassType;

  // A named module owns independent declaration and IR trees.
  class Module final : public Value
  {
    public:
      ~Module() override;

      Name name() const noexcept
      {
        return ModuleName;
      }

      std::span<const ClassType *const> classTypes() const noexcept
      {
        return ClassTypes;
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

      // Complete syntax snapshots backing restored declarations; shared ownership also supports detached nested modules.
      const std::vector<std::shared_ptr<const parser::ParseResult>> &archivedASTs() const noexcept
      {
        return ArchivedASTs;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::Module;
      }

    private:
      Module(const IRContext &Context, Name ModuleName, std::unique_ptr<BasicBlock> EntryBlock) noexcept;

      Name ModuleName;
      std::vector<const ClassType *> ClassTypes;
      // Syntax outlives both declaration and IR trees.
      std::vector<std::shared_ptr<const parser::ParseResult>> ArchivedASTs;
      // IR is destroyed before declarations it may reference. Both trees borrow shared context objects.
      std::unique_ptr<ModuleDecl> DeclarationRoot;
      std::unique_ptr<BasicBlock> EntryBlock;

      friend class IRBuilder;
      friend class ModuleArchiveAccess;
  };
} // namespace ink::ir

#endif
