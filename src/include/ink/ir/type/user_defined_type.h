#ifndef INK_IR_USER_DEFINED_TYPE_H
#define INK_IR_USER_DEFINED_TYPE_H

#include "ink/ir/type/type.h"
#include "ink/ir/name/name.h"

#include <type_traits>

namespace ink::ir
{
  // Each resolved nominal type has its own identity; names do not merge instances.
  class UserDefinedType : public Type
  {
    public:
      Name name() const noexcept
      {
        return TypeName;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        if (!Type::classof(ValueObject))
        {
          return false;
        }
        switch (static_cast<const Type *>(ValueObject)->typeKind())
        {
#define INK_IR_TYPE(Name, Base) \
  case TypeKind::Name:          \
    return std::is_same_v<Base, UserDefinedType>;
#include "ink/ir/type/Types.def"
#undef INK_IR_TYPE
        default:
          return false;
        }
      }

    protected:
      UserDefinedType(IRContext &Context, ValueKind ValueKindValue, TypeKind Kind, const Type &MetaType, Name TypeName) noexcept
          : Type(Context, ValueKindValue, Kind, &MetaType),
            TypeName(TypeName)
      {
      }

    private:
      Name TypeName;
  };
} // namespace ink::ir

#endif
