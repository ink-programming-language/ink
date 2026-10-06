#ifndef INK_SEMANTIC_NAME_RESOLVE_SCOPE_STORE_H
#define INK_SEMANTIC_NAME_RESOLVE_SCOPE_STORE_H

#include "ink/semantic/name_resolve/scope.h"
#include "ink/ir/lifetime_observer.h"

#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>

namespace ink::semantic
{
  class SemanticContext;

  // Context-owned lexical scopes and bindings. Targets remain borrowed and unregister on destruction.
  class ScopeStore final : private ir::LifetimeObserver
  {
    public:
      ~ScopeStore() override = default;
      ScopeStore(const ScopeStore &) = delete;
      ScopeStore &operator=(const ScopeStore &) = delete;
      ScopeStore(ScopeStore &&) = delete;
      ScopeStore &operator=(ScopeStore &&) = delete;

      const SemanticContext &context() const noexcept
      {
        return Context;
      }

      Scope &rootScope() noexcept
      {
        return *Scopes.front();
      }

      const Scope &rootScope() const noexcept
      {
        return *Scopes.front();
      }

      // Freezes the currently visible name and overload sets into a parentless scope.
      // Targets remain borrowed and mutable; a foreign source scope returns null.
      Scope *snapshotScope(const Scope &Source, const std::function<ir::Value *(ir::Value *)> &Transform = {});

      Scope *memberScope(const ir::Value &Owner) noexcept
      {
        const auto Found = MemberScopes.find(&Owner);
        return Found == MemberScopes.end() ? nullptr : Found->second;
      }

      const Scope *memberScope(const ir::Value &Owner) const noexcept
      {
        const auto Found = MemberScopes.find(&Owner);
        return Found == MemberScopes.end() ? nullptr : Found->second;
      }

      Scope *definitionScope(const ir::Decl &Declaration) noexcept
      {
        const auto Found = DefinitionScopes.find(&Declaration);
        return Found == DefinitionScopes.end() ? nullptr : Found->second;
      }

      const Scope *definitionScope(const ir::Decl &Declaration) const noexcept
      {
        const auto Found = DefinitionScopes.find(&Declaration);
        return Found == DefinitionScopes.end() ? nullptr : Found->second;
      }

    private:
      struct BindingLocation
      {
          Scope *ScopeValue;
          ir::Name BoundName;
      };

      explicit ScopeStore(SemanticContext &Context);
      Scope &createScope(Scope &Parent);
      void valueDestroyed(ir::Value &Target) noexcept override;
      void declDestroyed(ir::Decl &Target) noexcept override;

      template <BindingTarget T>
      static void removeBindings(T Target, std::unordered_map<T, std::vector<BindingLocation>> &Locations) noexcept;

      const SemanticContext &Context;
      std::vector<std::unique_ptr<Scope>> Scopes;
      std::unordered_map<const ir::Value *, Scope *> MemberScopes;
      std::unordered_map<const ir::Decl *, Scope *> DefinitionScopes;
      std::unordered_map<ir::Value *, std::vector<BindingLocation>> ValueBindingLocations;
      std::unordered_map<ir::Decl *, std::vector<BindingLocation>> DeclBindingLocations;

      friend class SemanticContext;
      friend class NameResolver;
  };
} // namespace ink::semantic

#endif
