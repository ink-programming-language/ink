#ifndef INK_EXECUTION_MEMORY_EXECUTION_HEAP_H
#define INK_EXECUTION_MEMORY_EXECUTION_HEAP_H

#include "ink/execution/memory/execution_memory_manager.h"
#include "ink/execution/bridge/semantic_value_bridge.h"

#include <limits>
#include <memory>

namespace ink::execution
{
  struct ExecutionHeapState;

  // One allocation boundary for immutable values and explicitly owned storage.
  // Values are released with their last C++ RAII handle. Cells and buffers are
  // released explicitly or when this heap ends; language pointers never own them.
  // There is no tracing collector. The IR context must outlive retained values.
  class ExecutionHeap final
  {
    public:
      explicit ExecutionHeap(ir::IRContext &Context, std::size_t MaxStorage = std::numeric_limits<std::size_t>::max(), std::size_t MaxStorageBytes = std::numeric_limits<std::size_t>::max());
      ~ExecutionHeap();
      ExecutionHeap(const ExecutionHeap &) = delete;
      ExecutionHeap &operator=(const ExecutionHeap &) = delete;
      ExecutionHeap(ExecutionHeap &&) = delete;
      ExecutionHeap &operator=(ExecutionHeap &&) = delete;

      ir::IRContext &context() const noexcept;
      ExecutionStatus lastStatus() const noexcept;
      std::size_t liveValueCount() const noexcept;
      std::size_t liveStorageCount() const noexcept;
      // Cumulative allocation budget, including released storage.
      std::size_t allocatedStorageCount() const noexcept;
      std::size_t liveStorageBytes() const noexcept;
      std::size_t allocatedStorageBytes() const noexcept;
      ExecutionMemoryManager &memoryManager() noexcept;
      const ExecutionMemoryManager &memoryManager() const noexcept;
      SemanticValueBridge &bridge() noexcept;
      const SemanticValueBridge &bridge() const noexcept;

      ExecutionValueRef fromConstant(const ir::Constant &Value);
      ExecutionValueRef boolean(const ir::Type &Type, bool Value);
      ExecutionValueRef integer(const ir::Type &Type, ExecutionInteger Value);
      ExecutionValueRef floating(const ir::Type &Type, ir::FloatBits Value);
      ExecutionValueRef string(const ir::Type &Type, std::string_view Value);
      ExecutionValueRef pointer(const ir::Type &Type, ExecutionPointer Value);
      ExecutionValueRef function(const ir::Function &Value);
      ExecutionValueRef voidValue(const ir::Type &Type);

      ExecutionPlaceResult allocateCell(const ir::Type &Type, bool Writable = true, const ExecutionValueRef &Initial = {}, bool Runtime = false);
      ExecutionStorageRef allocateBuffer(std::string_view Bytes, bool Terminate = true);
      ExecutionStorageRef allocateBuffer(std::size_t Size);
      bool owns(const ExecutionStorageRef &Storage) const noexcept;
      ExecutionStatus release(const ExecutionStorageRef &Storage) noexcept;
      ExecutionStatus release(ExecutionPlace Place) noexcept;
      ExecutionValueResult load(ExecutionPlace Place);
      ExecutionStatus store(ExecutionPlace Place, const ExecutionValueRef &Value);
      ExecutionPointer pointerFromAddress(void *Address) const noexcept;

    private:
      ExecutionValueRef ownValue(std::unique_ptr<ExecutionValue> Value, bool AllowExpiredPointer = false);
      ExecutionValueRef pointerSnapshot(const ir::Type &Type, ExecutionPointer Value);
      ExecutionStatus validatePlace(ExecutionPlace Place) const noexcept;

      ir::IRContext &Context;
      std::shared_ptr<ExecutionHeapState> State;
      SemanticValueBridge Bridge;
      ExecutionMemoryManager Memory;
      ExecutionStatus LastStatus = ExecutionStatus::Success;

      friend class SemanticValueBridge;
  };
} // namespace ink::execution

#endif
