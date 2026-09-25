#ifndef INK_SEMANTIC_NAME_RESOLVE_SCOPE_H
#define INK_SEMANTIC_NAME_RESOLVE_SCOPE_H

#include "ink/semantic/name_resolve/binding.h"

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
      // Both maps participate in one lexical namespace.
      BindingTable<Value *> ValueBindings;
      BindingTable<Decl *> DeclBindings;

      friend class NameResolver;
  };
} // namespace ink::semantic

#endif
