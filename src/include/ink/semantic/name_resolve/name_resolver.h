#ifndef INK_SEMANTIC_NAME_RESOLVE_NAME_RESOLVER_H
#define INK_SEMANTIC_NAME_RESOLVE_NAME_RESOLVER_H

#include "ink/semantic/name_resolve/scope_store.h"

#include <utility>

namespace ink::ir
{
  class Function;
} // namespace ink::ir

namespace ink::semantic
{
  class SemanticContext;
  class ScopeGuard;

  // A per-analysis cursor over caller-owned scope storage. Destroying it never removes scopes or bindings.
  class NameResolver final
  {
    public:
      using BindResult = semantic::BindResult;

      explicit NameResolver(ScopeStore &Store) noexcept;
      explicit NameResolver(SemanticContext &Context) noexcept;
      explicit NameResolver(Scope &StartingScope) noexcept;

      NameResolver(const NameResolver &) = delete;
      NameResolver &operator=(const NameResolver &) = delete;
      NameResolver(NameResolver &&) = delete;
      NameResolver &operator=(NameResolver &&) = delete;

      auto &scopeStore(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.Store);
      }

      auto &rootScope(this auto &&Self) noexcept
      {
        return Self.scopeStore().rootScope();
      }

      auto &currentScope(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(*Self.CurrentScope);
      }

      Scope &enterScope();
      // Owned entries create a new member scope; duplicates fail without moving the cursor.
      Scope *enterScope(ir::Value &Owner);
      // Resumes an existing scope from this store, including scopes used by an earlier resolver.
      bool enterScope(Scope &ExistingScope) noexcept;
      bool exitScope() noexcept;

      // Ordinary bindings are exclusive, including ordinary function bindings.
      // Targets carry their own type and storage properties; aliases preserve the same value identity.
      // Repeating the same target returns AlreadyBound without changing the binding.
      BindResult bind(core::Name Name, ir::Value &Target);
      // Only explicit overload bindings can accumulate distinct functions; signature checks are separate.
      BindResult bindOverload(core::Name Name, ir::Function &Target);

      // Lookup stops at the nearest binding; it never merges overloads from outer scopes.
      const Binding *lookup(core::Name Name) const noexcept;
      const Binding *lookupLocal(core::Name Name) const noexcept;
      // Member lookup only inspects the owner's scope and never walks its lexical parents.
      const Binding *lookupMember(const ir::Value &Owner, core::Name Name) const noexcept;

    private:
      friend class ScopeGuard;

      ScopeStore &Store;
      Scope *CurrentScope;
  };
} // namespace ink::semantic

#endif
