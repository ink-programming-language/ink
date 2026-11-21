#include "ink/backend/llvm_inkvm/inkvm_target.h"

#include "InkVM.h"

#include <llvm/MC/TargetRegistry.h>
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
} // namespace ink::backend::llvm_inkvm
