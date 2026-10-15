#ifndef INK_SEMANTIC_NAME_RESOLVE_SCOPE_GUARD_H
#define INK_SEMANTIC_NAME_RESOLVE_SCOPE_GUARD_H

#include <utility>

namespace ink::ir
{
  class Value;
} // namespace ink::ir

namespace ink::semantic
{
  class NameResolver;
  class Scope;

  // Restores the exact previous scope, including on early return. Resolver and store must outlive the guard.
  // Successful guards preserve their scopes and bindings; a failed entry performs no restoration.
  class ScopeGuard final
  {
    public:
      explicit ScopeGuard(NameResolver &Resolver);
      ScopeGuard(NameResolver &Resolver, Scope &ExistingScope) noexcept;
      ScopeGuard(NameResolver &Resolver, ir::Value &Owner);
      ~ScopeGuard() noexcept;

      ScopeGuard(const ScopeGuard &) = delete;
      ScopeGuard &operator=(const ScopeGuard &) = delete;
      ScopeGuard(ScopeGuard &&) = delete;
      ScopeGuard &operator=(ScopeGuard &&) = delete;

      auto *scope(this auto &&Self) noexcept
      {
        return Self.EnteredScope ? &std::forward_like<decltype(Self) &>(*Self.EnteredScope) : nullptr;
      }

    private:
      NameResolver &Resolver;
      Scope *SavedScope;
      Scope *EnteredScope;
  };
} // namespace ink::semantic

#endif
