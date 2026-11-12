#include "ink/backend/vm/executable_emitter/function_emitter.h"

namespace ink::vm
{
  FunctionEmitter::FunctionEmitter(ExecutableEmissionContext &ExecutableContext) noexcept
      : ExecutableContext(ExecutableContext),
        InstructionEmitter(ExecutableContext, Context)
  {
  }

  void FunctionEmitter::emit(const MachineFunction &Function)
  {
    Context.reset();
    scanUnResolvedBasicBlock(Function);
  }

  void FunctionEmitter::scanUnResolvedBasicBlock(const MachineFunction &Function)
  {
    Context.scanUnResolvedBasicBlock(Function);
  }
} // namespace ink::vm
