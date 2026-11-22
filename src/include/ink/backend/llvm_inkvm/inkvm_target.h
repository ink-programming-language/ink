#pragma once

#include "ink/backend/vm/module/machine_module.h"

#include <llvm/Support/CodeGen.h>

#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <string_view>

namespace llvm
{
    class Module;
    class TargetMachine;
} // namespace llvm

namespace ink::backend::llvm_inkvm
{
    inline constexpr std::string_view InkVMTargetName = "inkvm";
    inline constexpr std::string_view InkVMDefaultTriple = "inkvm-unknown-unknown";
    inline constexpr std::string_view InkVMDataLayout = "e-m:e-p:64:64-i64:64-f64:64-n64-S64";

    inline constexpr std::uint64_t InkVMTSFlagOpcodeMask = 0xFFFFULL;
    inline constexpr std::uint64_t InkVMTSFlagHasVMOpcode = std::uint64_t{1} << 16;

    constexpr bool hasEncodedVMOpcode(std::uint64_t TSFlags) noexcept
    {
        return (TSFlags & InkVMTSFlagHasVMOpcode) != 0;
    }

    constexpr std::uint16_t decodeVMOpcodeFromTSFlags(std::uint64_t TSFlags) noexcept
    {
        return static_cast<std::uint16_t>(TSFlags & InkVMTSFlagOpcodeMask);
    }

    void initializeInkVMTarget() noexcept;

    std::unique_ptr<llvm::TargetMachine> createInkVMTargetMachine(llvm::CodeGenOptLevel OptLevel, std::string& ErrorMessage);

    [[nodiscard]] std::expected<std::unique_ptr<vm::MachineModule>, std::string> emitInkVMMachineModule(llvm::Module& Module, llvm::CodeGenOptLevel OptLevel = llvm::CodeGenOptLevel::Default);

    bool emitInkVMAssembly(llvm::Module& Module, std::string& AssemblyOutput, std::string& ErrorMessage, llvm::CodeGenOptLevel OptLevel = llvm::CodeGenOptLevel::Default);
} // namespace ink::backend::llvm_inkvm
