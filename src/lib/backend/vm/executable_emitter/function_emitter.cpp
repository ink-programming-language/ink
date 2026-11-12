#include "ink/backend/vm/executable_emitter/function_emitter.h"

#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/module/machine_function.h"

namespace ink::vm
{
  FunctionEmitter::FunctionEmitter(ExecutableEmitter &Emitter) noexcept
      : Emitter(Emitter)
  {
  }

  void FunctionEmitter::emit(const MachineFunction &Function)
  {
    ResolvedBasicBlock.clear();
    scanUnResolvedBasicBlock(Function);
  }

  void FunctionEmitter::scanUnResolvedBasicBlock(const MachineFunction &Function)
  {
    UnResolvedBasicBlock.clear();
    for (const MachineBasicBlock &Block : Function.basicBlocks())
    {
      UnResolvedBasicBlock.try_emplace(&Block, 0);
    }
  }
} // namespace ink::vm
