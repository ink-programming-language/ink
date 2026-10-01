#ifndef INK_EXECUTION_MEMORY_EXECUTION_HEAP_H
#define INK_EXECUTION_MEMORY_EXECUTION_HEAP_H

#include "ink/execution/memory/execution_storage.h"

#include <limits>
#include <memory>

namespace ink::execution
{
  // One allocation boundary for immutable values and explicitly owned storage.
  // Values are released with their last C++ RAII handle. Cells and buffers are
  // released explicitly or when this heap ends; language pointers never own them.
  // There is no tracing collector. The IR context must outlive retained values.
  class ExecutionHeap final
  {
    public:
      explicit ExecutionHeap(ir::IRContext &Context, std::size_t MaxStorage = std::numeric_limits<std::size_t>::max());
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
      bool owns(const ExecutionStorageRef &Storage) const noexcept;
      ExecutionStatus release(const ExecutionStorageRef &Storage) noexcept;
      ExecutionStatus release(ExecutionPlace Place) noexcept;
      ExecutionValueResult load(ExecutionPlace Place);
      ExecutionStatus store(ExecutionPlace Place, const ExecutionValueRef &Value);

    private:
      ExecutionValueRef ownValue(std::unique_ptr<ExecutionValue> Value);
      ExecutionStorageRef ownStorage(std::unique_ptr<ExecutionStorage> Storage);
      ExecutionStatus validatePlace(ExecutionPlace Place) const noexcept;

      ir::IRContext &Context;
      std::shared_ptr<ExecutionHeapState> State;
      ExecutionStatus LastStatus = ExecutionStatus::Success;
  };
} // namespace ink::execution

#endif
