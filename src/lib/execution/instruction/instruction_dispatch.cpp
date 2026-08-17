#include "engine/function_executor.h"

namespace ink::execution
{
  FunctionExecutor::InstructionFlow FunctionExecutor::executeInstruction(const ir::Instruction &InstructionValue, FunctionExecutionState &State)
  {
    switch (InstructionValue.kind())
    {
    case ir::InstructionKind::Phi:
      return InstructionFlow::Continue;
    case ir::InstructionKind::Call:
      return executeCallInstruction(static_cast<const ir::CallInstruction &>(InstructionValue), State);
    case ir::InstructionKind::Import:
      return executeImportInstruction(static_cast<const ir::ImportInstruction &>(InstructionValue), State);
    case ir::InstructionKind::Alloca:
      return executeAllocaInstruction(static_cast<const ir::AllocaInstruction &>(InstructionValue), State);
    case ir::InstructionKind::GetElementPointer:
      return executeGetElementPointerInstruction(static_cast<const ir::GetElementPointerInstruction &>(InstructionValue), State);
    case ir::InstructionKind::Load:
      return executeLoadInstruction(static_cast<const ir::LoadInstruction &>(InstructionValue), State);
    case ir::InstructionKind::Store:
      return executeStoreInstruction(static_cast<const ir::StoreInstruction &>(InstructionValue), State);
    case ir::InstructionKind::LifetimeEnd:
      return executeLifetimeEndInstruction(static_cast<const ir::LifetimeEndInstruction &>(InstructionValue), State);
    case ir::InstructionKind::SliceData:
      return executeSliceDataInstruction(static_cast<const ir::SliceDataInstruction &>(InstructionValue), State);
    case ir::InstructionKind::SliceLength:
      return executeSliceLengthInstruction(static_cast<const ir::SliceLengthInstruction &>(InstructionValue), State);
    case ir::InstructionKind::Add:
      return executeAddInstruction(static_cast<const ir::AddInstruction &>(InstructionValue), State);
    case ir::InstructionKind::Compare:
      return executeCompareInstruction(static_cast<const ir::CompareInstruction &>(InstructionValue), State);
    case ir::InstructionKind::InsertValue:
      return executeInsertValueInstruction(static_cast<const ir::InsertValueInstruction &>(InstructionValue), State);
    case ir::InstructionKind::ExtractValue:
      return executeExtractValueInstruction(static_cast<const ir::ExtractValueInstruction &>(InstructionValue), State);
    case ir::InstructionKind::Branch:
      return executeBranchInstruction(static_cast<const ir::BranchInstruction &>(InstructionValue), State);
    case ir::InstructionKind::ConditionalBranch:
      return executeConditionalBranchInstruction(static_cast<const ir::ConditionalBranchInstruction &>(InstructionValue), State);
    case ir::InstructionKind::Return:
      return executeReturnInstruction(static_cast<const ir::ReturnInstruction &>(InstructionValue), State);
    }
    addFailure<core::DiagnosticKind::UnsupportedInstructionDuringExecution>(State.FunctionValue.Name, ir::instructionKindName(InstructionValue.kind()));
    return InstructionFlow::Failed;
  }
} // namespace ink::execution
