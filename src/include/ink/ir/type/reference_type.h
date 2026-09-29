#ifndef INK_IR_REFERENCE_TYPE_H
#define INK_IR_REFERENCE_TYPE_H

#include "ink/ir/type/builtin_type.h"

namespace ink::ir
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
        return ValueObject && ValueObject->kind() == ValueKind::ReferenceType;
      }

    private:
      ReferenceType(IRContext &Context, const Type &MetaType, const Type &ReferentType, AccessKind Access) noexcept
          : BuiltinType(Context, ValueKind::ReferenceType, TypeKind::Reference, &MetaType),
            ReferentType(ReferentType),
            Access(Access)
      {
      }

      const Type &ReferentType;
      AccessKind Access;

      friend class TypePool;
  };
} // namespace ink::ir

#endif
