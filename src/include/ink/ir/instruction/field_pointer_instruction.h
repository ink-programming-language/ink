#ifndef INK_IR_FIELD_POINTER_INSTRUCTION_H
#define INK_IR_FIELD_POINTER_INSTRUCTION_H

#include "ink/ir/type/pointer_type.h"

namespace ink::ir
{
  // Projects a field within its parent's allocation, retaining permissions and lifetime.
  class FieldPointerInstruction final : public Value
  {
    public:
      const Value &address() const noexcept
      {
        return Address;
      }

      std::size_t fieldIndex() const noexcept
      {
        return FieldIndex;
      }

      static bool classof(const Value *Object) noexcept
      {
        return Object && Object->kind() == ValueKind::FieldPointerInstruction;
      }

    private:
      FieldPointerInstruction(const PointerType &ResultType, const Value &Address, std::size_t FieldIndex) noexcept
          : Value(ResultType.context(), ValueKind::FieldPointerInstruction, ResultType),
            Address(Address),
            FieldIndex(FieldIndex)
      {
      }

      const Value &Address;
      std::size_t FieldIndex;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
