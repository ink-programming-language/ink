#ifndef INK_EXECUTION_FUNCTION_EXECUTOR_H
#define INK_EXECUTION_FUNCTION_EXECUTOR_H

#include "execution_frame.h"
#include "ink/core/diagnostic.h"
#include "ink/execution/context.h"
#include "ink/execution/runtime_value.h"
#include "ink/ir/arithmetic.h"
#include "ink/ir/control_flow.h"
#include "ink/ir/instruction.h"
#include "ink/ir/memory.h"
#include "ink/ir/module.h"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ink::execution
{
  class ExternalFunctionInvoker
  {
  public:
    virtual ~ExternalFunctionInvoker() = default;
    virtual bool invokeExternal(std::size_t FunctionIndex, const std::vector<RuntimeValueRef> &Arguments, RuntimeValueArena &Values, RuntimeValueRef &Result, std::vector<core::Diagnostic> &Diagnostics) = 0;
  };

  class FunctionExecutor
  {
  public:
    FunctionExecutor(ExecutionContext &Context, const ir::Module &ModuleValue, ExternalFunctionInvoker &ExternalInvoker, std::vector<core::Diagnostic> &Diagnostics);

    bool execute(std::size_t FunctionIndex, const std::vector<RuntimeValueRef> &Arguments, RuntimeValueRef &Result);

  private:
    enum class InstructionFlow
    {
      Continue,
      Branch,
      Return,
      Failed,
    };

    struct FunctionExecutionState
    {
      FunctionExecutionState(const ir::Function &FunctionValue, const std::vector<RuntimeValueRef> &Arguments, std::size_t Depth, std::size_t FrameId) : FunctionValue(FunctionValue), Frame(Arguments), Depth(Depth), FrameId(FrameId)
      {
      }

      const ir::Function &FunctionValue;
      ExecutionFrame Frame;
      std::size_t Depth;
      std::size_t FrameId;
      ir::BlockId NextBlock;
      ir::BlockId PreviousBlock;
      RuntimeValueRef ReturnValue = nullptr;
    };

    template <core::DiagnosticKind Kind, typename... ArgumentTypes>
    void addFailure(ArgumentTypes &&...Arguments)
    {
      core::Diagnostic DiagnosticEntry = core::makeDiagnostic<Kind>({}, std::forward<ArgumentTypes>(Arguments)...);
      Context.diagnosticEngine().report(DiagnosticEntry);
      Diagnostics.push_back(std::move(DiagnosticEntry));
    }

    RuntimeValueRef importValue(RuntimeValueRef Value);
    RuntimeValueRef zeroValue(const ir::Type &TypeValue);
    RuntimeValueRef evaluateValue(const ir::Value &Value, const ExecutionFrame &Frame, const std::string &FunctionName);
    InstructionFlow executeInstruction(const ir::Instruction &InstructionValue, FunctionExecutionState &State);
    InstructionFlow executeCallInstruction(const ir::CallInstruction &Call, FunctionExecutionState &State);
    InstructionFlow executeAllocaInstruction(const ir::AllocaInstruction &Alloca, FunctionExecutionState &State);
    InstructionFlow executeGetElementPointerInstruction(const ir::GetElementPointerInstruction &GetElementPointer, FunctionExecutionState &State);
    InstructionFlow executeLoadInstruction(const ir::LoadInstruction &Load, FunctionExecutionState &State);
    InstructionFlow executeStoreInstruction(const ir::StoreInstruction &Store, FunctionExecutionState &State);
    InstructionFlow executeLifetimeEndInstruction(const ir::LifetimeEndInstruction &LifetimeEnd, FunctionExecutionState &State);
    InstructionFlow executeSliceDataInstruction(const ir::SliceDataInstruction &SliceData, FunctionExecutionState &State);
    InstructionFlow executeSliceLengthInstruction(const ir::SliceLengthInstruction &SliceLength, FunctionExecutionState &State);
    InstructionFlow executeAddInstruction(const ir::AddInstruction &Add, FunctionExecutionState &State);
    InstructionFlow executeCompareInstruction(const ir::CompareInstruction &Compare, FunctionExecutionState &State);
    InstructionFlow executeInsertValueInstruction(const ir::InsertValueInstruction &Insert, FunctionExecutionState &State);
    InstructionFlow executeExtractValueInstruction(const ir::ExtractValueInstruction &Extract, FunctionExecutionState &State);
    InstructionFlow executeBranchInstruction(const ir::BranchInstruction &Branch, FunctionExecutionState &State);
    InstructionFlow executeConditionalBranchInstruction(const ir::ConditionalBranchInstruction &Branch, FunctionExecutionState &State);
    InstructionFlow executeReturnInstruction(const ir::ReturnInstruction &Return, FunctionExecutionState &State);
    bool selectBlockTarget(const ir::BlockTarget &Target, FunctionExecutionState &State);
    bool enterBlock(FunctionExecutionState &State);
    bool consumeInstructionStep(const ir::Function &FunctionValue);
    bool executeFunction(std::size_t FunctionIndex, const std::vector<RuntimeValueRef> &Arguments, std::size_t Depth, RuntimeValueRef &Result);

    ExecutionContext &Context;
    const ir::Module &ModuleValue;
    ExternalFunctionInvoker &ExternalInvoker;
    std::vector<core::Diagnostic> &Diagnostics;
    RuntimeValueArena Values;
    std::unordered_map<RuntimeValueRef, RuntimeValueRef> ImportedValues;
    std::unordered_map<std::size_t, RuntimeValueRef> GlobalPointers;
    std::size_t NextFrameId = 0;
    std::size_t ExecutedInstructionCount = 0;
  };
} // namespace ink::execution

#endif
