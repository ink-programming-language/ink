#pragma once

#include "ink/backend/vm/module/machine_module.h"

#include <cstdint>
#include <string_view>
#include <vector>

namespace llvm
{
    class Constant;
    class DataLayout;
    class GlobalVariable;
    class InkVMMachineModuleBuilder;

    bool isInkVMMetadataGlobal(const GlobalVariable& Global);
    std::string_view formatInkVMGlobalDataError(ink::vm::MachineModuleDataError Error) noexcept;
    bool lowerInkVMConstantData(const Constant* Initializer, const DataLayout& DL, InkVMMachineModuleBuilder& ModuleBuilder, std::vector<std::uint8_t>& OutBytes, std::vector<ink::vm::MachineGlobalRelocation>& OutRelocations);
} // namespace llvm
