#ifndef INK_IR_ARRAY_TYPE_H
#define INK_IR_ARRAY_TYPE_H

#include "ink/ir/type/builtin_type.h"

namespace ink::ir
{
  class ArrayType final : public BuiltinType
  {
    public:
      const Type &elementType() const noexcept
      {
        return ElementType;
      }

      std::uint64_t elementCount() const noexcept
      {
        return ElementCount;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::ArrayType;
      }

    private:
      ArrayType(IRContext &Context, const Type &MetaType, const Type &ElementType, std::uint64_t ElementCount) noexcept
          : BuiltinType(Context, ValueKind::ArrayType, TypeKind::Array, &MetaType),
            ElementType(ElementType),
            ElementCount(ElementCount)
      {
      }

      const Type &ElementType;
      std::uint64_t ElementCount;

      friend class TypePool;
  };
} // namespace ink::ir

#endif
