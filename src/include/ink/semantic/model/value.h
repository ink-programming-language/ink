#ifndef INK_SEMANTIC_MODEL_VALUE_H
#define INK_SEMANTIC_MODEL_VALUE_H

#include <cstdint>

namespace ink::semantic
{
  class Type;

  enum class ValueKind : std::uint8_t
  {
#define INK_SEMANTIC_VALUE(Name) Name,
#include "ink/semantic/model/Values.def"
  };

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
