#include "ink/ir/type/generic_type/generic_function_type.h"

#include <utility>

namespace ink::ir
{
  GenericFunctionType::GenericFunctionType(core::Name Name, std::vector<const Type *> GenericParameters, const Type *ReturnType, std::vector<const Type *> Parameters) noexcept
      : GenericType(ValueKind::GenericFunction, Name, std::move(GenericParameters)),
        ReturnType(ReturnType),
        Parameters(std::move(Parameters))
  {
  }

  const Type *GenericFunctionType::returnType() const noexcept
  {
    return ReturnType;
  }

  const std::vector<const Type *> &GenericFunctionType::parameters() const noexcept
  {
    return Parameters;
  }

  std::size_t GenericFunctionType::parameterCount() const noexcept
  {
    return Parameters.size();
  }

  bool GenericFunctionType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::GenericFunction;
  }
} // namespace ink::ir
