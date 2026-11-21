#include "InkVMFrameLowering.h"

#include "InkVMInstrInfo.h"
#include "InkVMSubtarget.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include <llvm/CodeGen/MachineBasicBlock.h>
#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/MachineInstrBuilder.h>
#include <llvm/Support/Alignment.h>

#include <cstdint>

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

    bool InkVMFrameLowering::hasReservedCallFrame(const MachineFunction& MF) const
    {
        static_cast<void>(MF);
        // Each call dynamically reserves and reclaims its own [CallBegin, sp) call area right below sp.
        return false;
    }

    void InkVMFrameLowering::emitPrologue(MachineFunction& MF, MachineBasicBlock& MBB) const
    {
        const std::uint64_t StackSize = alignTo(MF.getFrameInfo().getStackSize(), getStackAlign());
        if (StackSize == 0)
        {
            return;
        }

        const InkVMInstrInfo& TII = *MF.getSubtarget<InkVMSubtarget>().getInstrInfo();
        const MachineBasicBlock::iterator InsertPos = MBB.begin();
        const DebugLoc DL = InsertPos != MBB.end() ? InsertPos->getDebugLoc() : DebugLoc();
        BuildMI(MBB, InsertPos, DL, TII.get(InkVM::ADD64rri), InkVM::SP).addReg(InkVM::SP).addImm(static_cast<std::int64_t>(StackSize)).setMIFlag(MachineInstr::FrameSetup);
    }

    void InkVMFrameLowering::emitEpilogue(MachineFunction& MF, MachineBasicBlock& MBB) const
    {
        static_cast<void>(MF);
        static_cast<void>(MBB);
        // InkVM's ret instruction automatically restores sp = bp, saved bp, and return pc.
    }

    MachineBasicBlock::iterator InkVMFrameLowering::eliminateCallFramePseudoInstr(MachineFunction& MF, MachineBasicBlock& MBB, MachineBasicBlock::iterator I) const
    {
        const std::int64_t Amount = I->getOperand(0).getImm();
        if (Amount != 0)
        {
            const std::int64_t AlignedAmount = static_cast<std::int64_t>(alignTo(static_cast<std::uint64_t>(Amount), getStackAlign()));
            const std::int64_t Delta = I->getOpcode() == InkVM::ADJCALLSTACKDOWN ? AlignedAmount : -AlignedAmount;
            const InkVMInstrInfo& TII = *MF.getSubtarget<InkVMSubtarget>().getInstrInfo();
            BuildMI(MBB, I, I->getDebugLoc(), TII.get(InkVM::ADD64rri), InkVM::SP).addReg(InkVM::SP).addImm(Delta);
        }
        return MBB.erase(I);
    }
} // namespace llvm
