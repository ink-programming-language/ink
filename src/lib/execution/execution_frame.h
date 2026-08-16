#ifndef INK_EXECUTION_EXECUTION_FRAME_H
#define INK_EXECUTION_EXECUTION_FRAME_H

#include "ink/execution/execution_engine.h"
#include "ink/ir/id.h"

#include <cstddef>
#include <unordered_map>
#include <vector>

namespace ink::execution
{
  class ExecutionFrame
  {
  public:
    explicit ExecutionFrame(const std::vector<RuntimeValue> &Arguments);

    const RuntimeValue *find(ir::ValueId Id) const noexcept;
    bool define(ir::ValueId Id, RuntimeValue Value);

  private:
    std::unordered_map<std::size_t, RuntimeValue> Values;
  };
} // namespace ink::execution

#endif
