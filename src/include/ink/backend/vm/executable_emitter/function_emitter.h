#pragma once

#include "ink/backend/vm/executable_emitter/function_emission_context.h"
#include "ink/backend/vm/executable_emitter/instruction_emitter.h"

namespace ink::vm
{
  class ExecutableEmissionContext;
  class MachineBasicBlock;
  class MachineFunction;

  class FunctionEmitter
  {
    public:
      // The borrowed executable context must outlive this emitter.
      explicit FunctionEmitter(ExecutableEmissionContext &ExecutableContext) noexcept;

      FunctionEmitter(const FunctionEmitter &) = delete;
      FunctionEmitter &operator=(const FunctionEmitter &) = delete;
      FunctionEmitter(FunctionEmitter &&) = delete;
      FunctionEmitter &operator=(FunctionEmitter &&) = delete;

      // Borrows Function and its blocks until the next emission or scan replaces the block state.
      void emit(const MachineFunction &Function);

      auto &getContext(this auto &&Self) noexcept
      {
        return Self.Context;
      }

      // Rebuilds the block map with zero offsets until function layout is known.
      void scanUnResolvedBasicBlock(const MachineFunction &Function);

      // Records the block's image offset, then appends its instructions in order.
      void padBasicBlock(const MachineBasicBlock &Block);

      // Resolves forward basic-block references after every block has an image offset.
      void resolveBasicBlockStaticRelocations();

    private:
      ExecutableEmissionContext &ExecutableContext;
      FunctionEmissionContext Context;
      vm::InstructionEmitter InstructionEmitter;
  };
} // namespace ink::vm
