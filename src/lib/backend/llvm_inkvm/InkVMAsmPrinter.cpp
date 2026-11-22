#include "InkVM.h"
#include "InkVMMachineModuleEmitter.h"
#include "TargetInfo/InkVMTargetInfo.h"

#include <llvm/CodeGen/AsmPrinter.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/MachineModuleInfo.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCStreamer.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Pass.h>
#include <llvm/Support/SMLoc.h>
#include <llvm/Target/TargetLoweringObjectFile.h>
#include <llvm/Target/TargetMachine.h>

#include "ink/backend/vm/serialization/machine_module_text.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"

#include <memory>
#include <utility>

#define DEBUG_TYPE "inkvm-asm-printer"
#define PASS_NAME "InkVM Assembly Printer"

using namespace llvm;

namespace
{
    // Routes LLVM's AssemblyFile path through InkVMMachineModuleBuilder and the canonical VM text serializer.
    class InkVMAsmPrinter final : public AsmPrinter
    {
        public:
            static char ID;

            InkVMAsmPrinter(TargetMachine& TM, std::unique_ptr<MCStreamer> Streamer)
                : AsmPrinter(TM, std::move(Streamer), ID)
            {
            }

            StringRef getPassName() const override
            {
                return PASS_NAME;
            }

            bool doInitialization(Module& M) override
            {
                auto* MMIWP = getAnalysisIfAvailable<MachineModuleInfoWrapperPass>();
                MMI = MMIWP != nullptr ? &MMIWP->getMMI() : nullptr;
                TM.getObjFileLowering()->Initialize(OutContext, TM);
                Builder.initialize(M);
                return false;
            }

            bool runOnMachineFunction(MachineFunction& MachineFunc) override
            {
                SetupMachineFunction(MachineFunc);
                if (!Builder.hasFailed())
                {
                    Builder.emitFunction(MachineFunc);
                }
                return false;
            }

            bool doFinalization(Module& M) override
            {
                static_cast<void>(M);
                auto ModuleResult = Builder.takeModule();
                if (!ModuleResult)
                {
                    OutContext.reportError(SMLoc(), ModuleResult.error());
                }
                else
                {
                    auto TextResult = ink::vm::writeMachineModuleText(**ModuleResult);
                    if (!TextResult)
                    {
                        const auto Formatted = ink::core::DiagnosticFormatter{}.format(TextResult.error());
                        OutContext.reportError(SMLoc(), Formatted.Message);
                    }
                    else
                    {
                        OutStreamer->emitRawText(*TextResult);
                    }
                }
                MF = nullptr;
                MMI = nullptr;
                OutStreamer->finish();
                OutStreamer->reset();
                return false;
            }

        private:
            InkVMMachineModuleBuilder Builder;
    };
} // namespace

char InkVMAsmPrinter::ID = 0;

INITIALIZE_PASS(InkVMAsmPrinter, DEBUG_TYPE, PASS_NAME, false, false)

namespace llvm
{
    extern "C" void LLVMInitializeInkVMAsmPrinter()
    {
        RegisterAsmPrinter<InkVMAsmPrinter> RegisteredAsmPrinter(getTheInkVMTarget());
    }
} // namespace llvm
