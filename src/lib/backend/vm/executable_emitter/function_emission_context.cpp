#include "ink/backend/vm/executable_emitter/function_emission_context.h"

#include "ink/backend/vm/module/machine_function.h"

namespace ink::vm
{
  void FunctionEmissionContext::reset() noexcept
  {
    UnResolvedBasicBlock.clear();
    ResolvedBasicBlock.clear();
  }

  void FunctionEmissionContext::scanUnResolvedBasicBlock(const MachineFunction &Function)
  {
    UnResolvedBasicBlock.clear();
    for (const MachineBasicBlock &Block : Function.basicBlocks())
    {
      UnResolvedBasicBlock.try_emplace(&Block, 0);
    }
  }
} // namespace ink::vm
