#ifndef INK_IR_ARRAY_EXTRACT_INSTRUCTION_H
#define INK_IR_ARRAY_EXTRACT_INSTRUCTION_H

#include "ink/ir/type/array_type.h"

namespace ink::ir
{
  // Bounds-checked extraction from an array value; the result does not alias array storage.
  class ArrayExtractInstruction final : public Value
  {
    public:
      const Value &array() const noexcept
      {
        return Array;
      }

      const Value &index() const noexcept
      {
        return Index;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::ArrayExtractInstruction;
      }

    private:
      ArrayExtractInstruction(const Value &Array, const Value &Index) noexcept
          : Value(Array.context(), ValueKind::ArrayExtractInstruction, static_cast<const ArrayType &>(Array.type()).elementType()),
            Array(Array),
            Index(Index)
      {
      }

      const Value &Array;
      const Value &Index;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
