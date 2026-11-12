#include "ink/backend/vm/executable_emitter/function_emitter.h"

#include "ink/backend/vm/executable_emitter/executable_emission_context.h"
#include "ink/backend/vm/module/machine_function.h"

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
    for (const MachineBasicBlock &Block : Function.basicBlocks())
    {
      padBasicBlock(Block);
    }
    resolveBasicBlockStaticRelocations();
  }

  void FunctionEmitter::scanUnResolvedBasicBlock(const MachineFunction &Function)
  {
    Context.scanUnResolvedBasicBlock(Function);
  }

  void FunctionEmitter::padBasicBlock(const MachineBasicBlock &Block)
  {
    Context.resolveBasicBlock(Block, ExecutableContext.byteSize());
    for (const MachineInstruction &Instruction : Block.instructions())
    {
      InstructionEmitter.padInstruction(Instruction);
    }
  }

  void FunctionEmitter::resolveBasicBlockStaticRelocations()
  {
    Context.resolveStaticRelocations(ExecutableContext);
  }
} // namespace ink::vm
