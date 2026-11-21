#pragma once

#include "InkVMRegisterInfo.h"

#include <llvm/CodeGen/TargetInstrInfo.h>

#define GET_INSTRINFO_HEADER
#include "InkVMGenInstrInfo.inc"

namespace llvm
{
    class InkVMSubtarget;

    class InkVMInstrInfo : public InkVMGenInstrInfo
    {
        public:
            explicit InkVMInstrInfo(const InkVMSubtarget& Subtarget);

            const InkVMRegisterInfo& getRegisterInfo() const
            {
                return RegisterInfo;
            }

            void copyPhysReg(MachineBasicBlock& MBB, MachineBasicBlock::iterator InsertPos, const DebugLoc& DL, Register DestReg, Register SrcReg, bool KillSrc, bool RenamableDest = false, bool RenamableSrc = false) const override;

            void storeRegToStackSlot(MachineBasicBlock& MBB, MachineBasicBlock::iterator InsertPos, Register SrcReg, bool IsKill, int FrameIndex, const TargetRegisterClass* RC, Register VReg, MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

            void loadRegFromStackSlot(MachineBasicBlock& MBB, MachineBasicBlock::iterator InsertPos, Register DestReg, int FrameIndex, const TargetRegisterClass* RC, Register VReg, unsigned SubReg = 0, MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

            bool analyzeBranch(MachineBasicBlock& MBB, MachineBasicBlock*& TBB, MachineBasicBlock*& FBB, SmallVectorImpl<MachineOperand>& Cond, bool AllowModify = false) const override;

            unsigned removeBranch(MachineBasicBlock& MBB, int* BytesRemoved = nullptr) const override;

            unsigned insertBranch(MachineBasicBlock& MBB, MachineBasicBlock* TBB, MachineBasicBlock* FBB, ArrayRef<MachineOperand> Cond, const DebugLoc& DL, int* BytesAdded = nullptr) const override;

            bool reverseBranchCondition(SmallVectorImpl<MachineOperand>& Cond) const override;

        private:
            virtual void anchor();

            const InkVMRegisterInfo RegisterInfo;
    };
} // namespace llvm
