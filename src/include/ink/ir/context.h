#ifndef INK_IR_CONTEXT_H
#define INK_IR_CONTEXT_H

#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/type/type_pool.h"
#include "ink/target/target_context.h"

#include <unordered_set>

namespace ink::ir
{
  class Module;

  // Owns target-specific builtin/structural types and constants, but no modules or builders.
  // Modules and all users of pooled IR must be destroyed before this context.
  // Nominal types remain module-owned: all uses of them, including through pooled types,
  // must finish before their module is destroyed. Module unloading/cache invalidation is not supported.
  // Operations require external synchronization.
  class IRContext final
  {
    public:
      explicit IRContext(target::TargetContext Target);
      ~IRContext() noexcept;

      IRContext(const IRContext &) = delete;
      IRContext &operator=(const IRContext &) = delete;
      IRContext(IRContext &&) = delete;
      IRContext &operator=(IRContext &&) = delete;

      const target::TargetContext &targetContext() const noexcept;

      auto &typePool(this auto &&Self) noexcept
      {
        return Self.Types;
      }

      auto &constantPool(this auto &&Self) noexcept
      {
        return Self.Constants;
      }

      // Recognizes pooled types and registered module-owned nominal types without taking ownership.
      bool containsType(const Type *TypeValue) const noexcept;

    private:
      friend class Module;

      void registerNominalType(const Type *TypeValue);
      void unregisterNominalType(const Type *TypeValue) noexcept;

      const target::TargetContext Target;
      TypePool Types;
      // Constants are destroyed before the pooled types they borrow.
      ConstantPool Constants;
      // Validation-only registry; modules retain ownership and unregister before releasing their types.
      std::unordered_set<const Type *> NominalTypes;
  };
} // namespace ink::ir

#endif
