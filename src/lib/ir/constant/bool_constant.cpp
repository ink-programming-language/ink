#include "ink/ir/constant/bool_constant.h"

namespace ink::ir
{
  BoolConstant::BoolConstant(const BoolType *ConstantType, bool Value) noexcept
      : Constant(ValueKind::BoolConstant, ConstantType),
        StoredValue(Value)
  {
  }

  bool BoolConstant::value() const noexcept
  {
    return StoredValue;
  }

  bool BoolConstant::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::BoolConstant;
  }
} // namespace ink::ir
