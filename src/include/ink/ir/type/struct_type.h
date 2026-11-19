#pragma once

#include "ink/ir/type/record_type.h"

namespace ink::ir
{
    class StructType final : public RecordType
    {
        public:
            static constexpr TypeKind Kind = TypeKind::Struct;

            constexpr explicit StructType(core::Name Name = core::NAME_None) noexcept
                : RecordType(Kind, Name)
            {
            }

            static constexpr bool classof(const Value* Value) noexcept
            {
                return Type::classof(Value) && static_cast<const Type*>(Value)->getKind() == Kind;
            }
    };
} // namespace ink::ir
