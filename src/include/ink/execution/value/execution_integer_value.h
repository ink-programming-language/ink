#ifndef INK_EXECUTION_VALUE_EXECUTION_INTEGER_VALUE_H
#define INK_EXECUTION_VALUE_EXECUTION_INTEGER_VALUE_H

#include "ink/execution/value/execution_value.h"

#include "ink/execution/value/execution_integer.h"

namespace ink::execution
{
  class ExecutionIntegerValue final : public ExecutionValue
  {
    public:
      const ExecutionInteger &value() const noexcept
      {
        return Value;
      }

    private:
      ExecutionIntegerValue(const ir::Type &Type, ExecutionInteger Value);

      const ExecutionInteger Value;

      friend class ExecutionHeap;
  };
} // namespace ink::execution

#endif
