#include "ink/ir/type/generic_var_type.h"

#include <utility>

namespace ink::ir
{
  GenericVarType::GenericVarType(core::Name Name, std::vector<Parameter> GenericParameters, const Type *ValueType) noexcept
      : GenericType(ValueKind::GenericVar, Name, std::move(GenericParameters)),
        ValueType(ValueType)
  {
  }

  const Type *GenericVarType::valueType() const noexcept
  {
    return ValueType;
  }

  bool GenericVarType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::GenericVar;
  }
} // namespace ink::ir
