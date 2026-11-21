#pragma once

#include <llvm/Support/CodeGen.h>

namespace llvm
{
    class FunctionPass;
    class InkVMTargetMachine;
    class PassRegistry;

    extern "C" void LLVMInitializeInkVMTargetInfo();
    extern "C" void LLVMInitializeInkVMTarget();
    extern "C" void LLVMInitializeInkVMTargetMC();
    extern "C" void LLVMInitializeInkVMAsmPrinter();

    void initializeInkVMDAGToDAGISelLegacyPass(PassRegistry& Registry);
    void initializeInkVMAsmPrinterPass(PassRegistry& Registry);

    FunctionPass* createInkVMISelDag(InkVMTargetMachine& TM, CodeGenOptLevel OptLevel);
} // namespace llvm
