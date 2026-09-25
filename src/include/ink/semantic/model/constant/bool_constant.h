#ifndef INK_SEMANTIC_MODEL_CONSTANT_BOOL_CONSTANT_H
#define INK_SEMANTIC_MODEL_CONSTANT_BOOL_CONSTANT_H

#include "ink/semantic/model/constant/constant.h"

namespace ink::semantic
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
} // namespace ink::semantic

#endif
