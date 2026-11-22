#pragma once

#include <llvm/Support/CodeGen.h>

namespace llvm
{
    class FunctionPass;
    class InkVMTargetMachine;
    class PassRegistry;
    struct InkVMMachineModuleEmitSink;

    extern "C" void LLVMInitializeInkVMTargetInfo();
    extern "C" void LLVMInitializeInkVMTarget();
    extern "C" void LLVMInitializeInkVMTargetMC();
    extern "C" void LLVMInitializeInkVMAsmPrinter();

    void initializeInkVMDAGToDAGISelLegacyPass(PassRegistry& Registry);
    void initializeInkVMAsmPrinterPass(PassRegistry& Registry);
    void initializeInkVMMachineModuleEmitterPass(PassRegistry& Registry);

    FunctionPass* createInkVMISelDag(InkVMTargetMachine& TM, CodeGenOptLevel OptLevel);
    FunctionPass* createInkVMMachineModuleEmitter(InkVMTargetMachine& TM, InkVMMachineModuleEmitSink& Sink);
} // namespace llvm
