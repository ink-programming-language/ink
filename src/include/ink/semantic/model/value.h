#ifndef INK_SEMANTIC_MODEL_VALUE_H
#define INK_SEMANTIC_MODEL_VALUE_H

#include "ink/semantic/model/coredefines.h"

namespace ink::semantic
{
  class SemanticContext;
  class Type;

  // Semantic values have stable identities within their owning context.
  // Each value's type is fixed at construction and owned by the same context.
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

      const SemanticContext &context() const noexcept
      {
        return Context;
      }

      // Structural parent; storage lifetime is still managed by SemanticContext.
      Value *outer() noexcept
      {
        return Outer;
      }

      const Value *outer() const noexcept
      {
        return Outer;
      }

      const Type &type() const noexcept
      {
        return ValueType;
      }

    protected:
      Value(const SemanticContext &Context, ValueKind Kind, const Type &ValueType) noexcept
          : Context(Context),
            Kind(Kind),
            ValueType(ValueType)
      {
      }

    private:
      const SemanticContext &Context;
      ValueKind Kind;
      const Type &ValueType;
      Value *Outer = nullptr;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
