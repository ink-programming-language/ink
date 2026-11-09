#pragma once

#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic.h"

#include <expected>
#include <memory>
#include <string>
#include <string_view>

namespace ink::vm
{
  // Writes deterministic assembly text, preserving names, data, instructions and frame layout state.
  // Uses names directly when possible; generated labels carry an original name only when an alias is needed.
  // Numeric refN addresses retain their existing BaseReferenceIndex without resolving object symbols.
  [[nodiscard]] std::expected<std::string, core::Diagnostic> writeMachineModuleText(const MachineModule &ModuleValue);

  // Returns an independent owning module. No input text or temporary parser storage is borrowed.
  // Malformed input returns a diagnostic with byte offsets into Text; callers may attach a SourceId.
  [[nodiscard]] std::expected<std::unique_ptr<MachineModule>, core::Diagnostic> readMachineModuleText(std::string_view Text);
} // namespace ink::vm
