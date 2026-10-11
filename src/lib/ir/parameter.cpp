#include "ink/ir/parameter.h"

namespace ink::ir
{
  Parameter::Parameter(const Type *ParameterType, core::Name Name, bool Variadic) noexcept
      : ParameterType(ParameterType),
        Name(Name),
        Variadic(Variadic)
  {
  }

  const Type *Parameter::type() const noexcept
  {
    return ParameterType;
  }

  core::Name Parameter::name() const noexcept
  {
    return Name;
  }

  bool Parameter::hasName() const noexcept
  {
    return Name.valid();
  }

  bool Parameter::isVariadic() const noexcept
  {
    return Variadic;
  }
} // namespace ink::ir
