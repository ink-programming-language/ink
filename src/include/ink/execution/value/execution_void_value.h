#ifndef INK_EXECUTION_VALUE_EXECUTION_VOID_VALUE_H
#define INK_EXECUTION_VALUE_EXECUTION_VOID_VALUE_H

#include "ink/execution/value/execution_value.h"

namespace ink::execution
{
  class ExecutionVoidValue final : public ExecutionValue
  {
    private:
      explicit ExecutionVoidValue(const ir::Type &Type) noexcept;

      friend class ExecutionHeap;
  };
} // namespace ink::execution

#endif
