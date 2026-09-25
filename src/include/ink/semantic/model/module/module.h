#ifndef INK_SEMANTIC_MODEL_MODULE_H
#define INK_SEMANTIC_MODEL_MODULE_H

#include "ink/semantic/model/function/basic_block.h"
#include "ink/semantic/model/name/name.h"

namespace ink::semantic
{
  // A named module value with an entry block owned by the same context.
  class Module final : public Value
  {
    public:
      Name name() const noexcept
      {
        return ModuleName;
      }

      BasicBlock &entryBlock() noexcept
      {
        return EntryBlock;
      }

      const BasicBlock &entryBlock() const noexcept
      {
        return EntryBlock;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::Module;
      }

    private:
      Module(const SemanticContext &Context, Name ModuleName, BasicBlock &EntryBlock) noexcept;

      Name ModuleName;
      BasicBlock &EntryBlock;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
