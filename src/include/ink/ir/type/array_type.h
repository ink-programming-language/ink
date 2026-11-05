#pragma once

#include "ink/ir/type/type.h"

#include <cstddef>
#include <cstdint>

namespace ink::ir
{
  // Element type values are borrowed and must outlive the array type.
  // The caller must provide a value whose type is MetaType.
  class ArrayType final : public Type
  {
    public:
      struct Key
      {
          const Value *ElementType;
          std::uint64_t ElementCount;

          std::size_t hash() const noexcept;
          constexpr bool operator==(const Key &) const noexcept = default;
      };

      std::size_t hash() const noexcept;
      bool operator==(const ArrayType &Other) const noexcept;
      bool operator==(const Key &Other) const noexcept;

      static constexpr TypeKind Kind = TypeKind::Array;

      constexpr ArrayType(ValueRef ElementType, std::uint64_t ElementCount) noexcept
          : Type(Kind),
            ElementType(ElementType.get()),
            ElementCount(ElementCount)
      {
      }

      static constexpr bool classof(const Value *Value) noexcept
      {
        return Type::classof(Value) && static_cast<const Type *>(Value)->getKind() == Kind;
      }

      constexpr const Value &elementType() const noexcept
      {
        return ElementType;
      }

      constexpr std::uint64_t elementCount() const noexcept
      {
        return ElementCount;
      }

    private:
      const Value &ElementType;
      std::uint64_t ElementCount;
  };
} // namespace ink::ir
