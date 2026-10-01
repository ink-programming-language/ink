#ifndef INK_EXECUTION_MEMORY_EXECUTION_POINTER_H
#define INK_EXECUTION_MEMORY_EXECUTION_POINTER_H

#include "ink/execution/support/execution_result.h"
#include "ink/execution/memory/execution_storage_ref.h"

#include <cstddef>

namespace ink::execution
{
  class ExecutionBuffer;

  // Managed pointers borrow heap storage and never extend its lifetime.
  // Native addresses are execution-only and never become IR integer constants.
  class ExecutionPointer final
  {
    public:
      enum class Kind
      {
        Null,
        Place,
        Buffer,
        Native,
      };

      static ExecutionPointer fromPlace(ExecutionPlace Place) noexcept;
      static ExecutionPointer fromBuffer(ExecutionStorageRef Buffer, std::size_t Offset = 0) noexcept;
      static ExecutionPointer fromNative(void *Address) noexcept;

      Kind kind() const noexcept
      {
        return PointerKind;
      }

      ExecutionPlace place() const noexcept
      {
        return Place;
      }

      ExecutionBuffer *buffer() const noexcept;

      const ExecutionStorageRef &bufferRef() const noexcept
      {
        return Buffer;
      }

      std::size_t offset() const noexcept
      {
        return Offset;
      }

      // Null and places have no host address. A valid one-past buffer pointer
      // may be passed or compared, but must not be dereferenced.
      void *address() const noexcept;
      ExecutionStatus status() const noexcept;

      bool valid() const noexcept
      {
        return status() == ExecutionStatus::Success;
      }

    private:
      Kind PointerKind = Kind::Null;
      ExecutionPlace Place;
      ExecutionStorageRef Buffer;
      std::size_t Offset = 0;
      void *NativeAddress = nullptr;
  };
} // namespace ink::execution

#endif
