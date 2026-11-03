#pragma once

#include "ink/ir/type/type.h"

#include <cstdint>

namespace ink::ir
{
  // Element types are borrowed and must outlive the array type.
  class ArrayType final : public Type
  {
    public:
      static constexpr TypeKind Kind = TypeKind::Array;

      constexpr ArrayType(const Type &ElementType, std::uint64_t ElementCount) noexcept
          : Type(Kind),
            ElementType(ElementType),
            ElementCount(ElementCount)
      {
      }

      ArrayType(const Type &&, std::uint64_t) = delete;

      static constexpr bool classof(const Type *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr const Type &elementType() const noexcept
      {
        return ElementType;
      }

      constexpr std::uint64_t elementCount() const noexcept
      {
        return ElementCount;
      }

    private:
      const Type &ElementType;
      std::uint64_t ElementCount;
  };
} // namespace ink::ir
