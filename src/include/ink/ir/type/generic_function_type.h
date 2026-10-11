#ifndef INK_IR_TYPE_GENERIC_FUNCTION_TYPE_H
#define INK_IR_TYPE_GENERIC_FUNCTION_TYPE_H

#include "ink/ir/type/generic_type.h"

namespace ink::ir
{
  class TypeContext;

  // Describes a function template's generic parameters and ordinary callable signature.
  class GenericFunctionType final : public GenericType
  {
    public:
      const Type *returnType() const noexcept;
      const std::vector<Parameter> &parameters() const noexcept;
      std::size_t parameterCount() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypeContext;

      // Owns both parameter lists and borrows their types and ReturnType.
      // Null types are preserved for signatures awaiting semantic resolution.
      GenericFunctionType(core::Name Name, std::vector<Parameter> GenericParameters, const Type *ReturnType, std::vector<Parameter> Parameters) noexcept;

      const Type *ReturnType = nullptr;
      std::vector<Parameter> Parameters;
  };
} // namespace ink::ir

#endif
