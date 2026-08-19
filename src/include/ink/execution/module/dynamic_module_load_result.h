#ifndef INK_EXECUTION_DYNAMIC_MODULE_LOAD_RESULT_H
#define INK_EXECUTION_DYNAMIC_MODULE_LOAD_RESULT_H

#include "ink/execution/module/module_id.h"

namespace ink::execution
{
  class ExecutionEngine;

  class DynamicModuleLoadResult
  {
    public:
      bool succeeded() const noexcept;
      ModuleId moduleId() const noexcept;

    private:
      ModuleId Module;

      friend class ExecutionEngine;
  };
} // namespace ink::execution

#endif
