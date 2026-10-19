#include "ink/semantic/name_resolve/scope_store.h"

#include "ink/semantic/analyzer/analyzer_utils.h"

#include <algorithm>
#include <utility>

namespace ink::semantic
{
  ScopeStore::ScopeStore() noexcept
      : Root(*this, nullptr)
  {
  }

  ScopeStore::~ScopeStore() noexcept = default;

  Scope &ScopeStore::appendScope(Scope &Parent, std::optional<ir::Value *> Owner)
  {
    // Construct here because Scope deliberately exposes no public constructor.
    auto NewScope = std::unique_ptr<Scope>(new Scope(*this, &Parent, Owner));
    Scope &Result = *NewScope;
    Scopes.push_back(std::move(NewScope));
    return Result;
  }

  Scope *ScopeStore::createScope(Scope &Parent)
  {
    return &Parent.Store == this ? &appendScope(Parent, std::nullopt) : nullptr;
  }

  Scope *ScopeStore::createScope(Scope &Parent, ir::Value &Owner)
  {
    RETURN_IF(&Parent.Store != this || MemberScopes.contains(&Owner), nullptr);
    Scope &Result = appendScope(Parent, &Owner);
    MemberScopes.emplace(&Owner, &Result);
    return &Result;
  }

  BindResult ScopeStore::bind(Scope &Destination, core::Name Name, ir::Value *Target, bool Overload)
  {
    RETURN_IF(!Name.valid(), BindResult::InvalidName);
    const auto Found = Destination.Bindings.find(Name);
    if (Found == Destination.Bindings.end())
    {
      Destination.Bindings.emplace(Name, Binding(Name, Target, Overload));
      return BindResult::Inserted;
    }

    Binding &Existing = Found->second;
    RETURN_IF(Existing.OverloadSet != Overload, BindResult::Conflict);
    RETURN_IF(std::find(Existing.Targets.begin(), Existing.Targets.end(), Target) != Existing.Targets.end(), BindResult::AlreadyBound);
    RETURN_IF(!Overload, BindResult::Conflict);
    Existing.Targets.push_back(Target);
    return BindResult::Inserted;
  }

  void ScopeStore::removeTarget(const ir::Value &Target) noexcept
  {
    const auto RemoveFromScope = [&Target](Scope &Current)
    {
      if (Current.Owner == &Target)
      {
        Current.Owner.reset();
      }
      for (auto Entry = Current.Bindings.begin(); Entry != Current.Bindings.end();)
      {
        std::erase(Entry->second.Targets, &Target);
        if (Entry->second.Targets.empty())
        {
          Entry = Current.Bindings.erase(Entry);
        }
        else
        {
          ++Entry;
        }
      }
    };
    RemoveFromScope(Root);
    for (const auto &Current : Scopes)
    {
      RemoveFromScope(*Current);
    }
    MemberScopes.erase(&Target);
  }
} // namespace ink::semantic
