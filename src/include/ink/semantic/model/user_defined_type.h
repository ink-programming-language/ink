#ifndef INK_SEMANTIC_MODEL_USER_DEFINED_TYPE_H
#define INK_SEMANTIC_MODEL_USER_DEFINED_TYPE_H

#include "ink/semantic/model/type.h"

#include <type_traits>

namespace ink::semantic
{
  class TypeDecl;

  // Nominal identity is the declaration, never just its interned name.
  class UserDefinedType : public Type
  {
    public:
      const TypeDecl &declaration() const noexcept
      {
        return Declaration;
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
#include "ink/semantic/model/Types.def"
#undef INK_SEMANTIC_TYPE
        default:
          return false;
        }
      }

    private:
      UserDefinedType(SemanticContext &Context, TypeKind Kind, const Type &MetaType, const TypeDecl &Declaration) noexcept
          : Type(Context, Kind, &MetaType),
            Declaration(Declaration)
      {
      }

      const TypeDecl &Declaration;

      friend class ClassType;
      friend class EnumType;
      friend class InterfaceType;
  };
} // namespace ink::semantic

#endif
