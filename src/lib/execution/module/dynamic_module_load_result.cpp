#include "ink/execution/module/dynamic_module_load_result.h"

namespace ink::execution
{
  bool DynamicModuleLoadResult::succeeded() const noexcept
  {
    return Module.valid() && Diagnostics.empty();
  }

  ModuleId DynamicModuleLoadResult::moduleId() const noexcept
  {
    return Module;
  }

  const std::vector<core::Diagnostic> &DynamicModuleLoadResult::diagnostics() const noexcept
  {
    return Diagnostics;
  }
} // namespace ink::execution
