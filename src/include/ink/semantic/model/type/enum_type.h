#ifndef INK_SEMANTIC_MODEL_ENUM_TYPE_H
#define INK_SEMANTIC_MODEL_ENUM_TYPE_H

#include "ink/semantic/model/type/user_defined_type.h"

namespace ink::semantic
{
  class EnumType final : public UserDefinedType
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::EnumType;
      }

    private:
      EnumType(SemanticContext &Context, const Type &MetaType, Name TypeName) noexcept
          : UserDefinedType(Context, ValueKind::EnumType, TypeKind::Enum, MetaType, TypeName)
      {
      }

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
