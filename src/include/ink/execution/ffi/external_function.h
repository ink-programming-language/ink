#ifndef INK_EXECUTION_FFI_EXTERNAL_FUNCTION_H
#define INK_EXECUTION_FFI_EXTERNAL_FUNCTION_H

#include "ink/execution/memory/execution_heap.h"

#include <span>

namespace ink::ir
{
  class Function;
} // namespace ink::ir

namespace ink::execution
{
  class NativeSymbolCache;

  // Resolves the exact declared symbol in the current process and invokes it
  // through libffi using the declared C signature. No libraries are loaded.
  // The caller owns the frame and symbol cache, and keeps cached modules loaded.
  // Platform lookup and ABI marshalling are separate.
  ExecutionValueResult callExternalFunction(ExecutionHeap &Heap, NativeSymbolCache &Symbols, const ir::Function &Function, std::span<const ExecutionValueRef> Arguments);
} // namespace ink::execution

#endif
