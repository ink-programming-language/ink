#include "InkVMTargetMachine.h"

#include "InkVM.h"
#include "TargetInfo/InkVMTargetInfo.h"
#include "ink/backend/llvm_inkvm/inkvm_target.h"

#include <llvm/CodeGen/TargetLoweringObjectFileImpl.h>
#include <llvm/CodeGen/TargetPassConfig.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/PassRegistry.h>

namespace llvm
{
    namespace
    {
        Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM)
        {
            return RM.value_or(Reloc::Static);
        }

        class InkVMPassConfig : public TargetPassConfig
        {
            public:
                InkVMPassConfig(InkVMTargetMachine& TM, PassManagerBase& PM)
                    : TargetPassConfig(TM, PM)
                {
                }

                InkVMTargetMachine& getInkVMTargetMachine() const
                {
                    return getTM<InkVMTargetMachine>();
                }

                bool addInstSelector() override
                {
                    addPass(createInkVMISelDag(getInkVMTargetMachine(), getOptLevel()));
                    return false;
                }
        };
    } // namespace

    InkVMTargetMachine::InkVMTargetMachine(const Target& TheTarget, const Triple& TargetTriple, StringRef CPU, StringRef Features, const TargetOptions& Options, std::optional<Reloc::Model> RM, std::optional<CodeModel::Model> CM, CodeGenOptLevel OptLevel, bool JIT)
        : CodeGenTargetMachineImpl(TheTarget, StringRef(ink::backend::llvm_inkvm::InkVMDataLayout.data(), ink::backend::llvm_inkvm::InkVMDataLayout.size()), TargetTriple, CPU.empty() ? StringRef("generic") : CPU, Features, Options, getEffectiveRelocModel(RM), getEffectiveCodeModel(CM, CodeModel::Small), OptLevel),
          TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
          Subtarget(TargetTriple, CPU, Features, *this)
    {
        static_cast<void>(JIT);
        initAsmInfo();
    }

    InkVMTargetMachine::~InkVMTargetMachine() = default;

    TargetPassConfig* InkVMTargetMachine::createPassConfig(PassManagerBase& PM)
    {
        return new InkVMPassConfig(*this, PM);
    }

    extern "C" void LLVMInitializeInkVMTarget()
    {
        RegisterTargetMachine<InkVMTargetMachine> RegisteredTargetMachine(getTheInkVMTarget());

        PassRegistry& Registry = *PassRegistry::getPassRegistry();
        initializeInkVMDAGToDAGISelLegacyPass(Registry);
        initializeInkVMAsmPrinterPass(Registry);
    }
} // namespace llvm
