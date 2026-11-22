#pragma once

#include "InkVMSubtarget.h"

#include <llvm/CodeGen/CodeGenTargetMachineImpl.h>

#include <memory>
#include <optional>

namespace llvm
{
    class TargetLoweringObjectFile;
    struct InkVMMachineModuleEmitSink;

    class InkVMTargetMachine : public CodeGenTargetMachineImpl
    {
        public:
            InkVMTargetMachine(const Target& TheTarget, const Triple& TargetTriple, StringRef CPU, StringRef Features, const TargetOptions& Options, std::optional<Reloc::Model> RM, std::optional<CodeModel::Model> CM, CodeGenOptLevel OptLevel, bool JIT);
            ~InkVMTargetMachine() override;

            const InkVMSubtarget* getSubtargetImpl() const
            {
                return &Subtarget;
            }

            const InkVMSubtarget* getSubtargetImpl(const Function& Func) const override
            {
                static_cast<void>(Func);
                return &Subtarget;
            }

            TargetPassConfig* createPassConfig(PassManagerBase& PM) override;

            bool addAsmPrinter(PassManagerBase& PM, raw_pwrite_stream& Out, raw_pwrite_stream* DwoOut, CodeGenFileType FileType, MCContext& Context) override;

            bool addPassesToEmitMachineModule(PassManagerBase& PM, InkVMMachineModuleEmitSink& Sink, bool DisableVerify = true);

            TargetLoweringObjectFile* getObjFileLowering() const override
            {
                return TLOF.get();
            }

        private:
            std::unique_ptr<TargetLoweringObjectFile> TLOF;
            InkVMSubtarget Subtarget;
            InkVMMachineModuleEmitSink* PendingMachineModuleSink = nullptr;
    };
} // namespace llvm
