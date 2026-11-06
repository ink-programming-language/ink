#pragma once

#include "ink/ir/type/type.h"

#include <cstddef>

namespace ink::ir
{
  // Borrowed pointee type values must remain alive during use; destruction does not access them.
  // The caller must provide a value whose type is MetaType.
  class PointerType final : public Type
  {
    public:
      std::size_t hash() const noexcept;
      static std::size_t hash(const Value *Value) noexcept;
      bool operator==(const PointerType &Other) const noexcept;
      bool operator==(const Value *Other) const noexcept;

      static constexpr TypeKind Kind = TypeKind::Pointer;

      constexpr explicit PointerType(ValueRef PointeeType) noexcept
          : Type(Kind),
            PointeeType(PointeeType.get())
      {
      }

      static constexpr bool classof(const Value *Value) noexcept
      {
        return Type::classof(Value) && static_cast<const Type *>(Value)->getKind() == Kind;
      }

      constexpr const Value &pointeeType() const noexcept
      {
        return PointeeType;
      }

    private:
      const Value &PointeeType;
  };
} // namespace ink::ir
