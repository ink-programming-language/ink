#include "ink/ir/constant/constant.h"

#include "ink/ir/constant/null_pointer_constant.h"
#include "ink/ir/type/builtin_type/null_pointer_type.h"
#include "ink/ir/type/structure_type/pointer_type.h"
#include "ink/ir/context.h"

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

  bool Constant::canInitialize(const Type *DestinationType) const noexcept
  {
    if (!DestinationType)
    {
      return false;
    }
    if (ConstantType && ConstantType->unqualifiedType() == DestinationType->unqualifiedType())
    {
      return true;
    }
    if (!NullPointerConstant::classof(this) || !NullPointerType::classof(ConstantType))
    {
      return false;
    }
    const IRContext &Owner = static_cast<const NullPointerType *>(ConstantType)->irContext();
    return Owner.constantPool().get<NullPointerConstant>() == this && Owner.containsType(DestinationType) && PointerType::classof(DestinationType);
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
