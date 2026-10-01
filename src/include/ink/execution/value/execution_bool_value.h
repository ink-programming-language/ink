#ifndef INK_EXECUTION_VALUE_EXECUTION_BOOL_VALUE_H
#define INK_EXECUTION_VALUE_EXECUTION_BOOL_VALUE_H

#include "ink/execution/value/execution_value.h"

namespace ink::execution
{
  class ExecutionBoolValue final : public ExecutionValue
  {
    public:
      bool value() const noexcept
      {
        return Value;
      }

    private:
      ExecutionBoolValue(const ir::Type &Type, bool Value) noexcept;

      const bool Value;

      friend class ExecutionHeap;
  };
} // namespace ink::execution

#endif
