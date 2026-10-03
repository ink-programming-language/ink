#ifndef INK_EXECUTION_VALUE_EXECUTION_ARRAY_VALUE_H
#define INK_EXECUTION_VALUE_EXECUTION_ARRAY_VALUE_H

#include "ink/execution/value/execution_value.h"

#include <utility>
#include <vector>

namespace ink::execution
{
  class ExecutionArrayValue final : public ExecutionValue
  {
    public:
      std::span<const ExecutionValueRef> value() const noexcept
      {
        return Elements;
      }

    private:
      ExecutionArrayValue(const ir::Type &Type, std::vector<ExecutionValueRef> Elements)
          : ExecutionValue(ExecutionObjectKind::ArrayValue, Type),
            Elements(std::move(Elements))
      {
      }

      const std::vector<ExecutionValueRef> Elements;

      friend class ExecutionHeap;
  };
} // namespace ink::execution

#endif
