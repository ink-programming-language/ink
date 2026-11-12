#pragma once

#include <cstdint>
#include <unordered_map>

namespace ink::vm
{
  class ExecutableEmitter;
  class MachineBasicBlock;
  class MachineFunction;

  class FunctionEmitter
  {
    public:
      // The borrowed executable emitter must outlive this emitter.
      explicit FunctionEmitter(ExecutableEmitter &Emitter) noexcept;

      // Borrows Function and its blocks until the next emission or scan replaces the block state.
      void emit(const MachineFunction &Function);

      // Rebuilds the block map with zero offsets until function layout is known.
      void scanUnResolvedBasicBlock(const MachineFunction &Function);

    private:
      ExecutableEmitter &Emitter;
      std::unordered_map<const MachineBasicBlock *, std::uint64_t> UnResolvedBasicBlock;
      std::unordered_map<const MachineBasicBlock *, std::uint64_t> ResolvedBasicBlock;
  };
} // namespace ink::vm
