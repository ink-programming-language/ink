#ifndef INK_EXECUTION_MEMORY_EXECUTION_POINTER_H
#define INK_EXECUTION_MEMORY_EXECUTION_POINTER_H

#include "ink/execution/support/execution_result.h"
#include "ink/execution/memory/execution_storage_ref.h"

#include <cstddef>
#include <cstdint>

namespace ink::execution
{
  // A language pointer is one non-owning machine address. Allocation identities
  // belong to the interpreter's bookkeeping, never to pointer values.
  class ExecutionPointer final
  {
    public:
      enum class Kind
      {
        Null,
        Native,
      };

      static ExecutionPointer fromPlace(ExecutionPlace Place, std::size_t Offset = 0) noexcept;
      static ExecutionPointer fromBuffer(ExecutionStorageRef Buffer, std::size_t Offset = 0) noexcept;
      static ExecutionPointer fromNative(void *Address) noexcept;

      Kind kind() const noexcept
      {
        return Address ? Kind::Native : Kind::Null;
      }

      void *address() const noexcept
      {
        return Address;
      }

      ExecutionPointer offsetBy(std::size_t Bytes) const noexcept
      {
        return fromNative(reinterpret_cast<void *>(reinterpret_cast<std::uintptr_t>(Address) + Bytes));
      }

      // Copying a pointer never reads or validates its pointee.
      ExecutionStatus status() const noexcept
      {
        return ExecutionStatus::Success;
      }

      bool valid() const noexcept
      {
        return true;
      }

    private:
      void *Address = nullptr;
  };

  static_assert(sizeof(ExecutionPointer) == sizeof(void *));
} // namespace ink::execution

#endif
