#include "ink/execution/value/execution_void_value.h"

namespace ink::execution
{
  ExecutionVoidValue::ExecutionVoidValue(const ir::Type &Type) noexcept
      : ExecutionValue(ExecutionObjectKind::VoidValue, Type)
  {
  }
} // namespace ink::execution
