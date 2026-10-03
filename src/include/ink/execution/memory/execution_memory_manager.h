#ifndef INK_EXECUTION_MEMORY_EXECUTION_MEMORY_MANAGER_H
#define INK_EXECUTION_MEMORY_EXECUTION_MEMORY_MANAGER_H

#include "ink/execution/memory/execution_storage.h"

#include <limits>
#include <memory>

namespace ink::execution
{
  // Owns every mutable allocation and its identity independently of value
  // snapshots. Counts and byte budgets are cumulative and are not refunded.
  class ExecutionMemoryManager final
  {
    public:
      explicit ExecutionMemoryManager(std::size_t MaxStorage = std::numeric_limits<std::size_t>::max(), std::size_t MaxBytes = std::numeric_limits<std::size_t>::max());
      ~ExecutionMemoryManager();
      ExecutionMemoryManager(const ExecutionMemoryManager &) = delete;
      ExecutionMemoryManager &operator=(const ExecutionMemoryManager &) = delete;
      ExecutionMemoryManager(ExecutionMemoryManager &&) = delete;
      ExecutionMemoryManager &operator=(ExecutionMemoryManager &&) = delete;

      ExecutionStatus lastStatus() const noexcept;
      std::size_t liveStorageCount() const noexcept;
      std::size_t allocatedStorageCount() const noexcept;
      // Bytes include storage objects and their directly owned byte buffers.
      // Immutable value snapshots are accounted for separately by ExecutionHeap.
      std::size_t liveStorageBytes() const noexcept;
      std::size_t allocatedStorageBytes() const noexcept;

      // Initial and subsequent stored values use Layout's type domain; callers
      // must bridge values arriving from an independent execution image.
      ExecutionPlaceResult allocateCell(const StorageLayout &Layout, bool Writable = true, const RuntimeValue &Initial = {}, bool Runtime = false);
      ExecutionStatus chargeLocalAllocation() noexcept;
      ExecutionStatus validatePointer(const ExecutionPointer &Pointer) const noexcept;
      ExecutionStorageRef allocateBuffer(std::string_view Bytes, bool Terminate = true);
      ExecutionStorageRef allocateBuffer(std::size_t Size);
      bool owns(const ExecutionStorageRef &Storage) const noexcept;
      ExecutionStatus release(const ExecutionStorageRef &Storage) noexcept;
      ExecutionStatus release(ExecutionPlace Place) noexcept;
      ExecutionPointer pointerFromAddress(void *Address) const noexcept;

    private:
      bool checkBudget(std::size_t Bytes) noexcept;
      ExecutionStorageRef ownStorage(std::unique_ptr<ExecutionStorage> Storage, std::size_t Bytes);

      std::shared_ptr<ExecutionMemoryState> State;
      ExecutionStatus LastStatus = ExecutionStatus::Success;
  };
} // namespace ink::execution

#endif
