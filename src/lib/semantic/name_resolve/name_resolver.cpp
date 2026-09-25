#include "ink/semantic/name_resolve/name_resolver.h"

#include "ink/semantic/context.h"

#include <utility>

namespace ink::semantic
{
  NameResolver::NameResolver(const SemanticContext &Context)
      : Context(Context)
  {
    Scopes.push_back(std::unique_ptr<Scope>(new Scope(nullptr)));
    CurrentScope = Scopes.front().get();
  }

  Scope &NameResolver::enterScope()
  {
    auto Result = std::unique_ptr<Scope>(new Scope(CurrentScope));
    Scope *Pointer = Result.get();
    Scopes.push_back(std::move(Result));
    CurrentScope = Pointer;
    return *CurrentScope;
  }

  Scope *NameResolver::enterScope(Value &Owner)
  {
    if (&Owner.context() != &Context || MemberScopes.contains(&Owner))
    {
      return nullptr;
    }
    Scope &Created = enterScope();
    MemberScopes.emplace(&Owner, &Created);
    return &Created;
  }

  bool NameResolver::exitScope() noexcept
  {
    if (!CurrentScope->Parent)
    {
      return false;
    }
    CurrentScope = CurrentScope->Parent;
    return true;
  }

  NameResolver::BindResult NameResolver::bind(Name BoundName, Value &ValueObject)
  {
    if (&ValueObject.context() != &Context)
    {
      return BindResult::ForeignValue;
    }
    if (!Context.namePool().contains(BoundName))
    {
      return BindResult::InvalidName;
    }
    const bool OverloadSet = Function::classof(&ValueObject);
    auto [Entry, Inserted] = CurrentScope->Bindings.try_emplace(BoundName, Binding(BoundName, OverloadSet));
    Binding &Found = Entry->second;
    for (Value *Existing : Found.Targets)
    {
      if (Existing == &ValueObject)
      {
        return BindResult::AlreadyBound;
      }
    }
    if (!Inserted && (!Found.OverloadSet || !OverloadSet))
    {
      return BindResult::Conflict;
    }
    Found.Targets.push_back(&ValueObject);
    return BindResult::Inserted;
  }

  const Binding *NameResolver::lookup(Name BoundName) const noexcept
  {
    if (!Context.namePool().contains(BoundName))
    {
      return nullptr;
    }
    for (const Scope *Current = CurrentScope; Current; Current = Current->Parent)
    {
      const auto Found = Current->Bindings.find(BoundName);
      if (Found != Current->Bindings.end())
      {
        return &Found->second;
      }
    }
    return nullptr;
  }

  const Binding *NameResolver::lookupLocal(Name BoundName) const noexcept
  {
    if (!Context.namePool().contains(BoundName))
    {
      return nullptr;
    }
    const auto Found = CurrentScope->Bindings.find(BoundName);
    return Found == CurrentScope->Bindings.end() ? nullptr : &Found->second;
  }

  const Binding *NameResolver::lookupMember(Value &Owner, Name MemberName) const noexcept
  {
    if (!Context.namePool().contains(MemberName))
    {
      return nullptr;
    }
    const auto ScopeEntry = MemberScopes.find(&Owner);
    if (ScopeEntry == MemberScopes.end())
    {
      return nullptr;
    }
    const auto &Bindings = ScopeEntry->second->Bindings;
    const auto Found = Bindings.find(MemberName);
    return Found == Bindings.end() ? nullptr : &Found->second;
  }
} // namespace ink::semantic
