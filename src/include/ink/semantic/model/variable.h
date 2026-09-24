#ifndef INK_SEMANTIC_MODEL_VARIABLE_H
#define INK_SEMANTIC_MODEL_VARIABLE_H

#include "ink/semantic/model/name.h"

namespace ink::semantic
{
  class SemanticContext;
  class Type;
  class Value;

  // Binding mutability is independent of access through a value and compile-time evaluation.
  enum class BindingMutability : std::uint8_t
  {
    Mutable,
    Immutable,
  };

  // An analyzed variable binding, not a generic Decl or an execution-time storage slot.
  class Variable final
  {
    public:
      Variable(const Variable &) = delete;
      Variable &operator=(const Variable &) = delete;
      Variable(Variable &&) = delete;
      Variable &operator=(Variable &&) = delete;

      Name name() const noexcept
      {
        return VariableName;
      }

      const SemanticContext &context() const noexcept
      {
        return Context;
      }

      const Type *type() const noexcept
      {
        return ValueType;
      }

      // The type and initializer may each be published once; both must agree when present.
      bool setType(const Type &ResolvedType) noexcept;

      BindingMutability mutability() const noexcept
      {
        return Mutability;
      }

      bool isMutable() const noexcept
      {
        return Mutability == BindingMutability::Mutable;
      }

      const Value *initializer() const noexcept
      {
        return Initializer;
      }

      bool setInitializer(const Value &InitializerValue) noexcept;

    private:
      Variable(SemanticContext &Context, Name VariableName, BindingMutability Mutability, const Value *Initializer) noexcept
          : Context(Context),
            VariableName(VariableName),
            Mutability(Mutability),
            Initializer(Initializer)
      {
      }

      const SemanticContext &Context;
      Name VariableName;
      const Type *ValueType = nullptr;
      BindingMutability Mutability;
      const Value *Initializer;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
