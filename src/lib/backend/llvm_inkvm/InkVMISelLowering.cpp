#include "InkVMISelLowering.h"

#include "InkVMRegisterInfo.h"
#include "InkVMSubtarget.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"

namespace llvm
{
    InkVMTargetLowering::InkVMTargetLowering(const TargetMachine& TM, const InkVMSubtarget& Subtarget)
        : TargetLowering(TM, Subtarget)
    {
        addRegisterClass(MVT::i64, &InkVM::GPRRegClass);
        computeRegisterProperties(Subtarget.getRegisterInfo());

        setStackPointerRegisterToSaveRestore(InkVM::SP);
    }
} // namespace llvm
