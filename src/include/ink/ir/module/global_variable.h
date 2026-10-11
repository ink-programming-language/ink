#ifndef INK_IR_MODULE_GLOBAL_VARIABLE_H
#define INK_IR_MODULE_GLOBAL_VARIABLE_H

#include "ink/core/name.h"
#include "ink/ir/constant/constant.h"

namespace ink::ir
{
  // Borrows its type and optional initializer; both must outlive this global variable.
  class GlobalVariable final : public Value
  {
    public:
      explicit GlobalVariable(const Type &VariableType, core::Name Name = core::NAME_None) noexcept;

      GlobalVariable(const GlobalVariable &) = delete;
      GlobalVariable &operator=(const GlobalVariable &) = delete;
      GlobalVariable(GlobalVariable &&) = delete;
      GlobalVariable &operator=(GlobalVariable &&) = delete;

      const Type *type() const noexcept;
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      void setName(core::Name NewName) noexcept;

      const Constant *initializer() const noexcept;
      bool hasInitializer() const noexcept;
      // nullptr clears the initializer. A different type identity is rejected without changing it.
      bool setInitializer(const Constant *NewInitializer) noexcept;

      static bool classof(const Value *Value) noexcept;

    private:
      const Type *VariableType = nullptr;
      core::Name Name = core::NAME_None;
      const Constant *Initializer = nullptr;
  };
} // namespace ink::ir

#endif
