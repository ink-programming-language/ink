#include "ink/execution/module/dynamic_module_load_result.h"

namespace ink::execution
{
  bool DynamicModuleLoadResult::succeeded() const noexcept
  {
    return Module.valid();
  }

  ModuleId DynamicModuleLoadResult::moduleId() const noexcept
  {
    return Module;
  }

} // namespace ink::execution
