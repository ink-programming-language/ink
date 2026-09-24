#ifndef INK_SEMANTIC_MODEL_POINTER_TYPE_H
#define INK_SEMANTIC_MODEL_POINTER_TYPE_H

#include "ink/semantic/model/builtin_type.h"

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
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Pointer;
      }

    private:
      PointerType(SemanticContext &Context, const Type &MetaType, const Type &PointeeType, AccessKind Access) noexcept
          : BuiltinType(Context, TypeKind::Pointer, &MetaType),
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
