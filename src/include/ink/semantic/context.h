#ifndef INK_SEMANTIC_CONTEXT_H
#define INK_SEMANTIC_CONTEXT_H

#include "ink/core/core_define.h"
#include "ink/core/context.h"
#include "ink/semantic/model/constant/constant_pool.h"
#include "ink/semantic/model/module/module.h"
#include "ink/semantic/model/name/name_pool.h"
#include "ink/semantic/model/type/type_pool.h"
#include "ink/semantic/name_resolve/scope_store.h"

#include <memory>
#include <vector>

namespace ink::semantic
{
  class IRBuilder;

  // Owns shared scopes, types, constants and root modules; modules own independent declaration and IR trees.
  // CompilationContext must outlive this context, which must outlive detached IR owners.
  // Borrowed AST units must outlive their declaration-owning modules.
  class SemanticContext final
  {
    public:
      FORCE_INLINE explicit SemanticContext(core::CompilationContext &Compilation)
          : Compilation(Compilation)
      {
        Scopes.reset(new ScopeStore(*this));
        Types.reset(new TypePool(*this));
        Constants.reset(new ConstantPool(*this));
      }

      FORCE_INLINE ~SemanticContext() = default;
      SemanticContext(const SemanticContext &) = delete;
      SemanticContext &operator=(const SemanticContext &) = delete;
      SemanticContext(SemanticContext &&) = delete;
      SemanticContext &operator=(SemanticContext &&) = delete;

      FORCE_INLINE core::CompilationContext &compilationContext() noexcept
      {
        return Compilation;
      }

      FORCE_INLINE const core::CompilationContext &compilationContext() const noexcept
      {
        return Compilation;
      }

      FORCE_INLINE NamePool &namePool() noexcept
      {
        return Names;
      }

      FORCE_INLINE const NamePool &namePool() const noexcept
      {
        return Names;
      }

      FORCE_INLINE ScopeStore &scopeStore() noexcept
      {
        return *Scopes;
      }

      FORCE_INLINE const ScopeStore &scopeStore() const noexcept
      {
        return *Scopes;
      }

      FORCE_INLINE TypePool &typePool() noexcept
      {
        return *Types;
      }

      FORCE_INLINE const TypePool &typePool() const noexcept
      {
        return *Types;
      }

      FORCE_INLINE ConstantPool &constantPool() noexcept
      {
        return *Constants;
      }

      FORCE_INLINE const ConstantPool &constantPool() const noexcept
      {
        return *Constants;
      }

      FORCE_INLINE const std::vector<std::unique_ptr<Module>> &modules() const noexcept
      {
        return Modules;
      }

    private:
      core::CompilationContext &Compilation;
      NamePool Names;
      // Values and declarations unregister bindings during destruction, before scopes are released.
      std::unique_ptr<ScopeStore> Scopes;
      std::unique_ptr<TypePool> Types;
      std::unique_ptr<ConstantPool> Constants;
      std::vector<std::unique_ptr<Module>> Modules;

      friend class IRBuilder;
      friend class Value;
      friend class Decl;
  };
} // namespace ink::semantic

#endif
