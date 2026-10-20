#include "ink/semantic/name_resolve/name_resolver.h"

#include "ink/ir/function/function.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  NameResolver::NameResolver(ScopeStore &Store) noexcept
      : Store(Store),
        CurrentScope(&Store.rootScope())
  {
  }

  NameResolver::NameResolver(SemanticContext &Context) noexcept
      : NameResolver(Context.scopeStore())
  {
  }

  NameResolver::NameResolver(Scope &StartingScope) noexcept
      : Store(StartingScope.scopeStore()),
        CurrentScope(&StartingScope)
  {
  }

  Scope &NameResolver::enterScope()
  {
    CurrentScope = Store.createScope(*CurrentScope);
    return *CurrentScope;
  }

  Scope *NameResolver::enterScope(ir::Value &Owner)
  {
    Scope *EnteredScope = Store.createScope(*CurrentScope, Owner);
    if (EnteredScope)
    {
      CurrentScope = EnteredScope;
    }
    return EnteredScope;
  }

  bool NameResolver::enterScope(Scope &ExistingScope) noexcept
  {
    RETURN_IF(&ExistingScope.scopeStore() != &Store, false);
    CurrentScope = &ExistingScope;
    return true;
  }

  bool NameResolver::exitScope() noexcept
  {
    Scope *Parent = CurrentScope->parent();
    RETURN_IF(!Parent, false);
    CurrentScope = Parent;
    return true;
  }

  NameResolver::BindResult NameResolver::bind(core::Name Name, ir::Value &Target)
  {
    return Store.bind(*CurrentScope, Name, &Target, false);
  }

  NameResolver::BindResult NameResolver::bindOverload(core::Name Name, ir::Function &Target)
  {
    return Store.bind(*CurrentScope, Name, &Target, true);
  }

  const Binding *NameResolver::lookup(core::Name Name) const noexcept
  {
    for (const Scope *SearchScope = CurrentScope; SearchScope; SearchScope = SearchScope->parent())
    {
      if (const Binding *Found = SearchScope->lookupLocal(Name))
      {
        return Found;
      }
    }
    return nullptr;
  }

  const Binding *NameResolver::lookupLocal(core::Name Name) const noexcept
  {
    return CurrentScope->lookupLocal(Name);
  }

  const Binding *NameResolver::lookupMember(const ir::Value &Owner, core::Name Name) const noexcept
  {
    const ir::Value &Declaration = ir::Type::classof(&Owner) ? *static_cast<const ir::Type &>(Owner).unqualifiedType() : Owner;
    const Scope *Members = scopeStore().memberScope(Declaration);
    return Members ? Members->lookupLocal(Name) : nullptr;
  }
} // namespace ink::semantic
