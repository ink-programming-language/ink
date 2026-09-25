#ifndef INK_SEMANTIC_MODEL_RETURN_INSTRUCTION_H
#define INK_SEMANTIC_MODEL_RETURN_INSTRUCTION_H

#include "ink/semantic/model/value.h"

namespace ink::semantic
{
  class Function;

  // Terminates the current invocation. A void return has no returned operand.
  class ReturnInstruction final : public Value
  {
    public:
      const Type &type() const noexcept override;

      // Derived from outer() -> BasicBlock -> Function; null while detached.
      const Function *function() const noexcept;

      const Value *returnedValue() const noexcept
      {
        return ReturnedValue;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::ReturnInstruction;
      }

    private:
      ReturnInstruction(const SemanticContext &Context, const Value *ReturnedValue) noexcept
          : Value(Context, ValueKind::ReturnInstruction),
            ReturnedValue(ReturnedValue)
      {
      }

      const Value *ReturnedValue;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
