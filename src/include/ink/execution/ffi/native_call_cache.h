#ifndef INK_EXECUTION_FFI_NATIVE_CALL_CACHE_H
#define INK_EXECUTION_FFI_NATIVE_CALL_CACHE_H

#include "ink/execution/memory/execution_heap.h"
#include "ink/execution/runtime/runtime_value.h"

#include <cstddef>
#include <memory>
#include <span>

namespace ink::ir
{
  class Function;
} // namespace ink::ir

namespace ink::execution
{
  class NativeSymbolCache;

  // Caches native addresses and prepared ABI signatures for runtime functions.
  // Plans are prepared only when invoked; unresolved symbols remain retryable.
  // Argument buffers belong to each invocation, including recursive calls.
  // The owner keeps native modules loaded and clears plans whenever symbol
  // bindings or execution images change. Instances are single-threaded.
  class NativeCallCache final
  {
    public:
      NativeCallCache();
      ~NativeCallCache();

      NativeCallCache(const NativeCallCache &) = delete;
      NativeCallCache &operator=(const NativeCallCache &) = delete;

      ExecutionValueResult invoke(ExecutionHeap &Heap, NativeSymbolCache &Symbols, const ir::Function &Function, std::span<const ExecutionValueRef> Arguments);
      RuntimeValueResult invoke(ExecutionMemoryManager &Memory, const RuntimeTypeTable &Types, NativeSymbolCache &Symbols, const RuntimeFunctionDescriptor &Function, std::span<const RuntimeValue> Arguments);
      // Active calls retain their prepared interfaces through reentrant clearing.
      void clear() noexcept;
      std::size_t size() const noexcept;

    private:
      struct Impl;

      std::unique_ptr<Impl> Implementation;
  };
} // namespace ink::execution

#endif
