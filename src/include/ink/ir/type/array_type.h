#ifndef INK_IR_TYPE_ARRAY_TYPE_H
#define INK_IR_TYPE_ARRAY_TYPE_H

#include "ink/ir/type/type.h"

namespace ink::ir
{
  class TypeContext;

  class ArrayType final : public Type
  {
    public:
      const Type *elementType() const noexcept;
      std::uint64_t elementCount() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypeContext;

      ArrayType(const Type *ElementType, std::uint64_t ElementCount, std::uint64_t Size, std::uint64_t Alignment) noexcept;

      const Type *ElementType = nullptr;
      std::uint64_t ElementCount = 0;
      std::uint64_t Size = 0;
      std::uint64_t Alignment = 0;
  };
} // namespace ink::ir

#endif
