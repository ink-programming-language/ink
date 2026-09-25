#ifndef INK_SEMANTIC_NAME_RESOLVE_BINDING_H
#define INK_SEMANTIC_NAME_RESOLVE_BINDING_H

#include "ink/semantic/model/name/name.h"

#include <span>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace ink::semantic
{
  class NameResolver;
  class Value;
  class Decl;

  template <typename T>
  concept BindingTarget = std::is_same_v<T, Value *> || std::is_same_v<T, Decl *>;

  template <BindingTarget T>
  class Binding final
  {
    public:
      Name name() const noexcept
      {
        return BoundName;
      }

      // Even a single function or generic function is an overload set. Function-typed variables are not.
      bool isOverloadSet() const noexcept
      {
        return OverloadSet;
      }

      // In insertion order. A later insertion into this binding invalidates the span.
      std::span<const T> targets() const noexcept
      {
        return Targets;
      }

    private:
      Binding(Name BoundName, bool OverloadSet)
          : BoundName(BoundName),
            OverloadSet(OverloadSet)
      {
      }

      Name BoundName;
      bool OverloadSet;
      std::vector<T> Targets;

      friend class NameResolver;
  };

  template <BindingTarget T>
  using BindingTable = std::unordered_map<Name, Binding<T>>;
} // namespace ink::semantic

#endif
