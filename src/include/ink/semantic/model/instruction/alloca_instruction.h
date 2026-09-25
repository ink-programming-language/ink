#ifndef INK_SEMANTIC_MODEL_ALLOCA_INSTRUCTION_H
#define INK_SEMANTIC_MODEL_ALLOCA_INSTRUCTION_H

#include "ink/semantic/model/type/pointer_type.h"

namespace ink::semantic
{
  // Allocates one uninitialized object per execution, with function activation lifetime.
  // Arrays use ArrayType; target layout determines size and natural alignment.
  class AllocaInstruction final : public Value
  {
    public:
      const PointerType &type() const noexcept override
      {
        return ResultType;
      }

      const Type &allocatedType() const noexcept
      {
        return ResultType.pointeeType();
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::AllocaInstruction;
      }

    private:
      explicit AllocaInstruction(const PointerType &ResultType) noexcept
          : Value(ResultType.context(), ValueKind::AllocaInstruction),
            ResultType(ResultType)
      {
      }

      const PointerType &ResultType;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
