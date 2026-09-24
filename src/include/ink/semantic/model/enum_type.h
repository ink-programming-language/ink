#ifndef INK_SEMANTIC_MODEL_ENUM_TYPE_H
#define INK_SEMANTIC_MODEL_ENUM_TYPE_H

#include "ink/semantic/model/user_defined_type.h"

namespace ink::semantic
{
  class EnumType final : public UserDefinedType
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Enum;
      }

    private:
      EnumType(SemanticContext &Context, const Type &MetaType, const TypeDecl &Declaration) noexcept
          : UserDefinedType(Context, TypeKind::Enum, MetaType, Declaration)
      {
      }

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
