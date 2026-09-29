#ifndef INK_IR_POINTER_TYPE_H
#define INK_IR_POINTER_TYPE_H

#include "ink/ir/type/builtin_type.h"

namespace ink::ir
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
      PointerType(IRContext &Context, const Type &MetaType, const Type &PointeeType, AccessKind Access) noexcept
          : BuiltinType(Context, ValueKind::PointerType, TypeKind::Pointer, &MetaType),
            PointeeType(PointeeType),
            Access(Access)
      {
      }

      const Type &PointeeType;
      AccessKind Access;

      friend class TypePool;
  };
} // namespace ink::ir

#endif
