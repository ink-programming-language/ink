#pragma once

namespace ink::core
{
    class DiagnosticEngine;
} // namespace ink::core

namespace ink::vm
{
    class MachineModule;

    // Non-fatal structural and semantic verification of a standalone MachineModule.
    // Validates name pool membership, declaration/definition metadata, global data and relocations,
    // stack slot alignment, block ownership, register encodings, and shared instruction constraints.
    // Reports every discovered violation through Diagnostics as DiagnosticKind::VMModuleInvalid and returns false on failure.
    [[nodiscard]] bool verifyMachineModule(const MachineModule& ModuleValue, const core::DiagnosticEngine& Diagnostics);
} // namespace ink::vm
