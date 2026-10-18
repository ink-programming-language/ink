#ifndef INK_SEMANTIC_NAME_RESOLVE_BINDING_H
#define INK_SEMANTIC_NAME_RESOLVE_BINDING_H

#include "ink/core/name/name.h"

#include <optional>
#include <span>
#include <vector>

namespace ink::ir
{
  class Type;
  class Value;
} // namespace ink::ir

namespace ink::semantic
{
  class ScopeStore;

  // Borrows the declared type; the single binding target is the variable's storage object.
  struct VariableBindingInfo
  {
      const ir::Type *DeclaredType;
      bool Mutable;
  };

  // Every named IR entity, including a module, is borrowed through its Value identity.
  class Binding final
  {
    public:
      core::Name name() const noexcept
      {
        return Name;
      }

      bool isOverloadSet() const noexcept
      {
        return OverloadSet;
      }

      // Metadata and its address remain stable until this binding is removed.
      const VariableBindingInfo *variableInfo() const noexcept
      {
        return VariableInfo ? &*VariableInfo : nullptr;
      }

      // The view is invalidated when this binding changes; targets remain caller-owned.
      std::span<ir::Value *const> targets() const noexcept
      {
        return Targets;
      }

    private:
      friend class ScopeStore;

      Binding(core::Name Name, ir::Value *Target, bool OverloadSet, std::optional<VariableBindingInfo> VariableInfo)
          : Name(Name),
            Targets{Target},
            OverloadSet(OverloadSet),
            VariableInfo(VariableInfo)
      {
      }

      core::Name Name;
      std::vector<ir::Value *> Targets;
      bool OverloadSet;
      std::optional<VariableBindingInfo> VariableInfo;
  };
} // namespace ink::semantic

#endif
