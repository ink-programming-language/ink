#include "ink/ir/param/parameter.h"

#include "ink/ir/function/function.h"

namespace ink::ir
{
  Parameter::Parameter(Function &Parent, std::size_t Index) noexcept
      : Value(ValueKind::Parameter),
        Parent(Parent),
        Index(Index)
  {
  }

  Function &Parameter::function() noexcept
  {
    return Parent;
  }

  const Function &Parameter::function() const noexcept
  {
    return Parent;
  }

  std::size_t Parameter::index() const noexcept
  {
    return Index;
  }

  const Type *Parameter::type() const noexcept
  {
    return Parent.functionType()->parameters()[Index];
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
