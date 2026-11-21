#include "InkVMInstrInfo.h"

#include "InkVMSubtarget.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/MachineInstrBuilder.h>
#include <llvm/Support/ErrorHandling.h>

#define GET_INSTRINFO_CTOR_DTOR
#include "InkVMGenInstrInfo.inc"

namespace llvm
{
    void InkVMInstrInfo::anchor()
    {
    }

    InkVMInstrInfo::InkVMInstrInfo(const InkVMSubtarget& Subtarget)
        : InkVMGenInstrInfo(Subtarget, RegisterInfo, InkVM::ADJCALLSTACKDOWN, InkVM::ADJCALLSTACKUP, ~0u, InkVM::RET),
          RegisterInfo()
    {
    }

    void InkVMInstrInfo::copyPhysReg(MachineBasicBlock& MBB, MachineBasicBlock::iterator InsertPos, const DebugLoc& DL, Register DestReg, Register SrcReg, bool KillSrc, bool RenamableDest, bool RenamableSrc) const
    {
        static_cast<void>(RenamableDest);
        static_cast<void>(RenamableSrc);

        if (InkVM::GPRRegClass.contains(DestReg, SrcReg))
        {
            BuildMI(MBB, InsertPos, DL, get(InkVM::MOV64rr), DestReg).addReg(SrcReg, getKillRegState(KillSrc));
            return;
        }
        llvm_unreachable("Unsupported InkVM physical register copy");
    }

    void InkVMInstrInfo::storeRegToStackSlot(MachineBasicBlock& MBB, MachineBasicBlock::iterator InsertPos, Register SrcReg, bool IsKill, int FrameIndex, const TargetRegisterClass* RC, Register VReg, MachineInstr::MIFlag Flags) const
    {
        static_cast<void>(RC);
        static_cast<void>(VReg);
        static_cast<void>(Flags);

        DebugLoc DL;
        if (InsertPos != MBB.end())
        {
            DL = InsertPos->getDebugLoc();
        }
        MachineFunction& MF = *MBB.getParent();
        const MachineFrameInfo& MFI = MF.getFrameInfo();
        MachineMemOperand* MMO = MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(MF, FrameIndex), MachineMemOperand::MOStore, MFI.getObjectSize(FrameIndex), MFI.getObjectAlign(FrameIndex));

        BuildMI(MBB, InsertPos, DL, get(InkVM::STORE64mr)).addFrameIndex(FrameIndex).addImm(0).addReg(SrcReg, getKillRegState(IsKill)).addMemOperand(MMO);
    }

    void InkVMInstrInfo::loadRegFromStackSlot(MachineBasicBlock& MBB, MachineBasicBlock::iterator InsertPos, Register DestReg, int FrameIndex, const TargetRegisterClass* RC, Register VReg, unsigned SubReg, MachineInstr::MIFlag Flags) const
    {
        static_cast<void>(RC);
        static_cast<void>(VReg);
        static_cast<void>(SubReg);
        static_cast<void>(Flags);

        DebugLoc DL;
        if (InsertPos != MBB.end())
        {
            DL = InsertPos->getDebugLoc();
        }
        MachineFunction& MF = *MBB.getParent();
        const MachineFrameInfo& MFI = MF.getFrameInfo();
        MachineMemOperand* MMO = MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(MF, FrameIndex), MachineMemOperand::MOLoad, MFI.getObjectSize(FrameIndex), MFI.getObjectAlign(FrameIndex));

        BuildMI(MBB, InsertPos, DL, get(InkVM::LOAD64rm), DestReg).addFrameIndex(FrameIndex).addImm(0).addMemOperand(MMO);
    }
} // namespace llvm
