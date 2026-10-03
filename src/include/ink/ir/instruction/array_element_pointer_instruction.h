#ifndef INK_IR_ARRAY_ELEMENT_POINTER_INSTRUCTION_H
#define INK_IR_ARRAY_ELEMENT_POINTER_INSTRUCTION_H

#include "ink/ir/type/pointer_type.h"

namespace ink::ir
{
  // Bounds-checked array element address; preserves the source pointer's access permissions.
  class ArrayElementPointerInstruction final : public Value
  {
    public:
      const Value &address() const noexcept
      {
        return Address;
      }

      const Value &index() const noexcept
      {
        return Index;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::ArrayElementPointerInstruction;
      }

    private:
      ArrayElementPointerInstruction(const PointerType &ResultType, const Value &Address, const Value &Index) noexcept
          : Value(ResultType.context(), ValueKind::ArrayElementPointerInstruction, ResultType),
            Address(Address),
            Index(Index)
      {
      }

      const Value &Address;
      const Value &Index;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
