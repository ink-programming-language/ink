#ifndef INK_SEMANTIC_MODEL_INTERFACE_TYPE_H
#define INK_SEMANTIC_MODEL_INTERFACE_TYPE_H

#include "ink/semantic/model/type/user_defined_type.h"

namespace ink::semantic
{
  class InterfaceType final : public UserDefinedType
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::InterfaceType;
      }

    private:
      InterfaceType(SemanticContext &Context, const Type &MetaType, Name TypeName) noexcept
          : UserDefinedType(Context, ValueKind::InterfaceType, TypeKind::Interface, MetaType, TypeName)
      {
      }

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
