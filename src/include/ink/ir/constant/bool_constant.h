#ifndef INK_IR_CONSTANT_BOOL_CONSTANT_H
#define INK_IR_CONSTANT_BOOL_CONSTANT_H

#include "ink/ir/constant/constant.h"

namespace ink::ir
{
  class BoolConstant final : public Constant
  {
    public:
      bool value() const noexcept
      {
        return Payload;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::BoolConstant;
      }

    private:
      BoolConstant(const Type &ValueType, bool Payload) noexcept
          : Constant(ValueKind::BoolConstant, ValueType),
            Payload(Payload)
      {
      }

      bool Payload;

      friend class ConstantPool;
  };
} // namespace ink::ir

#endif
