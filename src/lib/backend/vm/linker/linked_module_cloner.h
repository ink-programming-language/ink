#pragma once

#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"

#include <memory>
#include <span>

namespace ink::vm::detail
{
  class LinkBindings;

  // Copies validated inputs using bindings resolved for the same input order, without resolving names again.
  // Success owns all output nodes; failure reports one diagnostic and discards the partially cloned module.
  std::unique_ptr<MachineModule> cloneLinkedModule(std::span<const MachineModule *const> Inputs, const LinkBindings &Bindings, const core::DiagnosticEngine &Diagnostics, core::Name OutputName);
} // namespace ink::vm::detail
