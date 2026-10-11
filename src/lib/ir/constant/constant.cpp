#include "ink/ir/constant/constant.h"

namespace ink::ir
{
  Constant::Constant(ValueKind Kind, const Type *ConstantType) noexcept
      : Value(Kind),
        ConstantType(ConstantType)
  {
  }

  Constant::~Constant() noexcept = default;

  const Type *Constant::type() const noexcept
  {
    return ConstantType;
  }

  bool Constant::classof(const Value *Value) noexcept
  {
    if (!Value)
    {
      return false;
    }
    switch (Value->kind())
    {
      case ValueKind::BoolConstant:
      case ValueKind::IntegerConstant:
      case ValueKind::FloatConstant:
      case ValueKind::ArrayConstant:
      case ValueKind::NullPointerConstant:
      case ValueKind::StringConstant:
        return true;
      default:
        return false;
    }
  }
} // namespace ink::ir
