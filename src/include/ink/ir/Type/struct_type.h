#pragma once

#include "ink/ir/Type/type.h"

namespace ink::ir
{
  class StructType final : public Type
  {
    public:
      static constexpr TypeKind Kind = TypeKind::Struct;

      constexpr StructType() noexcept
          : Type(Kind)
      {
      }

      static constexpr bool classof(const Type *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }
  };
} // namespace ink::ir
