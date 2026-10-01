#include "ink/execution/value/execution_integer_value.h"

#include <utility>

namespace ink::execution
{
  ExecutionIntegerValue::ExecutionIntegerValue(const ir::Type &Type, ExecutionInteger Value)
      : ExecutionValue(ExecutionObjectKind::IntegerValue, Type),
        Value(std::move(Value))
  {
  }
} // namespace ink::execution
