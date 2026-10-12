#ifndef INK_IR_CONSTANT_NULL_POINTER_CONSTANT_H
#define INK_IR_CONSTANT_NULL_POINTER_CONSTANT_H

#include "ink/ir/constant/constant.h"
#include "ink/ir/type/builtin_type/null_pointer_type.h"

namespace ink::ir
{
  class ConstantPool;

  // Represents the context's unique null pointer literal without storing a host address.
  // The borrowed NullPointerType must outlive the constant.
  class NullPointerConstant final : public Constant
  {
    public:
      static bool classof(const Value *Value) noexcept;

    private:
      friend class ConstantPool;

      explicit NullPointerConstant(const NullPointerType *ConstantType) noexcept;
  };
} // namespace ink::ir

#endif
