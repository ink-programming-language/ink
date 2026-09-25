#ifndef INK_SEMANTIC_NAME_RESOLVE_SCOPE_H
#define INK_SEMANTIC_NAME_RESOLVE_SCOPE_H

#include "ink/semantic/name_resolve/binding.h"

#include <unordered_map>

namespace ink::semantic
{
  class NameResolver;

  class Scope final
  {
    public:
      Scope(const Scope &) = delete;
      Scope &operator=(const Scope &) = delete;
      Scope(Scope &&) = delete;
      Scope &operator=(Scope &&) = delete;

      const Scope *parent() const noexcept
      {
        return Parent;
      }

    private:
      explicit Scope(Scope *Parent) noexcept
          : Parent(Parent)
      {
      }

      Scope *Parent;
      std::unordered_map<Name, Binding> Bindings;

      friend class NameResolver;
  };
} // namespace ink::semantic

#endif
