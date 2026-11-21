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

        private:
            virtual void anchor();

            const InkVMRegisterInfo RegisterInfo;
    };
} // namespace llvm
