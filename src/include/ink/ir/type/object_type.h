#pragma once

#include "ink/ir/type/record_type.h"

namespace ink::ir
{
  class ObjectType final : public RecordType
  {
    public:
      static constexpr TypeKind Kind = TypeKind::Object;

      constexpr ObjectType() noexcept
          : RecordType(Kind)
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
