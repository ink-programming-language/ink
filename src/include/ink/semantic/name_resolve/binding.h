#ifndef INK_SEMANTIC_NAME_RESOLVE_BINDING_H
#define INK_SEMANTIC_NAME_RESOLVE_BINDING_H

#include "ink/core/name/name.h"

#include <span>
#include <vector>

namespace ink::ir
{
  class Value;
} // namespace ink::ir

namespace ink::semantic
{
  class ScopeStore;

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

      // The view is invalidated when this binding changes; targets remain caller-owned.
      std::span<ir::Value *const> targets() const noexcept
      {
        return Targets;
      }

    private:
      friend class ScopeStore;

      Binding(core::Name Name, ir::Value *Target, bool OverloadSet)
          : Name(Name),
            Targets{Target},
            OverloadSet(OverloadSet)
      {
      }

      core::Name Name;
      std::vector<ir::Value *> Targets;
      bool OverloadSet;
  };
} // namespace ink::semantic

#endif
