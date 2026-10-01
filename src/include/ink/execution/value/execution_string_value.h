#ifndef INK_EXECUTION_VALUE_EXECUTION_STRING_VALUE_H
#define INK_EXECUTION_VALUE_EXECUTION_STRING_VALUE_H

#include "ink/execution/value/execution_value.h"

#include <string>
#include <string_view>

namespace ink::execution
{
  class ExecutionStringValue final : public ExecutionValue
  {
    public:
      std::string_view value() const noexcept
      {
        return Value;
      }

    private:
      ExecutionStringValue(const ir::Type &Type, std::string_view Value);

      const std::string Value;

      friend class ExecutionHeap;
  };
} // namespace ink::execution

#endif
