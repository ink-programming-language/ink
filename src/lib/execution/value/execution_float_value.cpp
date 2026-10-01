#include "ink/execution/value/execution_float_value.h"

namespace ink::execution
{
  ExecutionFloatValue::ExecutionFloatValue(const ir::Type &Type, ir::FloatBits Value) noexcept
      : ExecutionValue(ExecutionObjectKind::FloatValue, Type),
        Value(Value)
  {
  }
} // namespace ink::execution
