#ifndef INK_IR_ENUM_TYPE_H
#define INK_IR_ENUM_TYPE_H

#include "ink/ir/type/user_defined_type.h"

namespace ink::ir
{
  class EnumType final : public UserDefinedType
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::EnumType;
      }

    private:
      EnumType(IRContext &Context, const Type &MetaType, Name TypeName) noexcept
          : UserDefinedType(Context, ValueKind::EnumType, TypeKind::Enum, MetaType, TypeName)
      {
      }

      friend class TypePool;
  };
} // namespace ink::ir

#endif
