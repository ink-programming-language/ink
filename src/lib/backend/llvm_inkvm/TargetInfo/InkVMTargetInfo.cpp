#include "TargetInfo/InkVMTargetInfo.h"

#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Triple.h>

namespace llvm
{
    namespace
    {
        bool matchesInkVMArch(Triple::ArchType)
        {
            return false;
        }
    } // namespace

    Target& getTheInkVMTarget()
    {
        static Target TheInkVMTarget;
        return TheInkVMTarget;
    }

    extern "C" void LLVMInitializeInkVMTargetInfo()
    {
        // Out-of-tree target lookup uses TargetRegistry::lookupTarget("inkvm", Triple, Error) by target name.
        TargetRegistry::RegisterTarget(getTheInkVMTarget(), "inkvm", "Ink Virtual Machine (64-bit)", "InkVM", matchesInkVMArch, false);
    }
} // namespace llvm
