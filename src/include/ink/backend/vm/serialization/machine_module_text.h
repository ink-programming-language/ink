#pragma once

#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic.h"

#include <expected>
#include <memory>
#include <string>
#include <string_view>

namespace ink::vm
{
    // Appends deterministic canonical VM assembly text (see docs/VM-Text-Format.md) to Output,
    // preserving module metadata, declarations, global data/relocations, frame layout, and instructions.
    // Malformed modules fail before mutating Output and return a VMModuleInvalid diagnostic with an invalid SourceRange (Span.isValid() == false).
    [[nodiscard]] std::expected<void, core::Diagnostic> writeMachineModuleText(const MachineModule& ModuleValue, std::string& Output);

    // Writes deterministic canonical VM assembly text (see docs/VM-Text-Format.md), preserving
    // module metadata, declarations, global data/relocations, frame layout, and instructions.
    // Malformed modules return a VMModuleInvalid diagnostic with an invalid SourceRange (Span.isValid() == false).
    [[nodiscard]] std::expected<std::string, core::Diagnostic> writeMachineModuleText(const MachineModule& ModuleValue);

    // Parses VM assembly text into an independent owning MachineModule without borrowing Text.
    // Malformed input returns a VMTextInvalid diagnostic with byte offsets into Text; callers may attach a SourceId.
    [[nodiscard]] std::expected<std::unique_ptr<MachineModule>, core::Diagnostic> readMachineModuleText(std::string_view Text);
} // namespace ink::vm
