#ifndef INK_SEMANTIC_NAME_RESOLVE_SCOPE_STORE_H
#define INK_SEMANTIC_NAME_RESOLVE_SCOPE_STORE_H

#include "ink/semantic/name_resolve/scope.h"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ink::semantic
{
  class NameResolver;

  enum class BindResult
  {
    Inserted,
    AlreadyBound,
    Conflict,
    InvalidName,
  };

  // Owns every scope and binding; resolvers and saved scope references must die first.
  // Borrowed targets must remain alive while registered; targets can be explicitly removed before destruction.
  // Target context compatibility and declaration/signature validation belong to the analyzer.
  // Access requires external synchronization, including when resolvers have independent cursors.
  class ScopeStore final
  {
    public:
      ScopeStore() noexcept;
      ~ScopeStore() noexcept;

      ScopeStore(const ScopeStore &) = delete;
      ScopeStore &operator=(const ScopeStore &) = delete;
      ScopeStore(ScopeStore &&) = delete;
      ScopeStore &operator=(ScopeStore &&) = delete;

      auto &rootScope(this auto &&Self) noexcept
      {
        return Self.Root;
      }

      // Foreign parents and duplicate owner identities return nullptr without changing the store.
      Scope *createScope(Scope &Parent);
      Scope *createScope(Scope &Parent, ir::Value &Owner);

      auto *memberScope(this auto &&Self, const ir::Value &Owner) noexcept
      {
        const auto Found = Self.MemberScopes.find(&Owner);
        return Found == Self.MemberScopes.end() ? nullptr : &std::forward_like<decltype(Self) &>(*Found->second);
      }

      // Removes this exact target from all aliases, overloads and owner indexes, preserving scope addresses.
      // Removing a module does not remove its declarations: remove each borrowed value before it dies too.
      void removeTarget(const ir::Value &Target) noexcept;

    private:
      friend class NameResolver;

      Scope &appendScope(Scope &Parent, std::optional<ir::Value *> Owner);
      BindResult bind(Scope &Destination, core::Name Name, ir::Value *Target, bool Overload);

      Scope Root;
      std::vector<std::unique_ptr<Scope>> Scopes;
      std::unordered_map<const ir::Value *, Scope *> MemberScopes;
  };
} // namespace ink::semantic

#endif
