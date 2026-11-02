#pragma once

#include "ink/ir/Type/type.h"

namespace ink::ir
{
  class ClassType final : public Type
  {
    public:
      static constexpr TypeKind Kind = TypeKind::Class;

      constexpr ClassType() noexcept
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
