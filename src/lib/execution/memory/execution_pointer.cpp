#include "ink/execution/memory/execution_pointer.h"

#include "ink/execution/memory/execution_storage.h"

namespace ink::execution
{
  ExecutionPointer ExecutionPointer::fromPlace(ExecutionPlace Place, std::size_t Offset) noexcept
  {
    const ExecutionCell *Cell = Place.storage().cell();
    return Cell ? fromNative(const_cast<void *>(Cell->data())).offsetBy(Offset) : ExecutionPointer{};
  }

  ExecutionPointer ExecutionPointer::fromBuffer(ExecutionStorageRef Buffer, std::size_t Offset) noexcept
  {
    ExecutionBuffer *Storage = Buffer.buffer();
    return Storage ? fromNative(Storage->data()).offsetBy(Offset) : ExecutionPointer{};
  }

  ExecutionPointer ExecutionPointer::fromNative(void *Address) noexcept
  {
    ExecutionPointer Result;
    Result.Address = Address;
    return Result;
  }
} // namespace ink::execution
