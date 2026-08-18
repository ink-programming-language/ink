#include "ink/ir/model/parameter.h"

#include <utility>

namespace ink::ir
{
  Parameter::Parameter(const Type *ValueType) noexcept
      : ValueType(ValueType),
        DefaultValue(nullptr)
  {
  }

  Parameter::Parameter(ink::ir::Name Name, const Type *ValueType, const Constant *DefaultValue) noexcept
      : Name(std::move(Name)),
        ValueType(ValueType),
        DefaultValue(DefaultValue)
  {
  }

  const ink::ir::Name &Parameter::name() const noexcept
  {
    return Name;
  }

  const Type *Parameter::type() const noexcept
  {
    return ValueType;
  }

  const Constant *Parameter::defaultValue() const noexcept
  {
    return DefaultValue;
  }
} // namespace ink::ir
