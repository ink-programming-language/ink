#include "ink/ir/param/parameter.h"

#include "ink/ir/function/function.h"

namespace ink::ir
{
  Parameter::Parameter(const Type *ParameterType, bool Variadic) noexcept
      : Value(ValueKind::Parameter),
        ParameterType(ParameterType),
        Variadic(Variadic)
  {
  }

  Parameter::Parameter(Function &Parent, std::size_t Index) noexcept
      : Parameter(Parent.functionType()->parameters()[Index]->type(), Parent.functionType()->parameters()[Index]->isVariadic())
  {
    this->Parent = &Parent;
    this->Index = Index;
  }

  std::size_t Parameter::index() const noexcept
  {
    return Index;
  }

  const Type *Parameter::type() const noexcept
  {
    return ParameterType;
  }

  bool Parameter::isVariadic() const noexcept
  {
    return Variadic;
  }

  core::Name Parameter::name() const noexcept
  {
    return Name;
  }

  bool Parameter::hasName() const noexcept
  {
    return Name.valid();
  }

  void Parameter::setName(core::Name NewName) noexcept
  {
    Name = NewName;
  }

  bool Parameter::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Parameter;
  }
} // namespace ink::ir
