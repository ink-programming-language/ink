#include "ink/backend/vm/module/machine_function.h"

namespace ink::vm
{
  MachineFunction::MachineFunction(core::Name Name, MachineSymbolLinkage Linkage) noexcept
      : Name(Name),
        Linkage(Linkage)
  {
  }

  MachineBasicBlock &MachineFunction::createBasicBlock(core::Name BlockName)
  {
    MachineBasicBlock &Block = Blocks.emplace_back(BlockName);
    Block.Parent = this;
    return Block;
  }
} // namespace ink::vm
