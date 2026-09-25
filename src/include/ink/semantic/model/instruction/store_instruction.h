#ifndef INK_SEMANTIC_MODEL_STORE_INSTRUCTION_H
#define INK_SEMANTIC_MODEL_STORE_INSTRUCTION_H

#include "ink/semantic/model/type/builtin_type.h"

namespace ink::semantic
{
  // An ordinary, non-atomic write through a read-write pointer; it produces no result value.
  class StoreInstruction final : public Value
  {
    public:
      const Value &address() const noexcept
      {
        return Address;
      }

      const Value &storedValue() const noexcept
      {
        return StoredValue;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::StoreInstruction;
      }

    private:
      StoreInstruction(const Value &Address, const Value &StoredValue) noexcept;

      const Value &Address;
      const Value &StoredValue;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
