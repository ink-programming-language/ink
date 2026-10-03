#ifndef INK_IR_CONSTANT_ARRAY_CONSTANT_H
#define INK_IR_CONSTANT_ARRAY_CONSTANT_H

#include "ink/ir/constant/constant.h"
#include "ink/ir/type/array_type.h"

#include <span>
#include <vector>

namespace ink::ir
{
  // Immutable, interned array data. Elements are canonical constants in the same context.
  class ArrayConstant final : public Constant
  {
    public:
      const ArrayType &arrayType() const noexcept
      {
        return static_cast<const ArrayType &>(type());
      }

      std::span<const Constant *const> elements() const noexcept
      {
        return Elements;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::ArrayConstant;
      }

    private:
      ArrayConstant(const ArrayType &ValueType, std::span<const Constant *const> Elements)
          : Constant(ValueKind::ArrayConstant, ValueType),
            Elements(Elements.begin(), Elements.end())
      {
      }

      std::vector<const Constant *> Elements;

      friend class ConstantPool;
  };
} // namespace ink::ir

#endif
