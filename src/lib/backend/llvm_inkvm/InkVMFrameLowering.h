#pragma once

#include <llvm/CodeGen/TargetFrameLowering.h>

namespace llvm
{
    class InkVMSubtarget;

    class InkVMFrameLowering : public TargetFrameLowering
    {
        public:
            explicit InkVMFrameLowering(const InkVMSubtarget& Subtarget);

            void emitPrologue(MachineFunction& MF, MachineBasicBlock& MBB) const override;
            void emitEpilogue(MachineFunction& MF, MachineBasicBlock& MBB) const override;

            bool hasReservedCallFrame(const MachineFunction& MF) const override;
            MachineBasicBlock::iterator eliminateCallFramePseudoInstr(MachineFunction& MF, MachineBasicBlock& MBB, MachineBasicBlock::iterator I) const override;

        protected:
            bool hasFPImpl(const MachineFunction& MF) const override;
    };
} // namespace llvm
