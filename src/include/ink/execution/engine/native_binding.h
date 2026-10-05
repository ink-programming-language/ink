#ifndef INK_EXECUTION_ENGINE_NATIVE_BINDING_H
#define INK_EXECUTION_ENGINE_NATIVE_BINDING_H

#include "ink/execution/support/execution_result.h"

namespace ink::execution
{
  // Process-local bindings never enter a bytecode archive. Arguments and results
  // use the signature's object layout; the owner keeps Context alive during calls.
  struct NativeBinding
  {
      using Callback = ExecutionStatus (*)(void *Context, void *Result, const void *const *Arguments);
      Callback Invoke = nullptr;
      void *Context = nullptr;
  };
} // namespace ink::execution

#endif
