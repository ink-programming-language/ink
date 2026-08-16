#ifndef INK_EXECUTION_EXECUTION_FRAME_H
#define INK_EXECUTION_EXECUTION_FRAME_H

#include "ink/execution/runtime_value.h"
#include "ink/ir/id.h"

#include <cstddef>
#include <vector>

namespace ink::execution
{
  class ExecutionFrame
  {
  public:
    explicit ExecutionFrame(const std::vector<RuntimeValueRef> &Arguments);

    RuntimeValueRef find(ir::ValueId Id) const noexcept;
    bool define(ir::ValueId Id, RuntimeValueRef Value);

  private:
    std::vector<RuntimeValueRef> Values;
  };
} // namespace ink::execution

#endif
