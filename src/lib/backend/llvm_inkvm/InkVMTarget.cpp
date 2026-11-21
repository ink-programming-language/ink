#include "ink/backend/llvm_inkvm/inkvm_target.h"

#include "InkVM.h"

#include <llvm/ADT/SmallString.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Triple.h>

namespace ink::backend::llvm_inkvm
{
    void initializeInkVMTarget() noexcept
    {
        static const bool Initialized = []()
        {
            llvm::LLVMInitializeInkVMTargetInfo();
            llvm::LLVMInitializeInkVMTarget();
            llvm::LLVMInitializeInkVMTargetMC();
            llvm::LLVMInitializeInkVMAsmPrinter();
            return true;
        }();
        static_cast<void>(Initialized);
    }

    std::unique_ptr<llvm::TargetMachine> createInkVMTargetMachine(llvm::CodeGenOptLevel OptLevel, std::string& ErrorMessage)
    {
        initializeInkVMTarget();

        llvm::Triple TargetTriple(llvm::StringRef(InkVMDefaultTriple.data(), InkVMDefaultTriple.size()));
        const llvm::Target* Target = llvm::TargetRegistry::lookupTarget(llvm::StringRef(InkVMTargetName.data(), InkVMTargetName.size()), TargetTriple, ErrorMessage);
        if (Target == nullptr)
        {
            return nullptr;
        }

        const llvm::TargetOptions Options;
        return std::unique_ptr<llvm::TargetMachine>(Target->createTargetMachine(TargetTriple, "generic", "", Options, llvm::Reloc::Static, llvm::CodeModel::Small, OptLevel));
    }

    bool emitInkVMAssembly(llvm::Module& Module, std::string& AssemblyOutput, std::string& ErrorMessage, llvm::CodeGenOptLevel OptLevel)
    {
        AssemblyOutput.clear();
        ErrorMessage.clear();

        const auto TM = createInkVMTargetMachine(OptLevel, ErrorMessage);
        if (TM == nullptr)
        {
            return false;
        }

        Module.setDataLayout(TM->createDataLayout());
        Module.setTargetTriple(TM->getTargetTriple());

        llvm::SmallString<1024> OutputBuffer;
        llvm::raw_svector_ostream OutputStream(OutputBuffer);
        llvm::legacy::PassManager PassManager;
        if (TM->addPassesToEmitFile(PassManager, OutputStream, nullptr, llvm::CodeGenFileType::AssemblyFile))
        {
            ErrorMessage = "InkVM TargetMachine cannot emit assembly file";
            return false;
        }

        PassManager.run(Module);
        AssemblyOutput.assign(OutputBuffer.data(), OutputBuffer.size());
        return true;
    }
} // namespace ink::backend::llvm_inkvm
