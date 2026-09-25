#ifndef INK_SEMANTIC_MODEL_USER_DEFINED_TYPE_H
#define INK_SEMANTIC_MODEL_USER_DEFINED_TYPE_H

#include "ink/semantic/model/type/type.h"
#include "ink/semantic/model/name/name.h"

#include <type_traits>

namespace ink::semantic
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
#define INK_SEMANTIC_TYPE(Name, Base) \
  case TypeKind::Name:                \
    return std::is_same_v<Base, UserDefinedType>;
#include "ink/semantic/model/type/Types.def"
#undef INK_SEMANTIC_TYPE
        default:
          return false;
        }
      }

    protected:
      UserDefinedType(SemanticContext &Context, ValueKind ValueKindValue, TypeKind Kind, const Type &MetaType, Name TypeName) noexcept
          : Type(Context, ValueKindValue, Kind, &MetaType),
            TypeName(TypeName)
      {
      }

    private:
      Name TypeName;
  };
} // namespace ink::semantic

#endif
