#include "ink/ir/module/global_variable.h"

namespace ink::ir
{
  GlobalVariable::GlobalVariable(const Type &VariableType, core::Name Name) noexcept
      : Value(ValueKind::GlobalVariable),
        VariableType(&VariableType),
        Name(Name)
  {
  }

  const Type *GlobalVariable::type() const noexcept
  {
    return VariableType;
  }

  core::Name GlobalVariable::name() const noexcept
  {
    return Name;
  }

  bool GlobalVariable::hasName() const noexcept
  {
    return Name.valid();
  }

  void GlobalVariable::setName(core::Name NewName) noexcept
  {
    Name = NewName;
  }

  const Constant *GlobalVariable::initializer() const noexcept
  {
    return Initializer;
  }

  bool GlobalVariable::hasInitializer() const noexcept
  {
    return Initializer != nullptr;
  }

  bool GlobalVariable::setInitializer(const Constant *NewInitializer) noexcept
  {
    if (NewInitializer && NewInitializer->type() != VariableType)
    {
      return false;
    }
    Initializer = NewInitializer;
    return true;
  }

  bool GlobalVariable::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::GlobalVariable;
  }
} // namespace ink::ir
