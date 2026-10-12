#include "ink/ir/constant/null_pointer_constant.h"

namespace ink::ir
{
  NullPointerConstant::NullPointerConstant(const NullPointerType *ConstantType) noexcept
      : Constant(ValueKind::NullPointerConstant, ConstantType)
  {
  }

  bool NullPointerConstant::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::NullPointerConstant;
  }
} // namespace ink::ir
