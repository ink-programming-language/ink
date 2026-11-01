#pragma once

#include "IR/Type/type.h"

#include <cstdint>

namespace ink::ir
{
  class Int32Type final : public Type
  {
    public:
      static constexpr TypeKind Kind = TypeKind::Int32;

      constexpr Int32Type() noexcept
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
