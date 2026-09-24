#ifndef INK_SEMANTIC_MODEL_INTERFACE_TYPE_H
#define INK_SEMANTIC_MODEL_INTERFACE_TYPE_H

#include "ink/semantic/model/user_defined_type.h"

namespace ink::semantic
{
  class InterfaceType final : public UserDefinedType
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Interface;
      }

    private:
      InterfaceType(SemanticContext &Context, const Type &MetaType, const TypeDecl &Declaration) noexcept
          : UserDefinedType(Context, TypeKind::Interface, MetaType, Declaration)
      {
      }

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
