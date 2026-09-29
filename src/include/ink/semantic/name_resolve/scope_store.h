#ifndef INK_SEMANTIC_NAME_RESOLVE_SCOPE_STORE_H
#define INK_SEMANTIC_NAME_RESOLVE_SCOPE_STORE_H

#include "ink/semantic/name_resolve/scope.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace ink::semantic
{
  class SemanticContext;

  // Context-owned lexical scopes and bindings. Targets remain borrowed and unregister on destruction.
  class ScopeStore final
  {
    public:
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

      Scope *memberScope(const Value &Owner) noexcept
      {
        const auto Found = MemberScopes.find(&Owner);
        return Found == MemberScopes.end() ? nullptr : Found->second;
      }

      const Scope *memberScope(const Value &Owner) const noexcept
      {
        const auto Found = MemberScopes.find(&Owner);
        return Found == MemberScopes.end() ? nullptr : Found->second;
      }

      Scope *definitionScope(const Decl &Declaration) noexcept
      {
        const auto Found = DefinitionScopes.find(&Declaration);
        return Found == DefinitionScopes.end() ? nullptr : Found->second;
      }

      const Scope *definitionScope(const Decl &Declaration) const noexcept
      {
        const auto Found = DefinitionScopes.find(&Declaration);
        return Found == DefinitionScopes.end() ? nullptr : Found->second;
      }

    private:
      struct BindingLocation
      {
          Scope *ScopeValue;
          Name BoundName;
      };

      explicit ScopeStore(const SemanticContext &Context);
      Scope &createScope(Scope &Parent);
      void forget(Value &Target) noexcept;
      void forget(Decl &Target) noexcept;

      template <BindingTarget T>
      static void removeBindings(T Target, std::unordered_map<T, std::vector<BindingLocation>> &Locations) noexcept;

      const SemanticContext &Context;
      std::vector<std::unique_ptr<Scope>> Scopes;
      std::unordered_map<const Value *, Scope *> MemberScopes;
      std::unordered_map<const Decl *, Scope *> DefinitionScopes;
      std::unordered_map<Value *, std::vector<BindingLocation>> ValueBindingLocations;
      std::unordered_map<Decl *, std::vector<BindingLocation>> DeclBindingLocations;

      friend class SemanticContext;
      friend class NameResolver;
      friend class Value;
      friend class Decl;
  };
} // namespace ink::semantic

#endif
