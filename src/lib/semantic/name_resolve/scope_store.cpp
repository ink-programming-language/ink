#include "ink/semantic/name_resolve/scope_store.h"
#include "ink/semantic/context.h"

#include <algorithm>
#include <unordered_set>
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

  Scope *ScopeStore::snapshotScope(const Scope &Source, const std::function<Value *(Value *)> &Transform)
  {
    if (&Source.Store != this)
    {
      return nullptr;
    }
    auto Snapshot = std::unique_ptr<Scope>(new Scope(*this, nullptr));
    Scope *Result = Snapshot.get();
    std::unordered_set<Name> Seen;
    for (const Scope *Current = &Source; Current; Current = Current->Parent)
    {
      const auto CopyName = [&](Name BoundName)
      {
        if (!Seen.insert(BoundName).second)
        {
          return;
        }
        if (const auto Values = Current->ValueBindings.find(BoundName); Values != Current->ValueBindings.end())
        {
          auto &Binding = Result->ValueBindings.emplace(BoundName, Values->second).first->second;
          for (Value *&Target : Binding.Targets)
          {
            if (Transform)
            {
              Value *Replacement = Transform(Target);
              if (Replacement && &Replacement->context() == &Context.irContext())
              {
                Target = Replacement;
              }
            }
            ValueBindingLocations[Target].push_back({Result, BoundName});
          }
        }
        if (const auto Declarations = Current->DeclBindings.find(BoundName); Declarations != Current->DeclBindings.end())
        {
          Result->DeclBindings.emplace(BoundName, Declarations->second);
          for (Decl *Target : Declarations->second.Targets)
          {
            DeclBindingLocations[Target].push_back({Result, BoundName});
          }
        }
      };
      for (const auto &Entry : Current->ValueBindings)
      {
        CopyName(Entry.first);
      }
      for (const auto &Entry : Current->DeclBindings)
      {
        CopyName(Entry.first);
      }
    }
    Scopes.push_back(std::move(Snapshot));
    return Result;
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
