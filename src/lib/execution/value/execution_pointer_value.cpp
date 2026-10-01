#include "ink/execution/value/execution_pointer_value.h"

#include <utility>

namespace ink::execution
{
  ExecutionPointerValue::ExecutionPointerValue(const ir::Type &Type, ExecutionPointer Value) noexcept
      : ExecutionValue(ExecutionObjectKind::PointerValue, Type),
        Value(std::move(Value))
  {
  }
} // namespace ink::execution
