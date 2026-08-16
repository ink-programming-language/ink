#include "function_executor.h"

namespace ink::execution
{
  FunctionExecutor::InstructionFlow FunctionExecutor::executeInstruction(const ir::Instruction &InstructionValue, FunctionExecutionState &State)
  {
    switch (InstructionValue.kind())
    {
    case ir::InstructionKind::Call:
      return executeCallInstruction(static_cast<const ir::CallInstruction &>(InstructionValue), State);
    case ir::InstructionKind::InsertValue:
      return executeInsertValueInstruction(static_cast<const ir::InsertValueInstruction &>(InstructionValue), State);
    case ir::InstructionKind::ExtractValue:
      return executeExtractValueInstruction(static_cast<const ir::ExtractValueInstruction &>(InstructionValue), State);
    case ir::InstructionKind::Return:
      return executeReturnInstruction(static_cast<const ir::ReturnInstruction &>(InstructionValue), State);
    }
    addFailure<core::DiagnosticKind::UnsupportedInstructionDuringExecution>(State.FunctionValue.Name, ir::instructionKindName(InstructionValue.kind()));
    return InstructionFlow::Failed;
  }
} // namespace ink::execution
