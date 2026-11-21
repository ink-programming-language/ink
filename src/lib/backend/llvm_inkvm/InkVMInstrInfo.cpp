#include "InkVMInstrInfo.h"

#include "InkVMSubtarget.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include <llvm/CodeGen/MachineBasicBlock.h>
#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/MachineInstrBuilder.h>
#include <llvm/Support/ErrorHandling.h>

#include <cassert>
#include <iterator>

#define GET_INSTRINFO_CTOR_DTOR
#include "InkVMGenInstrInfo.inc"

namespace llvm
{
    namespace
    {
        unsigned getOppositeBranchOpcode(unsigned Opcode)
        {
            switch (Opcode)
            {
            case InkVM::JE64rrb:
                return InkVM::JNE64rrb;
            case InkVM::JE64rib:
                return InkVM::JNE64rib;
            case InkVM::JNE64rrb:
                return InkVM::JE64rrb;
            case InkVM::JNE64rib:
                return InkVM::JE64rib;
            case InkVM::JLT64rrb:
                return InkVM::JGE64rrb;
            case InkVM::JLT64rib:
                return InkVM::JGE64rib;
            case InkVM::JLE64rrb:
                return InkVM::JGT64rrb;
            case InkVM::JLE64rib:
                return InkVM::JGT64rib;
            case InkVM::JGT64rrb:
                return InkVM::JLE64rrb;
            case InkVM::JGT64rib:
                return InkVM::JLE64rib;
            case InkVM::JGE64rrb:
                return InkVM::JLT64rrb;
            case InkVM::JGE64rib:
                return InkVM::JLT64rib;
            case InkVM::JLTU64rrb:
                return InkVM::JGEU64rrb;
            case InkVM::JLTU64rib:
                return InkVM::JGEU64rib;
            case InkVM::JLEU64rrb:
                return InkVM::JGTU64rrb;
            case InkVM::JLEU64rib:
                return InkVM::JGTU64rib;
            case InkVM::JGTU64rrb:
                return InkVM::JLEU64rrb;
            case InkVM::JGTU64rib:
                return InkVM::JLEU64rib;
            case InkVM::JGEU64rrb:
                return InkVM::JLTU64rrb;
            case InkVM::JGEU64rib:
                return InkVM::JLTU64rib;
            default:
                llvm_unreachable("Unknown InkVM conditional branch opcode");
            }
        }

        void parseCondBranch(const MachineInstr& MI, MachineBasicBlock*& Target, SmallVectorImpl<MachineOperand>& Cond)
        {
            Target = MI.getOperand(2).getMBB();
            Cond.push_back(MachineOperand::CreateImm(MI.getOpcode()));
            MachineOperand LHS = MI.getOperand(0);
            MachineOperand RHS = MI.getOperand(1);
            if (LHS.isReg())
            {
                LHS.setIsKill(false);
            }
            if (RHS.isReg())
            {
                RHS.setIsKill(false);
            }
            Cond.push_back(LHS);
            Cond.push_back(RHS);
        }
    } // namespace

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

