#ifndef INK_IR_CONSTANT_NULL_POINTER_CONSTANT_H
#define INK_IR_CONSTANT_NULL_POINTER_CONSTANT_H

#include "ink/ir/constant/constant.h"
#include "ink/ir/type/pointer_type.h"

namespace ink::ir
{
  class ConstantContext;

  // Represents a typed null pointer without storing a host address.
  // The borrowed PointerType must outlive the constant.
  class NullPointerConstant final : public Constant
  {
    public:
      static bool classof(const Value *Value) noexcept;

    private:
      friend class ConstantContext;

      explicit NullPointerConstant(const PointerType *ConstantType) noexcept;
  };
} // namespace ink::ir

#endif
