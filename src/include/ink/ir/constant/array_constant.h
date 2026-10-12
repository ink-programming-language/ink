#ifndef INK_IR_CONSTANT_ARRAY_CONSTANT_H
#define INK_IR_CONSTANT_ARRAY_CONSTANT_H

#include "ink/ir/constant/constant.h"
#include "ink/ir/type/array_type.h"

#include <cstddef>
#include <vector>

namespace ink::ir
{
  class ConstantPool;

  // Owns the element list; the type and each element are borrowed and must outlive this constant.
  // Pointer elements may share the pool's NullPointerType constant; the array supplies their destination pointer type.
  class ArrayConstant final : public Constant
  {
    public:
      const std::vector<const Constant *> &elements() const noexcept;
      std::size_t elementCount() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class ConstantPool;

      ArrayConstant(const ArrayType *ConstantType, std::vector<const Constant *> Elements) noexcept;

      std::vector<const Constant *> Elements;
  };
} // namespace ink::ir

#endif
