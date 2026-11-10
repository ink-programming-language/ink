#pragma once

#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <spdlog/fmt/fmt.h>

#include <cstddef>
#include <string_view>

namespace ink::vm::detail
{
  inline bool reportLinkFailure(const core::DiagnosticEngine &Diagnostics, std::size_t ModuleIndex, const MachineModule *Module, std::string_view Description)
  {
    const auto Context = Module ? fmt::format("input #{} ('{}')", ModuleIndex, core::NamePool::instance().text(Module->name())) : fmt::format("input #{}", ModuleIndex);
    Diagnostics.report<core::DiagnosticKind::VMModuleLinkFailed>({}, Context, Description);
    return false;
  }
} // namespace ink::vm::detail
