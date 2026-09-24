#ifndef INK_SEMANTIC_MODEL_VALUE_H
#define INK_SEMANTIC_MODEL_VALUE_H

#include "ink/semantic/model/coredefines.h"

namespace ink::semantic
{
  class Type;

  // Semantic values have stable identities within their owning context.
  // ExprValue retains a checked expression's origin; mutable execution slots
  // and values produced by individual executions are represented separately.
  class Value
  {
    public:
      virtual ~Value() = default;
      Value(const Value &) = delete;
      Value &operator=(const Value &) = delete;
      Value(Value &&) = delete;
      Value &operator=(Value &&) = delete;

      ValueKind kind() const noexcept
      {
        return Kind;
      }

      virtual const Type &type() const noexcept = 0;

    protected:
      explicit Value(ValueKind Kind) noexcept
          : Kind(Kind)
      {
      }

    private:
      ValueKind Kind;
  };
} // namespace ink::semantic

#endif
