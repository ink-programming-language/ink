#ifndef INK_EXECUTION_FFI_FFI_CALL_H
#define INK_EXECUTION_FFI_FFI_CALL_H

#include "ink/execution/memory/execution_heap.h"
#include "ink/execution/ffi/native_symbol.h"
#include "ink/execution/runtime/runtime_value.h"

#include <span>

namespace ink::ir
{
  class Function;
} // namespace ink::ir

namespace ink::execution
{
  // Calls a resolved address once using the function's declared native C ABI.
  // Accepts evaluated values and preserves pointer provenance for owned buffers.
  // Each string receives independent writable, NUL-terminated storage; a pointer
  // returned into that storage promotes it to the heap lifetime without changing
  // the source value. The returned handle does not own or keep the storage alive.
  // Unsupported signatures and invalid arguments are rejected before invocation.
  ExecutionValueResult callWithLibffi(ExecutionHeap &Heap, NativeSymbol Symbol, const ir::Function &Function, std::span<const ExecutionValueRef> Arguments);
  RuntimeValueResult callWithLibffi(ExecutionMemoryManager &Memory, const RuntimeTypeTable &Types, NativeSymbol Symbol, const RuntimeFunctionDescriptor &Function, std::span<const RuntimeValue> Arguments);
} // namespace ink::execution

#endif
