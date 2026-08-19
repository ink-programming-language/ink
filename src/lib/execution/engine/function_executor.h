#ifndef INK_EXECUTION_FUNCTION_EXECUTOR_H
#define INK_EXECUTION_FUNCTION_EXECUTOR_H

#include "engine/execution_frame.h"
#include "ink/core/diagnostic.h"
#include "ink/execution/context.h"
#include "ink/execution/module/module_instance.h"
#include "ink/execution/runtime/runtime_value.h"
#include "ink/ir/instruction/arithmetic.h"
#include "ink/ir/instruction/control_flow.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/instruction/memory.h"
#include "ink/ir/model/module.h"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ink::ir
{
  class GlobalVariableAddressOperand;
}

namespace ink::execution
{
  class ExternalFunctionInvoker
  {
    public:
      virtual ~ExternalFunctionInvoker() = default;
      virtual bool invokeExternal(std::size_t FunctionIndex, const std::vector<RuntimeValueRef> &Arguments, RuntimeValueArena &Values, RuntimeValueRef &Result) = 0;
  };

  class ModuleExecutionRuntime
  {
    public:
      virtual ~ModuleExecutionRuntime() = default;
      virtual bool importModule(ModuleInstance &Importer, const ir::Name &Target) = 0;
      virtual ModuleInstance *resolveReferencedModule(ModuleInstance &Importer, ModuleId Target) = 0;
      virtual bool resolveImportedFunction(ModuleInstance &Importer, ir::FunctionId Import, ModuleInstance *&TargetModule, ir::FunctionId &TargetFunction) = 0;
      virtual bool resolveImportedGlobal(ModuleInstance &Importer, ir::GlobalId Import, ModuleInstance *&TargetModule, ir::GlobalId &TargetGlobal) = 0;
      virtual ExternalFunctionInvoker *externalInvoker(ModuleInstance &Module) noexcept = 0;
  };

  class FunctionExecutor
  {
    public:
      FunctionExecutor(ExecutionContext &Context, ModuleExecutionRuntime &Runtime, ModuleInstance &EntryModule);

      bool execute(ir::FunctionId Function, const std::vector<RuntimeValueRef> &Arguments, RuntimeValueRef &Result);

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
          FunctionExecutionState(ModuleInstance &Module, const ir::Function &FunctionValue, const std::vector<RuntimeValueRef> &Arguments, std::size_t Depth, std::size_t FrameId)
              : Module(Module),
                FunctionValue(FunctionValue),
                Frame(Arguments),
                Depth(Depth),
                FrameId(FrameId)
          {
          }

          ModuleInstance &Module;
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
      }

      RuntimeValueRef importValue(RuntimeValueRef Value);
      RuntimeValueRef zeroValue(const ir::Type &TypeValue);
      RuntimeValueRef evaluateValue(const ir::Value &Value, ModuleInstance &Module, const ExecutionFrame &Frame, const ir::Name &FunctionName);
      RuntimeValueRef evaluateGlobalVariableAddress(const ir::GlobalVariableAddressOperand &Address, ModuleInstance &Module, const ir::Name &FunctionName);
      bool validateGlobalVariableAccessType(const ir::Value &Pointer, ModuleInstance &Module, const ir::Type &AccessType);
      InstructionFlow executeInstruction(const ir::Instruction &InstructionValue, FunctionExecutionState &State);
      InstructionFlow executeCallInstruction(const ir::CallInstruction &Call, FunctionExecutionState &State);
      InstructionFlow executeImportInstruction(const ir::ImportInstruction &Import, FunctionExecutionState &State);
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
      bool executeFunction(ModuleInstance &Module, ir::FunctionId Function, const std::vector<RuntimeValueRef> &Arguments, std::size_t Depth, RuntimeValueRef &Result);

      ExecutionContext &Context;
      ModuleExecutionRuntime &Runtime;
      ModuleInstance &EntryModule;
      RuntimeValueArena Values;
      std::unordered_map<RuntimeValueRef, RuntimeValueRef> ImportedValues;
      std::unordered_map<const ir::ByteConstant *, RuntimeValueRef> GlobalPointers;
      std::size_t NextFrameId = 0;
      std::size_t ExecutedInstructionCount = 0;
  };
} // namespace ink::execution

#endif
