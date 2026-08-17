#ifndef INK_EXECUTION_DYNAMIC_MODULE_LOAD_RESULT_H
#define INK_EXECUTION_DYNAMIC_MODULE_LOAD_RESULT_H

#include "ink/core/diagnostic.h"
#include "ink/execution/module/module_id.h"

#include <vector>

namespace ink::execution
{
  class ExecutionEngine;

  class DynamicModuleLoadResult
  {
    public:
      bool succeeded() const noexcept;
      ModuleId moduleId() const noexcept;
      const std::vector<core::Diagnostic> &diagnostics() const noexcept;

    private:
      ModuleId Module;
      std::vector<core::Diagnostic> Diagnostics;

      friend class ExecutionEngine;
  };
} // namespace ink::execution

#endif
