#ifndef INK_IR_MODULE_GLOBAL_VARIABLE_H
#define INK_IR_MODULE_GLOBAL_VARIABLE_H

#include "ink/core/name/name.h"
#include "ink/ir/constant/constant.h"
#include "ink/ir/type/structure_type/pointer_type.h"

namespace ink::ir
{
  // Declares a global object and produces its address as an ordinary pointer value.
  // Borrows its pointer type and optional initializer; both must outlive this global variable.
  class GlobalVariable final : public Value
  {
    public:
      explicit GlobalVariable(const PointerType &AddressType, core::Name Name = core::NAME_None) noexcept;

      GlobalVariable(const GlobalVariable &) = delete;
      GlobalVariable &operator=(const GlobalVariable &) = delete;
      GlobalVariable(GlobalVariable &&) = delete;
      GlobalVariable &operator=(GlobalVariable &&) = delete;

      const PointerType *type() const noexcept override;
      // Describes the object reached through this global address, without loading it.
      const Type *variableType() const noexcept;
      // Describes the object's initializer, not the pointer value's initialization.
      bool isInitialized() const noexcept;
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      void setName(core::Name NewName) noexcept;

      const Constant *initializer() const noexcept;
      bool hasInitializer() const noexcept;
      // nullptr clears the initializer. Values must match the object type ignoring its top-level qualifier, or be the local pool's nullptr for a pointer variable.
      // Rejection preserves the previous initializer; a nullptr constant remains distinct from clearing it.
      bool setInitializer(const Constant *NewInitializer) noexcept;

      static bool classof(const Value *Value) noexcept;

    private:
      const PointerType *AddressType = nullptr;
      core::Name Name = core::NAME_None;
      const Constant *Initializer = nullptr;
  };
} // namespace ink::ir

#endif
