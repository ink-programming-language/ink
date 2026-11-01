#pragma once

#include "IR/Type/type.h"

#include <cstdint>

namespace ink::ir
{
  class Float32Type final : public Type
  {
    public:
      static constexpr TypeKind Kind = TypeKind::Float32;

      constexpr Float32Type() noexcept
          : Type(Kind)
      {
      }

      static constexpr bool classof(const Type *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr std::uint32_t bitWidth() const noexcept
      {
        return 32;
      }
  };
} // namespace ink::ir
