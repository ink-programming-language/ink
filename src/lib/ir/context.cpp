#include "ink/ir/context.h"

namespace ink::ir
{
  IRContext::IRContext(target::TargetContext Target)
      : Target(Target),
        Types(*this),
        Constants(*this)
  {
  }

  IRContext::~IRContext() noexcept = default;

  const target::TargetContext &IRContext::targetContext() const noexcept
  {
    return Target;
  }

  bool IRContext::containsType(const Type *TypeValue) const noexcept
  {
    return Types.owns(TypeValue) || NominalTypes.contains(TypeValue);
  }

  void IRContext::registerNominalType(const Type *TypeValue)
  {
    NominalTypes.insert(TypeValue);
  }

  void IRContext::unregisterNominalType(const Type *TypeValue) noexcept
  {
    NominalTypes.erase(TypeValue);
  }
} // namespace ink::ir
