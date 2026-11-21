#include "InkVMRegisterInfo.h"

#include "InkVMFrameLowering.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include <llvm/ADT/BitVector.h>
#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/MachineInstr.h>
#include <llvm/CodeGen/TargetSubtargetInfo.h>

#include <cassert>
#include <cstdint>

#define GET_REGINFO_TARGET_DESC
#include "InkVMGenRegisterInfo.inc"

namespace llvm
{
    InkVMRegisterInfo::InkVMRegisterInfo()
        : InkVMGenRegisterInfo(InkVM::PC)
    {
    }

    const MCPhysReg* InkVMRegisterInfo::getCalleeSavedRegs(const MachineFunction* MF) const
    {
        static_cast<void>(MF);
        // Per docs/VM-Calling-Convention.md, all general-purpose registers R0-R9 are caller-saved.
        static constexpr MCPhysReg CalleeSavedRegs[] = {
            0,
        };
        return CalleeSavedRegs;
    }

    BitVector InkVMRegisterInfo::getReservedRegs(const MachineFunction& MF) const
    {
        static_cast<void>(MF);
        BitVector Reserved(getNumRegs());
        Reserved.set(InkVM::BP);
        Reserved.set(InkVM::SP);
        Reserved.set(InkVM::PC);
        Reserved.set(InkVM::ZERO);
        return Reserved;
    }

    bool InkVMRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj, unsigned FIOperandNum, RegScavenger* RS) const
    {
        static_cast<void>(SPAdj);
        static_cast<void>(RS);

        MachineInstr& MI = *II;
        MachineFunction& MF = *MI.getParent()->getParent();
        const int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
        const std::int64_t ObjectOffset = MF.getFrameInfo().getObjectOffset(FrameIndex);

        assert(FIOperandNum + 1 < MI.getNumOperands() && MI.getOperand(FIOperandNum + 1).isImm() && "Expected immediate displacement right after FrameIndex");
        const std::int64_t TotalOffset = ObjectOffset + MI.getOperand(FIOperandNum + 1).getImm();

        MI.getOperand(FIOperandNum).ChangeToRegister(InkVM::BP, false);
        MI.getOperand(FIOperandNum + 1).ChangeToImmediate(TotalOffset);
        return false;
    }

    Register InkVMRegisterInfo::getFrameRegister(const MachineFunction& MF) const
    {
        static_cast<void>(MF);
        return InkVM::BP;
    }
} // namespace llvm
