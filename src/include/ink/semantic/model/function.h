#ifndef INK_SEMANTIC_MODEL_FUNCTION_H
#define INK_SEMANTIC_MODEL_FUNCTION_H

#include "ink/semantic/model/name.h"
#include "ink/semantic/model/type/function_type.h"

namespace ink::semantic
{
  // A closed callable identity. Generic definitions must be instantiated before creating this value.
  class Function final : public Value
  {
    public:
      Name name() const noexcept
      {
        return FunctionName;
      }

      const FunctionType &type() const noexcept override
      {
        return Signature;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::Function;
      }

    private:
      Function(Name FunctionName, const FunctionType &Signature) noexcept
          : Value(ValueKind::Function),
            FunctionName(FunctionName),
            Signature(Signature)
      {
      }

      Name FunctionName;
      const FunctionType &Signature;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
