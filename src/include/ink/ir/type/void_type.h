#pragma once

#include "ink/ir/type/type.h"

namespace ink::ir
{
  class VoidType final : public Type
  {
    public:
      static constexpr TypeKind Kind = TypeKind::Void;

      constexpr VoidType() noexcept
          : Type(Kind)
      {
      }

      static constexpr bool classof(const Type *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }
  };
} // namespace ink::ir
