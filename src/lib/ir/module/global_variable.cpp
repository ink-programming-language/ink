#include "ink/ir/module/global_variable.h"

namespace ink::ir
{
  GlobalVariable::GlobalVariable(const PointerType &AddressType, core::Name Name) noexcept
      : Value(ValueKind::GlobalVariable),
        AddressType(&AddressType),
        Name(Name)
  {
  }

  const PointerType *GlobalVariable::type() const noexcept
  {
    return AddressType;
  }

  const Type *GlobalVariable::variableType() const noexcept
  {
    return AddressType->pointeeType();
  }

  bool GlobalVariable::isInitialized() const noexcept
  {
    return hasInitializer();
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
    if (NewInitializer && !NewInitializer->canInitialize(variableType()))
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
