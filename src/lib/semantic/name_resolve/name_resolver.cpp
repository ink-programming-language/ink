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

  template <BindingTarget T>
  NameResolver::BindResult NameResolver::bindInTable(BindingTable<T> &Bindings, Name BoundName, T Target, bool OverloadSet)
  {
    auto [Entry, Inserted] = Bindings.try_emplace(BoundName, Binding<T>(BoundName, OverloadSet));
    Binding<T> &Found = Entry->second;
    for (T Existing : Found.Targets)
    {
      if (Existing == Target)
      {
        return BindResult::AlreadyBound;
      }
    }
    if (!Inserted && (!Found.OverloadSet || !OverloadSet))
    {
      return BindResult::Conflict;
    }
    Found.Targets.push_back(Target);
    return BindResult::Inserted;
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
    const auto *Generic = lookupLocal<Decl *>(BoundName);
    if (Generic && (!Generic->isOverloadSet() || !OverloadSet))
    {
      return BindResult::Conflict;
    }
    return bindInTable(CurrentScope->ValueBindings, BoundName, &ValueObject, OverloadSet);
  }

  NameResolver::BindResult NameResolver::bind(Name BoundName, Decl &Declaration)
  {
    if (!Context.owns(Declaration))
    {
      return BindResult::ForeignDecl;
    }
    const bool OverloadSet = FunctionDecl::classof(&Declaration);
    if (!OverloadSet && !ClassDecl::classof(&Declaration))
    {
      return BindResult::InvalidDecl;
    }
    if (!Context.namePool().contains(BoundName))
    {
      return BindResult::InvalidName;
    }
    const auto *Values = lookupLocal(BoundName);
    if (Values && (!Values->isOverloadSet() || !OverloadSet))
    {
      return BindResult::Conflict;
    }
    const BindResult Result = bindInTable(CurrentScope->DeclBindings, BoundName, &Declaration, OverloadSet);
    if (Result == BindResult::Inserted)
    {
      DefinitionScopes.try_emplace(&Declaration, CurrentScope);
    }
    return Result;
  }

  const Scope *NameResolver::definitionScope(const Decl &Declaration) const noexcept
  {
    const auto Found = DefinitionScopes.find(&Declaration);
    return Found == DefinitionScopes.end() ? nullptr : Found->second;
  }

  const Scope *NameResolver::findScope(Name BoundName) const noexcept
  {
    if (!Context.namePool().contains(BoundName))
    {
      return nullptr;
    }
    for (const Scope *Current = CurrentScope; Current; Current = Current->Parent)
    {
      if (Current->ValueBindings.contains(BoundName) || Current->DeclBindings.contains(BoundName))
      {
        return Current;
      }
    }
    return nullptr;
  }
} // namespace ink::semantic
