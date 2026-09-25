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

  // Owns lexical scopes and bindings, not their values. Context and values must
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
      // Only Function values may share a name. Rebinding the same value is a
      // no-op; signature legality and overload selection belong to later analysis.
      BindResult bind(Name BoundName, Value &ValueObject);

      // Searches the current scope and its parents; lookupLocal searches only the current scope.
      // The nearest scope containing the name hides the entire outer binding.
      // Invalid names and misses return null without interning.
      const Binding *lookup(Name BoundName) const noexcept;
      const Binding *lookupLocal(Name BoundName) const noexcept;

      // Searches only Owner's member scope, without changing the current scope or searching lexical parents.
      // Missing scopes, invalid names and misses return null; aliases of the same owner share one scope.
      const Binding *lookupMember(Value &Owner, Name MemberName) const noexcept;

    private:
      const SemanticContext &Context;
      std::vector<std::unique_ptr<Scope>> Scopes;
      std::unordered_map<Value *, Scope *> MemberScopes;
      Scope *CurrentScope;
  };
} // namespace ink::semantic

#endif
