#include "ink/backend/vm/linker/module_linker.h"

#include "link_bindings.h"
#include "linked_module_cloner.h"
#include "ink/core/name/name_pool.h"

namespace ink::vm
{
  std::unique_ptr<MachineModule> ModuleLinker::link(std::span<const MachineModule *const> Inputs, const core::DiagnosticEngine &Diagnostics, core::Name OutputName)
  {
    if (OutputName != core::NAME_None && !core::NamePool::instance().contains(OutputName))
    {
      Diagnostics.report<core::DiagnosticKind::VMModuleLinkFailed>({}, "output module", "module name is not present in the name pool");
      return nullptr;
    }
    const auto Bindings = detail::resolveLinkSymbols(Inputs, Diagnostics);
    if (!Bindings)
    {
      return nullptr;
    }
    return detail::cloneLinkedModule(Inputs, *Bindings, Diagnostics, OutputName);
  }
} // namespace ink::vm
