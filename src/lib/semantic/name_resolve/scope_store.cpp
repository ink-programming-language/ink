#include "ink/semantic/name_resolve/scope_store.h"
#include "ink/semantic/context.h"

#include <algorithm>
#include <utility>

namespace ink::semantic
{
  using namespace ink::ir;

  ScopeStore::ScopeStore(SemanticContext &Context)
      : ir::LifetimeObserver(Context.irContext()),
        Context(Context)
  {
    Scopes.push_back(std::unique_ptr<Scope>(new Scope(*this, nullptr)));
  }

  Scope &ScopeStore::createScope(Scope &Parent)
  {
    auto Result = std::unique_ptr<Scope>(new Scope(*this, &Parent));
    Scope *Pointer = Result.get();
    Scopes.push_back(std::move(Result));
    return *Pointer;
  }

  template <BindingTarget T>
  void ScopeStore::removeBindings(T Target, std::unordered_map<T, std::vector<BindingLocation>> &Locations) noexcept
  {
    const auto Found = Locations.find(Target);
    if (Found == Locations.end())
    {
      return;
    }
    for (const BindingLocation &Location : Found->second)
    {
      auto &Bindings = [&]() -> BindingTable<T> &
      {
        if constexpr (std::is_same_v<T, Value *>)
        {
          return Location.ScopeValue->ValueBindings;
        }
        else
        {
          return Location.ScopeValue->DeclBindings;
        }
      }();
      const auto Entry = Bindings.find(Location.BoundName);
      std::erase(Entry->second.Targets, Target);
      if (Entry->second.Targets.empty())
      {
        Bindings.erase(Entry);
      }
    }
    Locations.erase(Found);
  }

  void ScopeStore::valueDestroyed(Value &Target) noexcept
  {
    removeBindings(&Target, ValueBindingLocations);
    MemberScopes.erase(&Target);
  }

  void ScopeStore::declDestroyed(Decl &Target) noexcept
  {
    removeBindings(&Target, DeclBindingLocations);
    DefinitionScopes.erase(&Target);
  }
} // namespace ink::semantic
