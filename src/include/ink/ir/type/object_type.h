#pragma once

#include "ink/ir/type/type.h"

namespace ink::ir
{
  class ObjectType final : public Type
  {
    public:
      static constexpr TypeKind Kind = TypeKind::Object;

      constexpr ObjectType() noexcept
          : Type(Kind)
      {
      }

      static constexpr bool classof(const Type *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      constexpr TypeSemantics getSemantics() const noexcept override
      {
        return TypeSemantics::Reference;
      }
  };
} // namespace ink::ir
