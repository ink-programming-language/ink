#ifndef INK_SEMANTIC_MODEL_SLICE_TYPE_H
#define INK_SEMANTIC_MODEL_SLICE_TYPE_H

#include "ink/semantic/model/builtin_type.h"

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
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Slice;
      }

    private:
      SliceType(SemanticContext &Context, const Type &MetaType, const Type &ElementType, AccessKind Access) noexcept
          : BuiltinType(Context, TypeKind::Slice, &MetaType),
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
