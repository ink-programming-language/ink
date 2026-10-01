#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/function/function.h"

namespace ink::execution
{
  ExecutionValueResult ExecutionEngine::makeFunctionValue(const ir::Function &Function)
  {
    return {ExecutionStatus::Success, Heap.function(Function)};
  }
} // namespace ink::execution
