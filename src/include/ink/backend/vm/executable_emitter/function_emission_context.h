#pragma once

#include <cstdint>
#include <unordered_map>

namespace ink::vm
{
  class MachineBasicBlock;
  class MachineFunction;

  // Owns layout state for one function; block pointers borrow the source function.
  class FunctionEmissionContext
  {
    public:
      void reset() noexcept;

      // Rebuilds the block map with zero offsets until function layout is known.
      void scanUnResolvedBasicBlock(const MachineFunction &Function);

    private:
      std::unordered_map<const MachineBasicBlock *, std::uint64_t> UnResolvedBasicBlock;
      std::unordered_map<const MachineBasicBlock *, std::uint64_t> ResolvedBasicBlock;
  };
} // namespace ink::vm
