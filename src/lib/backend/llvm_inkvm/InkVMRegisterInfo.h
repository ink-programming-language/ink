#pragma once

#include <llvm/CodeGen/TargetRegisterInfo.h>

#define GET_REGINFO_HEADER
#include "InkVMGenRegisterInfo.inc"

namespace llvm
{
    class InkVMRegisterInfo : public InkVMGenRegisterInfo
    {
        public:
            InkVMRegisterInfo();

            const MCPhysReg* getCalleeSavedRegs(const MachineFunction* MF) const override;
            const uint32_t* getCallPreservedMask(const MachineFunction& MF, CallingConv::ID CC) const override;
            const uint32_t* getNoPreservedMask() const override;
            BitVector getReservedRegs(const MachineFunction& MF) const override;
            bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj, unsigned FIOperandNum, RegScavenger* RS = nullptr) const override;
            Register getFrameRegister(const MachineFunction& MF) const override;
    };
} // namespace llvm
