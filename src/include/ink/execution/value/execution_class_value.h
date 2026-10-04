#ifndef INK_EXECUTION_VALUE_EXECUTION_CLASS_VALUE_H
#define INK_EXECUTION_VALUE_EXECUTION_CLASS_VALUE_H

#include "ink/execution/value/execution_value.h"

#include <utility>
#include <vector>

namespace ink::execution
{
  class ExecutionClassValue final : public ExecutionValue
  {
    public:
      std::span<const ExecutionValueRef> value() const noexcept
      {
        return Fields;
      }

    private:
      ExecutionClassValue(const ir::Type &Type, std::vector<ExecutionValueRef> Fields)
          : ExecutionValue(ExecutionObjectKind::ClassValue, Type),
            Fields(std::move(Fields))
      {
      }

      const std::vector<ExecutionValueRef> Fields;

      friend class ExecutionHeap;
  };
} // namespace ink::execution

#endif
