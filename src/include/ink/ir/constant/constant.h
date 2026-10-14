#ifndef INK_IR_CONSTANT_CONSTANT_H
#define INK_IR_CONSTANT_CONSTANT_H

#include "ink/ir/type/type.h"

namespace ink::ir
{
  // A compile-time value with a borrowed, non-null type that must outlive it.
  // Storage layout belongs to type(); constants themselves have no layout API.
  class Constant : public Value
  {
    public:
      ~Constant() noexcept override = 0;

      const Type *type() const noexcept override;
      // Accepts the same type identity, or the pool's nullptr value for any pointer type in its Module.
      // The destination supplies the concrete pointer type; this constant keeps its NullPointerType.
      bool canInitialize(const Type *DestinationType) const noexcept;
      static bool classof(const Value *Value) noexcept;

    protected:
      Constant(ValueKind Kind, const Type *ConstantType) noexcept;

    private:
      const Type *ConstantType = nullptr;
  };
} // namespace ink::ir

#endif
