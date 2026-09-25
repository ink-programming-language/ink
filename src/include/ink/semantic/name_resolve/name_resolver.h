#ifndef INK_SEMANTIC_NAME_RESOLVE_NAME_RESOLVER_H
#define INK_SEMANTIC_NAME_RESOLVE_NAME_RESOLVER_H

#include "ink/semantic/name_resolve/scope.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace ink::semantic
{
  class SemanticContext;
  class Value;

  // Owns lexical scopes and bindings, not their values or declarations. Context and targets must
  // outlive the resolver. Names must use Context's name pool; compact Name indices
  // cannot prove provenance.
  class NameResolver final
  {
    public:
      enum class BindResult
      {
        Inserted,
        AlreadyBound,
        Conflict,
        InvalidName,
        ForeignValue,
        ForeignDecl,
        InvalidDecl,
      };

      explicit NameResolver(const SemanticContext &Context);
      NameResolver(const NameResolver &) = delete;
      NameResolver &operator=(const NameResolver &) = delete;
      NameResolver(NameResolver &&) = delete;
      NameResolver &operator=(NameResolver &&) = delete;

      Scope &rootScope() noexcept
      {
        return *Scopes.front();
      }

      const Scope &rootScope() const noexcept
      {
        return *Scopes.front();
      }

      Scope &currentScope() noexcept
      {
        return *CurrentScope;
      }

      const Scope &currentScope() const noexcept
      {
        return *CurrentScope;
      }

      // Creates and enters a child of the current scope.
      // Scopes and binding addresses remain stable until resolver destruction, even after exit.
      Scope &enterScope();

      // Creates and enters Owner's member scope under the current scope.
      // Foreign owners or owners already associated with a scope return null without changing state.
      Scope *enterScope(Value &Owner);

      // Restores the parent without destroying the exited scope; false at the root.
      bool exitScope() noexcept;

      // Binds in the current scope. Names may alias their values.
      // Only Function values and generic FunctionDecls may share a name. Rebinding the same target is a
      // no-op; signature legality and overload selection belong to later analysis.
      BindResult bind(Name BoundName, Value &ValueObject);
      // Accepts only local generic FunctionDecl/ClassDecl definitions, not ModuleDecl.
      // The first successful binding records the definition scope; later aliases preserve it.
      BindResult bind(Name BoundName, Decl &Declaration);

      const Scope *definitionScope(const Decl &Declaration) const noexcept;

      // Searches the current scope and its parents; lookupLocal searches only the current scope.
      // The nearest scope containing either target category hides both outer categories.
      // Select Decl * explicitly for generic candidates; the default preserves value lookup.
      // Mixed function overloads are retrieved as two typed lists from that same scope.
      // Invalid names and misses return null without interning.
      template <BindingTarget T = Value *>
      const Binding<T> *lookup(Name BoundName) const noexcept
      {
        const Scope *Found = findScope(BoundName);
        return Found ? lookupInScope<T>(*Found, BoundName) : nullptr;
      }

      template <BindingTarget T = Value *>
      const Binding<T> *lookupLocal(Name BoundName) const noexcept
      {
        return lookupInScope<T>(*CurrentScope, BoundName);
      }

      // Searches only Owner's member scope, without changing the current scope or searching lexical parents.
      // Missing scopes, invalid names and misses return null; aliases of the same owner share one scope.
      template <BindingTarget T = Value *>
      const Binding<T> *lookupMember(Value &Owner, Name MemberName) const noexcept
      {
        const auto Found = MemberScopes.find(&Owner);
        return Found == MemberScopes.end() ? nullptr : lookupInScope<T>(*Found->second, MemberName);
      }

    private:
      const Scope *findScope(Name BoundName) const noexcept;

      template <BindingTarget T>
      static const Binding<T> *lookupInScope(const Scope &ScopeValue, Name BoundName) noexcept
      {
        if constexpr (std::is_same_v<T, Value *>)
        {
          const auto Found = ScopeValue.ValueBindings.find(BoundName);
          return Found == ScopeValue.ValueBindings.end() ? nullptr : &Found->second;
        }
        else
        {
          const auto Found = ScopeValue.DeclBindings.find(BoundName);
          return Found == ScopeValue.DeclBindings.end() ? nullptr : &Found->second;
        }
      }

      const SemanticContext &Context;
      std::vector<std::unique_ptr<Scope>> Scopes;
      std::unordered_map<Value *, Scope *> MemberScopes;
      std::unordered_map<const Decl *, Scope *> DefinitionScopes;
      Scope *CurrentScope;
  };
} // namespace ink::semantic

#endif
