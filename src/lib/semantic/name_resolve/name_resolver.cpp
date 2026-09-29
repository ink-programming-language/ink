#include "ink/semantic/name_resolve/name_resolver.h"

#include "ink/semantic/context.h"
#include "ink/semantic/model/decl/class_decl.h"
#include "ink/semantic/model/decl/function_decl.h"
#include "ink/semantic/model/function/function.h"

namespace ink::semantic
{
  NameResolver::NameResolver(SemanticContext &Context) noexcept
      : NameResolver(Context.scopeStore().rootScope())
  {
  }

  NameResolver::NameResolver(Scope &InitialScope) noexcept
      : Store(InitialScope.Store),
        CurrentScope(&InitialScope)
  {
  }

  Scope &NameResolver::enterScope()
  {
    CurrentScope = &Store.createScope(*CurrentScope);
    return *CurrentScope;
  }

  Scope *NameResolver::enterScope(Value &Owner)
  {
    if (&Owner.context() != &Store.context() || Store.memberScope(Owner))
    {
      return nullptr;
    }
    Scope &Created = enterScope();
    Store.MemberScopes.emplace(&Owner, &Created);
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
    if (&ValueObject.context() != &Store.context())
    {
      return BindResult::ForeignValue;
    }
    if (!Store.context().namePool().contains(BoundName))
    {
      return BindResult::InvalidName;
    }
    const bool OverloadSet = Function::classof(&ValueObject);
    const auto *Generic = lookupLocal<Decl *>(BoundName);
    if (Generic && (!Generic->isOverloadSet() || !OverloadSet))
    {
      return BindResult::Conflict;
    }
    const BindResult Result = bindInTable(CurrentScope->ValueBindings, BoundName, &ValueObject, OverloadSet);
    if (Result == BindResult::Inserted)
    {
      Store.ValueBindingLocations[&ValueObject].push_back({CurrentScope, BoundName});
    }
    return Result;
  }

  NameResolver::BindResult NameResolver::bind(Name BoundName, Decl &Declaration)
  {
    if (&Declaration.module().context() != &Store.context())
    {
      return BindResult::ForeignDecl;
    }
    const bool OverloadSet = FunctionDecl::classof(&Declaration);
    if (!OverloadSet && !ClassDecl::classof(&Declaration))
    {
      return BindResult::InvalidDecl;
    }
    if (!Store.context().namePool().contains(BoundName))
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
      Store.DeclBindingLocations[&Declaration].push_back({CurrentScope, BoundName});
      Store.DefinitionScopes.try_emplace(&Declaration, CurrentScope);
    }
    return Result;
  }

  Scope *NameResolver::definitionScope(const Decl &Declaration) noexcept
  {
    return Store.definitionScope(Declaration);
  }

  const Scope *NameResolver::definitionScope(const Decl &Declaration) const noexcept
  {
    return Store.definitionScope(Declaration);
  }

  const Scope *NameResolver::findScope(Name BoundName) const noexcept
  {
    if (!Store.context().namePool().contains(BoundName))
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
