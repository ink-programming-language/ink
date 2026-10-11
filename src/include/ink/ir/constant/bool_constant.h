#ifndef INK_IR_CONSTANT_BOOL_CONSTANT_H
#define INK_IR_CONSTANT_BOOL_CONSTANT_H

#include "ink/ir/constant/constant.h"
#include "ink/ir/type/bool_type.h"

namespace ink::ir
{
  class ConstantContext;

  // The borrowed BoolType must outlive the constant.
  class BoolConstant final : public Constant
  {
    public:
      bool value() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class ConstantContext;

      BoolConstant(const BoolType *ConstantType, bool Value) noexcept;

      bool StoredValue = false;
  };
} // namespace ink::ir

#endif
