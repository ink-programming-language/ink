#include "ink/execution/memory/execution_pointer.h"

#include "ink/execution/memory/execution_storage.h"

#include <utility>

namespace ink::execution
{
  ExecutionPointer ExecutionPointer::fromPlace(ExecutionPlace Place, std::size_t Offset) noexcept
  {
    ExecutionPointer Result;
    Result.PointerKind = Kind::Place;
    Result.Place = std::move(Place);
    Result.Offset = Offset;
    return Result;
  }

  ExecutionPointer ExecutionPointer::fromBuffer(ExecutionStorageRef Buffer, std::size_t Offset) noexcept
  {
    ExecutionPointer Result;
    Result.PointerKind = Kind::Buffer;
    Result.Buffer = std::move(Buffer);
    Result.Offset = Offset;
    return Result;
  }

  ExecutionPointer ExecutionPointer::fromNative(void *Address) noexcept
  {
    ExecutionPointer Result;
    Result.PointerKind = Address ? Kind::Native : Kind::Null;
    Result.NativeAddress = Address;
    return Result;
  }

  ExecutionBuffer *ExecutionPointer::buffer() const noexcept
  {
    return PointerKind == Kind::Buffer ? Buffer.buffer() : nullptr;
  }

  void *ExecutionPointer::address() const noexcept
  {
    if (PointerKind == Kind::Native)
    {
      return NativeAddress;
    }
    if (PointerKind == Kind::Buffer && valid())
    {
      char *Start = buffer()->data();
      return Offset == 0 ? Start : Start + Offset;
    }
    if (PointerKind == Kind::Place && valid())
    {
      void *Data = Place.storage().cell()->data();
      return Data && Offset != 0 ? static_cast<unsigned char *>(Data) + Offset : Data;
    }
    return nullptr;
  }

  ExecutionStatus ExecutionPointer::status() const noexcept
  {
    switch (PointerKind)
    {
    case Kind::Null:
      return ExecutionStatus::Success;
    case Kind::Place:
    {
      const ExecutionStatus Status = Place.storage().status();
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      const ExecutionCell *Cell = Place.storage().cell();
      return Cell && Offset <= Cell->size() ? ExecutionStatus::Success : ExecutionStatus::InvalidPlace;
    }
    case Kind::Buffer:
    {
      const ExecutionStatus Status = Buffer.status();
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      const ExecutionBuffer *Storage = Buffer.buffer();
      return Storage && Offset <= Storage->size() ? ExecutionStatus::Success : ExecutionStatus::InvalidPlace;
    }
    case Kind::Native:
      return NativeAddress ? ExecutionStatus::Success : ExecutionStatus::InvalidPlace;
    }
    return ExecutionStatus::InvalidPlace;
  }
} // namespace ink::execution
