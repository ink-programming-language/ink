#include "InkVMFrameLowering.h"

#include "InkVMSubtarget.h"

namespace llvm
{
    InkVMFrameLowering::InkVMFrameLowering(const InkVMSubtarget& Subtarget)
        : TargetFrameLowering(TargetFrameLowering::StackGrowsUp, Align(8), 16, Align(8))
    {
        static_cast<void>(Subtarget);
    }

    bool InkVMFrameLowering::hasFPImpl(const MachineFunction& MF) const
    {
        static_cast<void>(MF);
        // InkVM's call/ret instructions always maintain BP as the fixed frame base.
        return true;
    }

    void InkVMFrameLowering::emitPrologue(MachineFunction& MF, MachineBasicBlock& MBB) const
    {
        static_cast<void>(MF);
        static_cast<void>(MBB);
    }

    void InkVMFrameLowering::emitEpilogue(MachineFunction& MF, MachineBasicBlock& MBB) const
    {
        static_cast<void>(MF);
        static_cast<void>(MBB);
    }
} // namespace llvm
