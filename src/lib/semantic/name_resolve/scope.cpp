#include "ink/semantic/name_resolve/scope.h"

namespace ink::semantic
{
  Scope::Scope(ScopeStore &Store, Scope *Parent, std::optional<ir::Value *> Owner) noexcept
      : Store(Store),
        Parent(Parent),
        Owner(Owner)
  {
  }

  const std::optional<ir::Value *> &Scope::owner() const noexcept
  {
    return Owner;
  }

  const Binding *Scope::lookupLocal(core::Name Name) const noexcept
  {
    const auto Found = Bindings.find(Name);
    return Found == Bindings.end() ? nullptr : &Found->second;
  }
} // namespace ink::semantic
