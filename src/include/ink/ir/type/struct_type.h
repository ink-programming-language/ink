#pragma once

#include "ink/ir/type/record_type.h"

namespace ink::ir
{
  class StructType final : public RecordType
  {
    public:
      static constexpr TypeKind Kind = TypeKind::Struct;

      constexpr StructType() noexcept
          : RecordType(Kind)
      {
      }

      static constexpr bool classof(const Value *Value) noexcept
      {
        return Type::classof(Value) && static_cast<const Type *>(Value)->getKind() == Kind;
      }
  };
} // namespace ink::ir
