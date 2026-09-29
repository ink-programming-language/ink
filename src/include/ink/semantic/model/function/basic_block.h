#ifndef INK_SEMANTIC_MODEL_BASIC_BLOCK_H
#define INK_SEMANTIC_MODEL_BASIC_BLOCK_H

#include "ink/semantic/model/type/builtin_type.h"

#include <memory>
#include <vector>

namespace ink::semantic
{
  // A block owns its ordered children; detached blocks are owned by the caller.
  class BasicBlock final : public Value
  {
    public:
      // Owning children in insertion order. IRBuilder's edit APIs transfer ownership and maintain parent links.
      const std::vector<std::unique_ptr<Value>> &values() const noexcept
      {
        return Values;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::BasicBlock;
      }

    private:
      explicit BasicBlock(const SemanticContext &Context) noexcept;

      std::vector<std::unique_ptr<Value>> Values;

      friend class IRBuilder;
  };
} // namespace ink::semantic

#endif
