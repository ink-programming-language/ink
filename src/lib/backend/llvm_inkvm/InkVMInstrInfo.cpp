#include "InkVMInstrInfo.h"

#include "InkVMSubtarget.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "InkVMGenInstrInfo.inc"

namespace llvm
{
    void InkVMInstrInfo::anchor()
    {
    }

    InkVMInstrInfo::InkVMInstrInfo(const InkVMSubtarget& Subtarget)
        : InkVMGenInstrInfo(Subtarget, RegisterInfo, ~0u, ~0u, ~0u, InkVM::RET),
          RegisterInfo()
    {
    }
} // namespace llvm
