#ifndef INK_SEMANTIC_MODEL_CLASS_TYPE_H
#define INK_SEMANTIC_MODEL_CLASS_TYPE_H

#include "ink/semantic/model/type/user_defined_type.h"

namespace ink::semantic
{
  class ClassType final : public UserDefinedType
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::ClassType;
      }

    private:
      ClassType(SemanticContext &Context, const Type &MetaType, Name TypeName) noexcept
          : UserDefinedType(Context, ValueKind::ClassType, TypeKind::Class, MetaType, TypeName)
      {
      }

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
