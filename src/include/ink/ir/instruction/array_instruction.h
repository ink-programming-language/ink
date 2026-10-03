#ifndef INK_IR_ARRAY_INSTRUCTION_H
#define INK_IR_ARRAY_INSTRUCTION_H

#include "ink/ir/type/array_type.h"

#include <span>
#include <vector>

namespace ink::ir
{
  // Constructs an array value. Repetition uses one already evaluated element, even for an empty array.
  class ArrayInstruction final : public Value
  {
    public:
      const ArrayType &arrayType() const noexcept
      {
        return static_cast<const ArrayType &>(type());
      }

      std::span<const Value *const> elements() const noexcept
      {
        return Elements;
      }

      bool repeated() const noexcept
      {
        return Repeated;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::ArrayInstruction;
      }

    private:
      ArrayInstruction(const ArrayType &ValueType, std::span<const Value *const> Elements, bool Repeated)
          : Value(ValueType.context(), ValueKind::ArrayInstruction, ValueType),
            Elements(Elements.begin(), Elements.end()),
            Repeated(Repeated)
      {
      }

      std::vector<const Value *> Elements;
      bool Repeated;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
