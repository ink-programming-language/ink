#include "ink/execution/value/execution_bool_value.h"

namespace ink::execution
{
  ExecutionBoolValue::ExecutionBoolValue(const ir::Type &Type, bool Value) noexcept
      : ExecutionValue(ExecutionObjectKind::BooleanValue, Type),
        Value(Value)
  {
  }
} // namespace ink::execution
