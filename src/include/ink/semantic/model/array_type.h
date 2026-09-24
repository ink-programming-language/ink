#ifndef INK_SEMANTIC_MODEL_ARRAY_TYPE_H
#define INK_SEMANTIC_MODEL_ARRAY_TYPE_H

#include "ink/semantic/model/builtin_type.h"

namespace ink::semantic
{
  class ArrayType final : public BuiltinType
  {
    public:
      const Type &elementType() const noexcept
      {
        return ElementType;
      }

      std::uint64_t elementCount() const noexcept
      {
        return ElementCount;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Array;
      }

    private:
      ArrayType(SemanticContext &Context, const Type &MetaType, const Type &ElementType, std::uint64_t ElementCount) noexcept
          : BuiltinType(Context, TypeKind::Array, &MetaType),
            ElementType(ElementType),
            ElementCount(ElementCount)
      {
      }

      const Type &ElementType;
      std::uint64_t ElementCount;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
