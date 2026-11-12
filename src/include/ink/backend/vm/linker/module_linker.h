#pragma once

#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"

#include <memory>
#include <span>

namespace ink::vm
{
  // Resolves VM definitions and retains unmatched symbol declarations as native imports without laying out an executable image.
  class ModuleLinker final
  {
    public:
      // Inputs are borrowed only for this call. Success returns an independent owning module;
      // failure reports through Diagnostics and returns nullptr, leaving every input unchanged.
      // Local names and their instruction references are rewritten deterministically in the cloned output.
      [[nodiscard]] static std::unique_ptr<MachineModule> link(std::span<const MachineModule *const> Inputs, const core::DiagnosticEngine &Diagnostics, core::Name OutputName = core::NAME_None);
  };
} // namespace ink::vm
