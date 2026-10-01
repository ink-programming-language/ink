#ifndef INK_SEMANTIC_CONTEXT_H
#define INK_SEMANTIC_CONTEXT_H

#include "ink/ir/context.h"
#include "ink/semantic/name_resolve/scope_store.h"
#include "ink/semantic/comptime_state.h"

namespace ink::semantic
{
  // Combines independent IR storage with frontend scopes and bindings.
  // ScopeStore unregisters its observer before IR storage is destroyed.
  class SemanticContext final
  {
    public:
      FORCE_INLINE explicit SemanticContext(core::CompilationContext &Compilation)
          : IR(Compilation),
            Comptime(IR)
      {
        Scopes.reset(new ScopeStore(*this));
      }

      FORCE_INLINE ~SemanticContext() = default;
      SemanticContext(const SemanticContext &) = delete;
      SemanticContext &operator=(const SemanticContext &) = delete;
      SemanticContext(SemanticContext &&) = delete;
      SemanticContext &operator=(SemanticContext &&) = delete;

      FORCE_INLINE ir::IRContext &irContext() noexcept
      {
        return IR;
      }

      FORCE_INLINE const ir::IRContext &irContext() const noexcept
      {
        return IR;
      }

      FORCE_INLINE core::CompilationContext &compilationContext() noexcept
      {
        return IR.compilationContext();
      }

      FORCE_INLINE const core::CompilationContext &compilationContext() const noexcept
      {
        return IR.compilationContext();
      }

      FORCE_INLINE ir::NamePool &namePool() noexcept
      {
        return IR.namePool();
      }

      FORCE_INLINE const ir::NamePool &namePool() const noexcept
      {
        return IR.namePool();
      }

      FORCE_INLINE ScopeStore &scopeStore() noexcept
      {
        return *Scopes;
      }

      FORCE_INLINE const ScopeStore &scopeStore() const noexcept
      {
        return *Scopes;
      }

      FORCE_INLINE ir::TypePool &typePool() noexcept
      {
        return IR.typePool();
      }

      FORCE_INLINE const ir::TypePool &typePool() const noexcept
      {
        return IR.typePool();
      }

      FORCE_INLINE ir::ConstantPool &constantPool() noexcept
      {
        return IR.constantPool();
      }

      FORCE_INLINE const ir::ConstantPool &constantPool() const noexcept
      {
        return IR.constantPool();
      }

      FORCE_INLINE const std::vector<std::unique_ptr<ir::Module>> &modules() const noexcept
      {
        return IR.modules();
      }

      ComptimeState &comptimeState() noexcept
      {
        return Comptime;
      }

    private:
      ir::IRContext IR;
      ComptimeState Comptime;
      std::unique_ptr<ScopeStore> Scopes;
  };
} // namespace ink::semantic

#endif
