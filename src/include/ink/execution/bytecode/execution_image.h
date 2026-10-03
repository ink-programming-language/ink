#ifndef INK_EXECUTION_BYTECODE_EXECUTION_IMAGE_H
#define INK_EXECUTION_BYTECODE_EXECUTION_IMAGE_H

#include "ink/execution/bytecode/executable_function.h"

#include <memory>
#include <unordered_map>

namespace ink::execution
{
  // Runtime identities and owned code stay stable while an invocation is active.
  // Every function in the image shares this one lowered type domain.
  struct ExecutionImage
  {
      std::shared_ptr<const RuntimeTypeTable> Layouts;
      std::unordered_map<FunctionId, RuntimeFunctionDescriptor> Descriptors;
      std::unordered_map<FunctionId, std::unique_ptr<ExecutableFunction>> Functions;
  };
} // namespace ink::execution

#endif
