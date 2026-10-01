#ifndef INK_EXECUTION_VALUE_EXECUTION_POINTER_VALUE_H
#define INK_EXECUTION_VALUE_EXECUTION_POINTER_VALUE_H

#include "ink/execution/value/execution_value.h"

#include "ink/execution/memory/execution_pointer.h"

namespace ink::execution
{
  class ExecutionPointerValue final : public ExecutionValue
  {
    public:
      const ExecutionPointer &value() const noexcept
      {
        return Value;
      }

    private:
      ExecutionPointerValue(const ir::Type &Type, ExecutionPointer Value) noexcept;

      const ExecutionPointer Value;

      friend class ExecutionHeap;
  };
} // namespace ink::execution

#endif
