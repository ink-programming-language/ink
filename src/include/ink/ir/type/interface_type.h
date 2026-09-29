#ifndef INK_IR_INTERFACE_TYPE_H
#define INK_IR_INTERFACE_TYPE_H

#include "ink/ir/type/user_defined_type.h"

namespace ink::ir
{
  class InterfaceType final : public UserDefinedType
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::InterfaceType;
      }

    private:
      InterfaceType(IRContext &Context, const Type &MetaType, Name TypeName) noexcept
          : UserDefinedType(Context, ValueKind::InterfaceType, TypeKind::Interface, MetaType, TypeName)
      {
      }

      friend class TypePool;
  };
} // namespace ink::ir

#endif
