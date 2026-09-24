#ifndef INK_SEMANTIC_MODEL_POINTER_TYPE_H
#define INK_SEMANTIC_MODEL_POINTER_TYPE_H

#include "ink/semantic/model/type/builtin_type.h"

namespace ink::semantic
{
  class PointerType final : public BuiltinType
  {
    public:
      const Type &pointeeType() const noexcept
      {
        return PointeeType;
      }

      AccessKind access() const noexcept
      {
        return Access;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::PointerType;
      }

    private:
      PointerType(SemanticContext &Context, const Type &MetaType, const Type &PointeeType, AccessKind Access) noexcept
          : BuiltinType(Context, ValueKind::PointerType, TypeKind::Pointer, &MetaType),
            PointeeType(PointeeType),
            Access(Access)
      {
      }

      const Type &PointeeType;
      AccessKind Access;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
