#pragma once

#include "IR/Type/type.h"

#include <cstdint>

namespace ink::ir
{
  class Int16Type final : public Type
  {
    public:
      static constexpr TypeKind Kind = TypeKind::Int16;

      constexpr Int16Type() noexcept
          : Type(Kind)
      {
      }

      static constexpr bool classof(const Type *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr std::uint32_t bitWidth() const noexcept
      {
        return 16;
      }
  };
} // namespace ink::ir
