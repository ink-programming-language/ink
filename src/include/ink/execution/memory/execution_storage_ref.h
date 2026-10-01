#ifndef INK_EXECUTION_MEMORY_EXECUTION_STORAGE_REF_H
#define INK_EXECUTION_MEMORY_EXECUTION_STORAGE_REF_H

#include <cstddef>
#include <cstdint>
#include <memory>

namespace ink::execution
{
  enum class ExecutionStatus;
  class ExecutionHeap;
  struct ExecutionHeapState;
  class ExecutionStorage;
  class ExecutionCell;
  class ExecutionBuffer;

  // A non-owning allocation identity. The weak control block distinguishes heap
  // lifetimes even when an engine is reconstructed at the same host address.
  class ExecutionStorageRef final
  {
    public:
      ExecutionStorageRef() = default;
      bool valid() const noexcept;
      ExecutionStatus status() const noexcept;
      ExecutionStorage *get() const noexcept;
      ExecutionCell *cell() const noexcept;
      ExecutionBuffer *buffer() const noexcept;
      bool operator==(const ExecutionStorageRef &Other) const noexcept;

    private:
      ExecutionStorageRef(const std::shared_ptr<ExecutionHeapState> &State, std::size_t Slot, std::uint64_t Generation) noexcept
          : State(State),
            Slot(Slot),
            Generation(Generation)
      {
      }

      std::weak_ptr<ExecutionHeapState> State;
      std::size_t Slot = 0;
      std::uint64_t Generation = 0;

      friend class ExecutionHeap;
  };
} // namespace ink::execution

#endif
