#ifndef INK_IR_CONSTANT_CONSTANT_H
#define INK_IR_CONSTANT_CONSTANT_H

#include "ink/ir/type/type.h"

namespace ink::ir
{
  class ConstantPool;

  // Concrete immutable data constants. A Type is also a compile-time value,
  // so Constant membership alone does not describe evaluation availability.
  class Constant : public Value
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        if (!ValueObject)
        {
          return false;
        }
        switch (ValueObject->kind())
        {
#define INK_IR_CONSTANT_VALUE(Name) case ValueKind::Name:
#include "ink/ir/Values.def"
          return true;
        default:
          return false;
        }
      }

    protected:
      Constant(ValueKind Kind, const Type &ValueType) noexcept
          : Value(ValueType.context(), Kind, ValueType)
      {
      }
  };
} // namespace ink::ir

#endif
