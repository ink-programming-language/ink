#include "ink/semantic/name_resolve/scope_guard.h"

#include "ink/semantic/name_resolve/name_resolver.h"

namespace ink::semantic
{
  ScopeGuard::ScopeGuard(NameResolver &Resolver)
      : Resolver(Resolver),
        SavedScope(&Resolver.currentScope()),
        EnteredScope(&Resolver.enterScope())
  {
  }

  ScopeGuard::ScopeGuard(NameResolver &Resolver, Scope &ExistingScope) noexcept
      : Resolver(Resolver),
        SavedScope(&Resolver.currentScope()),
        EnteredScope(Resolver.enterScope(ExistingScope) ? &ExistingScope : nullptr)
  {
  }

  ScopeGuard::ScopeGuard(NameResolver &Resolver, ir::Value &Owner)
      : Resolver(Resolver),
        SavedScope(&Resolver.currentScope()),
        EnteredScope(Resolver.enterScope(Owner))
  {
  }

  ScopeGuard::~ScopeGuard() noexcept
  {
    if (EnteredScope)
    {
      Resolver.CurrentScope = SavedScope;
    }
  }
} // namespace ink::semantic
