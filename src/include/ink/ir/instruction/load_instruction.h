#ifndef INK_IR_LOAD_INSTRUCTION_H
#define INK_IR_LOAD_INSTRUCTION_H

#include "ink/ir/type/pointer_type.h"

namespace ink::ir
{
  // An ordinary, non-atomic read. The instruction is the loaded value, not a live alias.
  class LoadInstruction final : public Value
  {
    public:
      const Value &address() const noexcept
      {
        return Address;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::LoadInstruction;
      }

    private:
      explicit LoadInstruction(const Value &Address) noexcept
          : Value(Address.context(), ValueKind::LoadInstruction, static_cast<const PointerType &>(Address.type()).pointeeType()),
            Address(Address)
      {
      }

      const Value &Address;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
