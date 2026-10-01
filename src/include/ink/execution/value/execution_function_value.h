#ifndef INK_EXECUTION_VALUE_EXECUTION_FUNCTION_VALUE_H
#define INK_EXECUTION_VALUE_EXECUTION_FUNCTION_VALUE_H

#include "ink/execution/value/execution_value.h"

namespace ink::execution
{
  class ExecutionFunctionValue final : public ExecutionValue
  {
    public:
      const ir::Function &value() const noexcept
      {
        return Value;
      }

    private:
      explicit ExecutionFunctionValue(const ir::Function &Value) noexcept;

      const ir::Function &Value;

      friend class ExecutionHeap;
  };
} // namespace ink::execution

#endif
