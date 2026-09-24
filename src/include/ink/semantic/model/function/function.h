#ifndef INK_SEMANTIC_MODEL_FUNCTION_H
#define INK_SEMANTIC_MODEL_FUNCTION_H

#include "ink/semantic/model/function/basic_block.h"
#include "ink/semantic/model/function/function_type.h"
#include "ink/semantic/model/name.h"

#include <vector>

namespace ink::semantic
{
  // A closed callable identity. Generic definitions must be instantiated before creating this value.
  class Function final : public Value
  {
    public:
      Name name() const noexcept
      {
        return FunctionName;
      }

      const FunctionType &type() const noexcept override
      {
        return Signature;
      }

      bool hasBody() const noexcept
      {
        return !Blocks.empty();
      }

      // The first block is the entry; declarations have no blocks and return null.
      BasicBlock *entryBlock() noexcept
      {
        return Blocks.empty() ? nullptr : Blocks.front();
      }

      const BasicBlock *entryBlock() const noexcept
      {
        return Blocks.empty() ? nullptr : Blocks.front();
      }

      // Structural children in insertion order, not execution order. Context manages membership.
      const std::vector<BasicBlock *> &blocks() const noexcept
      {
        return Blocks;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::Function;
      }

    private:
      Function(Name FunctionName, const FunctionType &Signature) noexcept
          : Value(Signature.context(), ValueKind::Function),
            FunctionName(FunctionName),
            Signature(Signature)
      {
      }

      Name FunctionName;
      const FunctionType &Signature;
      std::vector<BasicBlock *> Blocks;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
