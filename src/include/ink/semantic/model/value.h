#ifndef INK_SEMANTIC_MODEL_VALUE_H
#define INK_SEMANTIC_MODEL_VALUE_H

#include "ink/semantic/model/coredefines.h"

namespace ink::semantic
{
  class SemanticContext;
  class Type;

  // Semantic values keep their addresses while alive, including across ownership transfers.
  // Context owns shared types/constants and must outlive every detached or attached value that refers to it.
  class Value
  {
    public:
      virtual ~Value();
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

      // Non-owning parent link. A structural parent owns this value; operands and types remain borrowed.
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

      friend class IRBuilder;
  };
} // namespace ink::semantic

#endif
