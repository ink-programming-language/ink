#ifndef INK_IR_FUNCTION_PARAMETER_H
#define INK_IR_FUNCTION_PARAMETER_H

#include "ink/ir/type/type.h"
#include "ink/ir/name/name.h"

#include <cstddef>

namespace ink::ir
{
  class Function;

  // An incoming argument value, identified by its outer function and zero-based index.
  // ParameterKind records binding metadata; runtime call operands remain normalized slots.
  class FunctionParameter final : public Value
  {
    public:
      // Invalid for unnamed parameters synthesized directly through the model API.
      Name name() const noexcept
      {
        return ParameterName;
      }

      const Function &function() const noexcept;

      std::size_t index() const noexcept
      {
        return Index;
      }

      ParameterKind parameterKind() const noexcept
      {
        return Kind;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::FunctionParameter;
      }

    private:
      FunctionParameter(Name ParameterName, const Type &ValueType, std::size_t Index, ParameterKind Kind) noexcept
          : Value(ValueType.context(), ValueKind::FunctionParameter, ValueType),
            ParameterName(ParameterName),
            Index(Index),
            Kind(Kind)
      {
      }

      Name ParameterName;
      std::size_t Index;
      ParameterKind Kind;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
