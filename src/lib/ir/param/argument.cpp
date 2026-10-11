#include "ink/ir/param/argument.h"

namespace ink::ir
{
  Argument::Argument(const Value &ArgumentValue, core::Name Name) noexcept
      : ArgumentValue(&ArgumentValue),
        Name(Name)
  {
  }

  const Value *Argument::value() const noexcept
  {
    return ArgumentValue;
  }

  core::Name Argument::name() const noexcept
  {
    return Name;
  }

  bool Argument::hasName() const noexcept
  {
    return Name.valid();
  }

  void Argument::setName(core::Name NewName) noexcept
  {
    Name = NewName;
  }
} // namespace ink::ir
