#ifndef INK_SEMANTIC_MODEL_TYPE_H
#define INK_SEMANTIC_MODEL_TYPE_H

#include "ink/semantic/model/coredefines.h"
#include "ink/semantic/model/value.h"

namespace ink::semantic
{
  class SemanticContext;
  class BuiltinType;
  class UserDefinedType;

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
        if (!ValueObject)
        {
          return false;
        }
        switch (ValueObject->kind())
        {
#define INK_SEMANTIC_TYPE_VALUE(Name) case ValueKind::Name:
#include "ink/semantic/model/Values.def"
          return true;
        default:
          return false;
        }
      }

    private:
      // Only the context's metatype supplies null and is its own value type.
      Type(SemanticContext &Context, ValueKind ValueKindValue, TypeKind Kind, const Type *MetaType) noexcept
          : Value(ValueKindValue),
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
