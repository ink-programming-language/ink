#pragma once

#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/name/name.h"

#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>

namespace llvm
{
    class DataLayout;
    class GlobalVariable;
    class MachineFunction;
    class Module;

    struct InkVMMachineModuleEmitSink
    {
            std::unique_ptr<ink::vm::MachineModule> Output;
            std::string ErrorMessage;
            bool Failed = false;
    };

    // Lowers an LLVM Module and its post-RA/post-PEI MachineFunctions directly into an owning ink::vm::MachineModule.
    class InkVMMachineModuleBuilder final
    {
        public:
            InkVMMachineModuleBuilder() = default;

            bool initialize(const Module& IRModule);
            bool emitFunction(const MachineFunction& MF);
            [[nodiscard]] std::expected<std::unique_ptr<ink::vm::MachineModule>, std::string> takeModule();

            bool hasFailed() const noexcept
            {
                return Failed;
            }

            const std::string& errorMessage() const noexcept
            {
                return ErrorMessage;
            }

            void fail(std::string Message);

            ink::core::Name internName(std::string_view Text);
            bool recordExternalSymbolDeclaration(ink::core::Name SymbolName);

        private:
            bool error(std::string Message);
            bool emitExternalDeclarations(const Module& IRModule);
            bool emitDataGlobals(const Module& IRModule);
            bool emitDataGlobal(const GlobalVariable& Global, const DataLayout& DL);
            void recordDefinedFunctionSymbols(const Module& IRModule);

            std::unique_ptr<ink::vm::MachineModule> Output;
            std::unordered_set<ink::core::Name> KnownSymbols;
            std::string ErrorMessage;
            bool Failed = false;
    };
} // namespace llvm
