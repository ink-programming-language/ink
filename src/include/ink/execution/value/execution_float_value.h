#ifndef INK_EXECUTION_VALUE_EXECUTION_FLOAT_VALUE_H
#define INK_EXECUTION_VALUE_EXECUTION_FLOAT_VALUE_H

#include "ink/execution/value/execution_value.h"

#include "ink/ir/constant/float_constant.h"

namespace ink::execution
{
  class ExecutionFloatValue final : public ExecutionValue
  {
    public:
      ir::FloatBits value() const noexcept
      {
        return Value;
      }

    private:
      ExecutionFloatValue(const ir::Type &Type, ir::FloatBits Value) noexcept;

      const ir::FloatBits Value;

      friend class ExecutionHeap;
  };
} // namespace ink::execution

#endif
