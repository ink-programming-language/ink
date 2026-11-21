#pragma once

#include "InkVMFrameLowering.h"
#include "InkVMISelLowering.h"
#include "InkVMInstrInfo.h"
#include "InkVMRegisterInfo.h"

#include <llvm/CodeGen/SelectionDAGTargetInfo.h>
#include <llvm/CodeGen/TargetSubtargetInfo.h>

#include <memory>
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "InkVMGenSubtargetInfo.inc"

namespace llvm
{
    class StringRef;
    class TargetMachine;
    class Triple;

    class InkVMSubtarget : public InkVMGenSubtargetInfo
    {
        public:
            InkVMSubtarget(const Triple& TargetTriple, StringRef CPUName, StringRef Features, const TargetMachine& TM);
            ~InkVMSubtarget() override;

            void ParseSubtargetFeatures(StringRef CPUName, StringRef TuneCPU, StringRef FS);

            const InkVMInstrInfo* getInstrInfo() const override
            {
                return &InstrInfo;
            }

            const InkVMRegisterInfo* getRegisterInfo() const override
            {
                return &InstrInfo.getRegisterInfo();
            }

            const InkVMFrameLowering* getFrameLowering() const override
            {
                return &FrameLowering;
            }

            const InkVMTargetLowering* getTargetLowering() const override
            {
                return &TargetLoweringInfo;
            }

            const SelectionDAGTargetInfo* getSelectionDAGInfo() const override
            {
                return TSInfo.get();
            }

        private:
            virtual void anchor();
            InkVMSubtarget& initializeSubtargetDependencies(StringRef CPUName, StringRef Features);

            InkVMInstrInfo InstrInfo;
            InkVMFrameLowering FrameLowering;
            InkVMTargetLowering TargetLoweringInfo;
            std::unique_ptr<const SelectionDAGTargetInfo> TSInfo;
    };
} // namespace llvm
