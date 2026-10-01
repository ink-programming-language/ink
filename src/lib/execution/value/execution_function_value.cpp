#include "ink/execution/value/execution_function_value.h"

#include "ink/ir/function/function.h"

namespace ink::execution
{
  ExecutionFunctionValue::ExecutionFunctionValue(const ir::Function &Value) noexcept
      : ExecutionValue(ExecutionObjectKind::FunctionValue, Value.type()),
        Value(Value)
  {
  }
} // namespace ink::execution
