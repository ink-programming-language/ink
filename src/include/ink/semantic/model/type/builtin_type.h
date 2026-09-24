#ifndef INK_SEMANTIC_MODEL_BUILTIN_TYPE_H
#define INK_SEMANTIC_MODEL_BUILTIN_TYPE_H

#include "ink/semantic/model/type/type.h"

#include <type_traits>

namespace ink::semantic
{
  // Includes language-defined type constructors, even for user-defined elements.
  class BuiltinType : public Type
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        if (!Type::classof(ValueObject))
        {
          return false;
        }
        switch (static_cast<const Type *>(ValueObject)->typeKind())
        {
#define INK_SEMANTIC_TYPE(Name, Base) \
  case TypeKind::Name:                \
    return std::is_same_v<Base, BuiltinType>;
#include "ink/semantic/model/type/Types.def"
#undef INK_SEMANTIC_TYPE
        default:
          return false;
        }
      }

    protected:
      BuiltinType(SemanticContext &Context, ValueKind ValueKindValue, TypeKind Kind, const Type *MetaType) noexcept
          : Type(Context, ValueKindValue, Kind, MetaType)
      {
      }

    private:
      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
