#ifndef INK_IR_TYPE_STRUCTURE_TYPE_FUNCTION_TYPE_PARAMETER_H
#define INK_IR_TYPE_STRUCTURE_TYPE_FUNCTION_TYPE_PARAMETER_H

namespace ink::ir
{
  class Type;

  // A copyable signature descriptor that borrows its type and has no value identity or name.
  struct FunctionTypeParameter
  {
    public:
      constexpr FunctionTypeParameter(const Type *ParameterType, bool Variadic = false) noexcept
          : ParameterType(ParameterType),
            Variadic(Variadic)
      {
      }

      const Type *ParameterType = nullptr;
      bool Variadic = false;
  };
} // namespace ink::ir

#endif
