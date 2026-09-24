#ifndef INK_SEMANTIC_MODEL_FUNCTION_TYPE_H
#define INK_SEMANTIC_MODEL_FUNCTION_TYPE_H

#include "ink/semantic/model/type/builtin_type.h"

#include <span>
#include <vector>

namespace ink::semantic
{
  // A resolved, fixed-arity function signature. Names, defaults and generic bindings belong to analysis.
  class FunctionType final : public BuiltinType
  {
    public:
      const Type &returnType() const noexcept
      {
        return ReturnType;
      }

      std::span<const Type *const> parameterTypes() const noexcept
      {
        return ParameterTypes;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::FunctionType;
      }

    private:
      FunctionType(SemanticContext &Context, const Type &MetaType, const Type &ReturnType, std::span<const Type *const> ParameterTypes)
          : BuiltinType(Context, ValueKind::FunctionType, TypeKind::Function, &MetaType),
            ReturnType(ReturnType),
            ParameterTypes(ParameterTypes.begin(), ParameterTypes.end())
      {
      }

      const Type &ReturnType;
      std::vector<const Type *> ParameterTypes;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
