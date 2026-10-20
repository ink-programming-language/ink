#include "ink/ir/context.h"
#include "ink/ir/type/nominal_type/nominal_type.h"

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
    if (Types.owns(TypeValue))
    {
      return !NominalType::classof(TypeValue) || NominalTypes.contains(static_cast<const NominalType *>(TypeValue)->declarationType());
    }
    return NominalTypes.contains(TypeValue);
  }

  void IRContext::registerNominalType(const Type *TypeValue)
  {
    NominalTypes.insert(TypeValue);
  }

  void IRContext::unregisterNominalType(const Type *TypeValue) noexcept
  {
    Types.unregisterNominalType(TypeValue);
    NominalTypes.erase(TypeValue);
  }
} // namespace ink::ir
