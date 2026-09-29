#ifndef INK_IR_CLASS_TYPE_H
#define INK_IR_CLASS_TYPE_H

#include "ink/ir/type/user_defined_type.h"

namespace ink::ir
{
  class ClassType final : public UserDefinedType
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::ClassType;
      }

    private:
      ClassType(IRContext &Context, const Type &MetaType, Name TypeName) noexcept
          : UserDefinedType(Context, ValueKind::ClassType, TypeKind::Class, MetaType, TypeName)
      {
      }

      friend class TypePool;
  };
} // namespace ink::ir

#endif
