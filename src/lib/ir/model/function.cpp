#include "ink/ir/model/function.h"

#include <cassert>

namespace ink::ir
{
  Function::Function(const Type &ResultType) noexcept
      : ResultType(&ResultType)
  {
  }

  const Attribute *Function::attribute(AttributeKind AttributeKindValue) const noexcept
  {
    return findAttribute(Attributes, AttributeKindValue);
  }

  bool Function::hasAttribute(AttributeKind AttributeKindValue) const noexcept
  {
    return ink::ir::hasAttribute(Attributes, AttributeKindValue);
  }

  std::size_t Function::parameterCount() const noexcept
  {
    return Parameters.size();
  }

  const Parameter &Function::parameter(std::size_t ParameterIndex) const noexcept
  {
    assert(ParameterIndex < Parameters.size());
    return Parameters[ParameterIndex];
  }

  const Type *Function::parameterType(std::size_t ParameterIndex) const noexcept
  {
    return parameter(ParameterIndex).type();
  }
} // namespace ink::ir
