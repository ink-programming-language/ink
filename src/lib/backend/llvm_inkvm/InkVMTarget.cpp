#include "ink/backend/llvm_inkvm/inkvm_target.h"

#include "InkVM.h"
#include "InkVMMachineModuleEmitter.h"
#include "InkVMTargetMachine.h"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Triple.h>

#include "ink/backend/vm/serialization/machine_module_text.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"

#include <expected>
#include <memory>
#include <string>
#include <utility>

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

    std::expected<std::unique_ptr<vm::MachineModule>, std::string> emitInkVMMachineModule(llvm::Module& Module, llvm::CodeGenOptLevel OptLevel)
    {
        std::string ErrorMessage;
        const auto TM = createInkVMTargetMachine(OptLevel, ErrorMessage);
        if (TM == nullptr)
        {
            return std::unexpected(std::move(ErrorMessage));
        }

        auto* InkTM = static_cast<llvm::InkVMTargetMachine*>(TM.get());
        Module.setDataLayout(InkTM->createDataLayout());
        Module.setTargetTriple(InkTM->getTargetTriple());

        llvm::InkVMMachineModuleEmitSink Sink;
        llvm::legacy::PassManager PassManager;
        if (InkTM->addPassesToEmitMachineModule(PassManager, Sink))
        {
            return std::unexpected(Sink.ErrorMessage.empty() ? std::string("InkVM TargetMachine cannot emit MachineModule") : std::move(Sink.ErrorMessage));
        }

        PassManager.run(Module);
        if (Sink.Failed || Sink.Output == nullptr)
        {
            return std::unexpected(Sink.ErrorMessage.empty() ? std::string("InkVM MachineModule emission failed") : std::move(Sink.ErrorMessage));
        }

        return std::move(Sink.Output);
    }

    bool emitInkVMAssembly(llvm::Module& Module, std::string& AssemblyOutput, std::string& ErrorMessage, llvm::CodeGenOptLevel OptLevel)
    {
        AssemblyOutput.clear();
        ErrorMessage.clear();

        auto MachineModuleResult = emitInkVMMachineModule(Module, OptLevel);
        if (!MachineModuleResult)
        {
            ErrorMessage = std::move(MachineModuleResult.error());
            return false;
        }

        auto TextResult = vm::writeMachineModuleText(**MachineModuleResult);
        if (!TextResult)
        {
            ErrorMessage = core::DiagnosticFormatter{}.format(TextResult.error()).Message;
            return false;
        }

        AssemblyOutput = std::move(*TextResult);
        return true;
    }
} // namespace ink::backend::llvm_inkvm
