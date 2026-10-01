#include "ink/execution/value/execution_string_value.h"

namespace ink::execution
{
  ExecutionStringValue::ExecutionStringValue(const ir::Type &Type, std::string_view Value)
      : ExecutionValue(ExecutionObjectKind::StringValue, Type),
        Value(Value)
  {
  }
} // namespace ink::execution
