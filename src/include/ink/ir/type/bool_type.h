#pragma once

#include "ink/ir/type/type.h"

#include <cstdint>

namespace ink::ir
{
  class BoolType final : public Type
  {
    public:
      static constexpr TypeKind Kind = TypeKind::Bool;

      constexpr BoolType() noexcept
          : Type(Kind)
      {
      }

      static constexpr bool classof(const Type *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr std::uint32_t bitWidth() const noexcept
      {
        return 1;
      }
  };
} // namespace ink::ir
