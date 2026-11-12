#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

namespace ink::vm
{
  class ExecutableEmissionContext;
  class MachineBasicBlock;
  class MachineFunction;

  // Owns layout and relocation state for one function; block pointers borrow the source function.
  class FunctionEmissionContext
  {
    public:
      struct StaticRelocationBasicBlock
      {
          std::uint64_t ImageOffset = 0;
          const MachineBasicBlock *Block = nullptr;
          std::uint64_t InstructionOffset = 0;
      };

      void reset() noexcept;

      // Rebuilds the block map with zero offsets until function layout is known.
      void scanUnResolvedBasicBlock(const MachineFunction &Function);

      // Assigns an image offset to the block and removes its unresolved entry.
      void resolveBasicBlock(const MachineBasicBlock &Block, std::uint64_t ImageOffset);
      std::optional<std::uint64_t> resolvedBasicBlockOffset(const MachineBasicBlock &Block) const noexcept;
      bool hasUnResolvedBasicBlock(const MachineBasicBlock &Block) const noexcept;

      void addBasicBlockStaticRelocation(StaticRelocationBasicBlock Relocation);

      // Every referenced block must have an image offset after function layout.
      void resolveStaticRelocations(ExecutableEmissionContext &ExecutableContext);

    private:
      std::unordered_map<const MachineBasicBlock *, std::uint64_t> UnResolvedBasicBlock;
      std::unordered_map<const MachineBasicBlock *, std::uint64_t> ResolvedBasicBlock;
      std::vector<StaticRelocationBasicBlock> PendingStaticRelocationBasicBlock;
  };
} // namespace ink::vm
