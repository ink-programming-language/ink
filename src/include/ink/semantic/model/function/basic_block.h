#ifndef INK_SEMANTIC_MODEL_BASIC_BLOCK_H
#define INK_SEMANTIC_MODEL_BASIC_BLOCK_H

#include "ink/semantic/model/type/builtin_type.h"

#include <vector>

namespace ink::semantic
{
  // A context-owned block with the dedicated label type and an ordered value list.
  class BasicBlock final : public Value
  {
    public:
      // Structural children in insertion order. SemanticContext manages membership and parent links.
      const std::vector<Value *> &values() const noexcept
      {
        return Values;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::BasicBlock;
      }

    private:
      explicit BasicBlock(const SemanticContext &Context) noexcept;

      std::vector<Value *> Values;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
