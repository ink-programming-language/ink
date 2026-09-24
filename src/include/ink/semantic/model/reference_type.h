#ifndef INK_SEMANTIC_MODEL_REFERENCE_TYPE_H
#define INK_SEMANTIC_MODEL_REFERENCE_TYPE_H

#include "ink/semantic/model/builtin_type.h"

namespace ink::semantic
{
  // A reference type is distinct from an expression's place/value category.
  class ReferenceType final : public BuiltinType
  {
    public:
      const Type &referentType() const noexcept
      {
        return ReferentType;
      }

      AccessKind access() const noexcept
      {
        return Access;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Reference;
      }

    private:
      ReferenceType(SemanticContext &Context, const Type &MetaType, const Type &ReferentType, AccessKind Access) noexcept
          : BuiltinType(Context, TypeKind::Reference, &MetaType),
            ReferentType(ReferentType),
            Access(Access)
      {
      }

      const Type &ReferentType;
      AccessKind Access;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
