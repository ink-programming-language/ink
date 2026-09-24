#ifndef INK_SEMANTIC_MODEL_SLICE_TYPE_H
#define INK_SEMANTIC_MODEL_SLICE_TYPE_H

#include "ink/semantic/model/type/builtin_type.h"

namespace ink::semantic
{
  // A view with runtime length. Ownership/allocation belongs to a container value.
  class SliceType final : public BuiltinType
  {
    public:
      const Type &elementType() const noexcept
      {
        return ElementType;
      }

      AccessKind access() const noexcept
      {
        return Access;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::SliceType;
      }

    private:
      SliceType(SemanticContext &Context, const Type &MetaType, const Type &ElementType, AccessKind Access) noexcept
          : BuiltinType(Context, ValueKind::SliceType, TypeKind::Slice, &MetaType),
            ElementType(ElementType),
            Access(Access)
      {
      }

      const Type &ElementType;
      AccessKind Access;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
