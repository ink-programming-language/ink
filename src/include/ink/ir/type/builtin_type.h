#ifndef INK_IR_BUILTIN_TYPE_H
#define INK_IR_BUILTIN_TYPE_H

#include "ink/ir/type/type.h"

#include <type_traits>

namespace ink::ir
{
  // Includes language-defined type constructors, even for user-defined elements.
  class BuiltinType : public Type
  {
    public:
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
    return std::is_same_v<Base, BuiltinType>;
#include "ink/ir/type/Types.def"
#undef INK_IR_TYPE
        default:
          return false;
        }
      }

    protected:
      BuiltinType(IRContext &Context, ValueKind ValueKindValue, TypeKind Kind, const Type *MetaType) noexcept
          : Type(Context, ValueKindValue, Kind, MetaType)
      {
      }

    private:
      friend class TypePool;
  };
} // namespace ink::ir

#endif
