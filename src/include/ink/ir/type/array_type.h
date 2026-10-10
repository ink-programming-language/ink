#ifndef INK_IR_TYPE_ARRAY_TYPE_H
#define INK_IR_TYPE_ARRAY_TYPE_H

#include "ink/ir/type/type.h"

#include <memory>

namespace ink::ir
{
  class ArrayType final : public Type
  {
    public:
      // ElementType is borrowed and must outlive the array. Null, invalid layout or size overflow returns nullptr.
      static std::unique_ptr<ArrayType> create(const Type *ElementType, std::uint64_t ElementCount);

      const Type *elementType() const noexcept;
      std::uint64_t elementCount() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      ArrayType(const Type *ElementType, std::uint64_t ElementCount, std::uint64_t Size, std::uint64_t Alignment) noexcept;

      const Type *ElementType = nullptr;
      std::uint64_t ElementCount = 0;
      std::uint64_t Size = 0;
      std::uint64_t Alignment = 0;
  };
} // namespace ink::ir

#endif
