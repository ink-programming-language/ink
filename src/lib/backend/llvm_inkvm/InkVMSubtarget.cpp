#include "InkVMSubtarget.h"

#include <llvm/Target/TargetMachine.h>

#define DEBUG_TYPE "inkvm-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "InkVMGenSubtargetInfo.inc"

namespace llvm
{
    void InkVMSubtarget::anchor()
    {
    }

    InkVMSubtarget& InkVMSubtarget::initializeSubtargetDependencies(StringRef CPUName, StringRef Features)
    {
        const StringRef ResolvedCPU = CPUName.empty() ? StringRef("generic") : CPUName;
        ParseSubtargetFeatures(ResolvedCPU, ResolvedCPU, Features);
        return *this;
    }

    InkVMSubtarget::InkVMSubtarget(const Triple& TargetTriple, StringRef CPUName, StringRef Features, const TargetMachine& TM)
        : InkVMGenSubtargetInfo(TargetTriple, CPUName.empty() ? StringRef("generic") : CPUName, CPUName.empty() ? StringRef("generic") : CPUName, Features),
          InstrInfo(initializeSubtargetDependencies(CPUName, Features)),
          FrameLowering(*this),
          TargetLoweringInfo(TM, *this),
          TSInfo(std::make_unique<SelectionDAGTargetInfo>())
    {
    }

    InkVMSubtarget::~InkVMSubtarget() = default;
} // namespace llvm
