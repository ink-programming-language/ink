#ifndef INK_SEMANTIC_NAME_RESOLVE_SCOPE_H
#define INK_SEMANTIC_NAME_RESOLVE_SCOPE_H

#include "ink/semantic/name_resolve/binding.h"

namespace ink::semantic
{
  class NameResolver;
  class ScopeStore;

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
      Scope(ScopeStore &Store, Scope *Parent) noexcept
          : Store(Store),
            Parent(Parent)
      {
      }

      ScopeStore &Store;
      Scope *Parent;
      // Both maps participate in one lexical namespace.
      BindingTable<ir::Value *> ValueBindings;
      BindingTable<ir::Decl *> DeclBindings;

      friend class NameResolver;
      friend class ScopeStore;
  };
} // namespace ink::semantic

#endif