        const unsigned StoreOpc = RC == &InkVM::FPRRegClass ? InkVM::STORE64mr_f64 : InkVM::STORE64mr;
        BuildMI(MBB, InsertPos, DL, get(StoreOpc)).addFrameIndex(FrameIndex).addImm(0).addReg(SrcReg, getKillRegState(IsKill)).addMemOperand(MMO);
    }

    void InkVMInstrInfo::loadRegFromStackSlot(MachineBasicBlock& MBB, MachineBasicBlock::iterator InsertPos, Register DestReg, int FrameIndex, const TargetRegisterClass* RC, Register VReg, unsigned SubReg, MachineInstr::MIFlag Flags) const
    {
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

        const unsigned LoadOpc = RC == &InkVM::FPRRegClass ? InkVM::LOAD64rm_f64 : InkVM::LOAD64rm;
        BuildMI(MBB, InsertPos, DL, get(LoadOpc), DestReg).addFrameIndex(FrameIndex).addImm(0).addMemOperand(MMO);
    }

    bool InkVMInstrInfo::analyzeBranch(MachineBasicBlock& MBB, MachineBasicBlock*& TBB, MachineBasicBlock*& FBB, SmallVectorImpl<MachineOperand>& Cond, bool AllowModify) const
    {
        TBB = nullptr;
        FBB = nullptr;
        Cond.clear();

        MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
        if (I == MBB.end() || !isUnpredicatedTerminator(*I))
        {
            return false;
        }

        MachineBasicBlock::iterator FirstUncondOrIndirectBr = MBB.end();
        int NumTerminators = 0;
        for (auto J = I.getReverse(); J != MBB.rend() && isUnpredicatedTerminator(*J); ++J)
        {
            ++NumTerminators;
            if (J->getDesc().isUnconditionalBranch() || J->getDesc().isIndirectBranch())
            {
                FirstUncondOrIndirectBr = J.getReverse();
            }
        }

        if (AllowModify && FirstUncondOrIndirectBr != MBB.end())
        {
            while (std::next(FirstUncondOrIndirectBr) != MBB.end())
            {
                std::next(FirstUncondOrIndirectBr)->eraseFromParent();
                --NumTerminators;
            }
            I = FirstUncondOrIndirectBr;
        }

        if (I->getDesc().isIndirectBranch() || I->isPreISelOpcode() || NumTerminators > 2)
        {
            return true;
        }

        if (NumTerminators == 1 && I->getDesc().isUnconditionalBranch())
        {
            TBB = I->getOperand(0).getMBB();
            return false;
        }

        if (NumTerminators == 1 && I->getDesc().isConditionalBranch())
        {
            parseCondBranch(*I, TBB, Cond);
            return false;
        }

        if (NumTerminators == 2 && std::prev(I)->getDesc().isConditionalBranch() && I->getDesc().isUnconditionalBranch())
        {
            parseCondBranch(*std::prev(I), TBB, Cond);
            FBB = I->getOperand(0).getMBB();
            return false;
        }

        return true;
    }

    unsigned InkVMInstrInfo::removeBranch(MachineBasicBlock& MBB, int* BytesRemoved) const
    {
        if (BytesRemoved != nullptr)
        {
            *BytesRemoved = 0;
        }

        MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
        if (I == MBB.end())
        {
            return 0;
        }

        if (!I->getDesc().isUnconditionalBranch() && !I->getDesc().isConditionalBranch())
        {
            return 0;
        }

        if (BytesRemoved != nullptr)
        {
            *BytesRemoved += static_cast<int>(I->getDesc().getSize());
        }
        I->eraseFromParent();

        I = MBB.getLastNonDebugInstr();
        if (I == MBB.end() || !I->getDesc().isConditionalBranch())
        {
            return 1;
        }

        if (BytesRemoved != nullptr)
        {
            *BytesRemoved += static_cast<int>(I->getDesc().getSize());
        }
        I->eraseFromParent();
        return 2;
    }

    unsigned InkVMInstrInfo::insertBranch(MachineBasicBlock& MBB, MachineBasicBlock* TBB, MachineBasicBlock* FBB, ArrayRef<MachineOperand> Cond, const DebugLoc& DL, int* BytesAdded) const
    {
        if (BytesAdded != nullptr)
        {
            *BytesAdded = 0;
        }

        assert(TBB != nullptr && "insertBranch must not be told to insert a fallthrough");
        assert((Cond.empty() || Cond.size() == 3) && "InkVM branch conditions carry [Opcode, LHS, RHS]");

        if (Cond.empty())
        {
            MachineInstr& JumpMI = *BuildMI(&MBB, DL, get(InkVM::JMP)).addMBB(TBB);
            if (BytesAdded != nullptr)
            {
                *BytesAdded += static_cast<int>(JumpMI.getDesc().getSize());
            }
            return 1;
        }

        MachineInstr& CondMI = *BuildMI(&MBB, DL, get(static_cast<unsigned>(Cond[0].getImm()))).add(Cond[1]).add(Cond[2]).addMBB(TBB);
        if (BytesAdded != nullptr)
        {
            *BytesAdded += static_cast<int>(CondMI.getDesc().getSize());
        }

        if (FBB == nullptr)
        {
            return 1;
        }

        MachineInstr& JumpMI = *BuildMI(&MBB, DL, get(InkVM::JMP)).addMBB(FBB);
        if (BytesAdded != nullptr)
        {
            *BytesAdded += static_cast<int>(JumpMI.getDesc().getSize());
        }
        return 2;
    }

    bool InkVMInstrInfo::reverseBranchCondition(SmallVectorImpl<MachineOperand>& Cond) const
    {
        assert(Cond.size() == 3 && "Invalid InkVM branch condition");
        Cond[0].setImm(getOppositeBranchOpcode(static_cast<unsigned>(Cond[0].getImm())));
        return false;
    }
} // namespace llvm
