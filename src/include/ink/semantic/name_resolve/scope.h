#ifndef INK_SEMANTIC_NAME_RESOLVE_SCOPE_H
#define INK_SEMANTIC_NAME_RESOLVE_SCOPE_H

#include "ink/semantic/name_resolve/binding.h"

#include <optional>
#include <unordered_map>
#include <utility>

namespace ink::semantic
{
  class ScopeStore;

  // Owns local bindings, but borrows its store, parent and optional owner identity.
  // Scope addresses remain stable until the store is destroyed, including after exiting a scope.
  class Scope final
  {
    public:
      Scope(const Scope &) = delete;
      Scope &operator=(const Scope &) = delete;
      Scope(Scope &&) = delete;
      Scope &operator=(Scope &&) = delete;

      auto &scopeStore(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.Store);
      }

      auto *parent(this auto &&Self) noexcept
      {
        return Self.Parent ? &std::forward_like<decltype(Self) &>(*Self.Parent) : nullptr;
      }

      const std::optional<ir::Value *> &owner() const noexcept;

      // Does not search parents. Binding addresses survive insertion, but not removal of that binding.
      const Binding *lookupLocal(core::Name Name) const noexcept;

    private:
      friend class ScopeStore;

      Scope(ScopeStore &Store, Scope *Parent, std::optional<ir::Value *> Owner = std::nullopt) noexcept;

      ScopeStore &Store;
      Scope *Parent;
      std::optional<ir::Value *> Owner;
      std::unordered_map<core::Name, Binding> Bindings;
  };
} // namespace ink::semantic

#endif
