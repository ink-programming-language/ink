#include "ink/execution/engine/execution_engine.h"

#include "ink/ir/instruction/branch_instruction.h"

namespace ink::execution
{
  ExecutionInstructionResult ExecutionEngine::executeBranch(const ir::BranchInstruction &Branch)
  {
    return ExecutionInstructionResult::jumpTo(Branch.target());
  }
} // namespace ink::execution
