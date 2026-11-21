#pragma once

#include <llvm/CodeGen/TargetLowering.h>

namespace llvm
{
    class InkVMSubtarget;

    class InkVMTargetLowering : public TargetLowering
    {
        public:
            explicit InkVMTargetLowering(const TargetMachine& TM, const InkVMSubtarget& Subtarget);
    };
} // namespace llvm
