#ifndef INK_SEMANTIC_NAME_RESOLVE_BINDING_H
#define INK_SEMANTIC_NAME_RESOLVE_BINDING_H

#include "ink/ir/name/name.h"

#include <span>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace ink::ir
{
  class Value;
  class Decl;
} // namespace ink::ir

namespace ink::semantic
{
  class NameResolver;
  class ScopeStore;

  template <typename T>
  concept BindingTarget = std::is_same_v<T, ir::Value *> || std::is_same_v<T, ir::Decl *>;

  template <BindingTarget T>
  class Binding final
  {
    public:
      ir::Name name() const noexcept
      {
        return BoundName;
      }

      // Even a single function or generic function is an overload set. Function-typed variables are not.
      bool isOverloadSet() const noexcept
      {
        return OverloadSet;
      }

      // In insertion order. Insertion or target destruction invalidates the span.
      std::span<const T> targets() const noexcept
      {
        return Targets;
      }

    private:
      Binding(ir::Name BoundName, bool OverloadSet)
          : BoundName(BoundName),
            OverloadSet(OverloadSet)
      {
      }

      ir::Name BoundName;
      bool OverloadSet;
      std::vector<T> Targets;

      friend class NameResolver;
      friend class ScopeStore;
  };

  template <BindingTarget T>
  using BindingTable = std::unordered_map<ir::Name, Binding<T>>;
} // namespace ink::semantic

#endif
