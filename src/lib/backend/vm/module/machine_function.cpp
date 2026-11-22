#include "ink/backend/vm/module/machine_function.h"

#include <iterator>

namespace ink::vm
{
    MachineFunction::MachineFunction(core::Name Name, MachineSymbolLinkage Linkage) noexcept
        : Name(Name),
          Linkage(Linkage)
    {
    }

    MachineBasicBlock& MachineFunction::createBasicBlock(core::Name BlockName)
    {
        MachineBasicBlock& Block = Blocks.emplace_back(BlockName);
        Block.Parent = this;
        Block.Position = std::prev(Blocks.end());
        return Block;
    }

    bool MachineFunction::moveBasicBlockToEnd(MachineBasicBlock& Block) noexcept
    {
        if (Block.Parent != this)
        {
            return false;
        }
        Blocks.splice(Blocks.end(), Blocks, Block.Position);
        return true;
    }
} // namespace ink::vm
