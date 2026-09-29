#ifndef INK_IR_CONTEXT_H
#define INK_IR_CONTEXT_H

#include "ink/core/core_define.h"
#include "ink/core/context.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/module/module.h"
#include "ink/ir/name/name_pool.h"
#include "ink/ir/type/type_pool.h"

#include <memory>
#include <vector>

namespace ink::ir
{
  class IRBuilder;
  class LifetimeObserver;
  class Decl;

  // Owns shared IR storage and root modules, including their borrowed-AST declaration trees.
  // CompilationContext and borrowed AST units must outlive the context.
  // Detached owners and lifetime observers must be destroyed before the context.
  class IRContext final
  {
    public:
      FORCE_INLINE explicit IRContext(core::CompilationContext &Compilation)
          : Compilation(Compilation)
      {
        Types.reset(new TypePool(*this));
        Constants.reset(new ConstantPool(*this));
      }

      FORCE_INLINE ~IRContext() = default;
      IRContext(const IRContext &) = delete;
      IRContext &operator=(const IRContext &) = delete;
      IRContext(IRContext &&) = delete;
      IRContext &operator=(IRContext &&) = delete;

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
      void notifyDestroyed(Value &Target) const noexcept;
      void notifyDestroyed(Decl &Target) const noexcept;

      core::CompilationContext &Compilation;
      NamePool Names;
      // Initialized before pools; notifications remain available while owned values are destroyed.
      std::vector<LifetimeObserver *> Observers;
      std::unique_ptr<TypePool> Types;
      std::unique_ptr<ConstantPool> Constants;
      std::vector<std::unique_ptr<Module>> Modules;

      friend class IRBuilder;
      friend class Value;
      friend class Decl;
      friend class LifetimeObserver;
  };
} // namespace ink::ir

#endif
