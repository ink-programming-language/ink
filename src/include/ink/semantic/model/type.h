#ifndef INK_SEMANTIC_MODEL_TYPE_H
#define INK_SEMANTIC_MODEL_TYPE_H

#include "ink/semantic/model/value.h"

namespace ink::semantic
{
  class SemanticContext;
  class BuiltinType;
  class UserDefinedType;

  enum class TypeKind : std::uint8_t
  {
#define INK_SEMANTIC_TYPE(Name, Base) Name,
#include "ink/semantic/model/Types.def"
#undef INK_SEMANTIC_TYPE
  };

  // Access through a pointer/reference/slice, independent of binding mutability.
  enum class AccessKind : std::uint8_t
  {
    ReadOnly,
    ReadWrite,
  };

  class Type : public Value
  {
    public:
      TypeKind typeKind() const noexcept
      {
        return Kind;
      }

      const Type &type() const noexcept final
      {
        return *MetaType;
      }

      const SemanticContext &context() const noexcept
      {
        return Context;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::Type;
      }

    private:
      // Only the context's metatype supplies null and is its own value type.
      Type(SemanticContext &Context, TypeKind Kind, const Type *MetaType) noexcept
          : Value(ValueKind::Type),
            Context(Context),
            Kind(Kind),
            MetaType(MetaType ? MetaType : this)
      {
      }

      const SemanticContext &Context;
      TypeKind Kind;
      const Type *MetaType;

      friend class BuiltinType;
      friend class UserDefinedType;
  };
} // namespace ink::semantic

#endif
