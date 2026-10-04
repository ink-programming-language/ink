#ifndef INK_EXECUTION_MEMORY_EXECUTION_STORAGE_H
#define INK_EXECUTION_MEMORY_EXECUTION_STORAGE_H

#include "ink/execution/runtime/runtime_value.h"
#include "ink/execution/runtime/runtime_bytes.h"
#include "ink/execution/support/execution_object.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <new>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::execution
{
  // Read and write the common Ink object representation, including native FFI
  // addresses. The caller supplies a valid address and a complete type layout.
  ExecutionStatus validateStorageValue(const TypeDesc &Layout, const RuntimeValue &Value);
  RuntimeValueResult readStorage(const TypeDesc &Layout, const void *Address);
  ExecutionStatus writeStorage(const TypeDesc &Layout, void *Address, const RuntimeValue &Value);

  // Storage is owned exclusively by ExecutionMemoryManager. Borrowed pointers obtained
  // from a storage reference are valid only until its explicit release.
  class ExecutionStorage : public ExecutionObject
  {
    protected:
      explicit ExecutionStorage(ExecutionObjectKind Kind) noexcept
          : ExecutionObject(Kind)
      {
      }
  };

  class ExecutionCell final : public ExecutionStorage
  {
    public:
      RuntimeTypeId type() const noexcept
      {
        return Layout.Type;
      }

      const TypeDesc &layout() const noexcept
      {
        return Layout;
      }

      bool initialized() const noexcept
      {
        return Initialized;
      }

      void *data() noexcept;
      const void *data() const noexcept;
      std::size_t size() const noexcept;

      bool writable() const noexcept
      {
        return Writable;
      }

      bool runtime() const noexcept
      {
        return Runtime;
      }

      RuntimeValueResult loadRuntime() const;
      ExecutionStatus storeRuntime(const RuntimeValue &Value);
      const TypeDesc *elementLayout(std::size_t Offset, RuntimeTypeId Type, bool AllowOnePast = false) const noexcept;
      RuntimeValueResult loadElement(std::size_t Offset, RuntimeTypeId Type) const;
      ExecutionStatus storeElement(std::size_t Offset, const RuntimeValue &Value);
      ExecutionStatus loadBits(std::uint64_t &Bits) const noexcept;
      ExecutionStatus storeBits(std::uint64_t Bits) noexcept;
      ExecutionStatus loadBits(std::uint64_t &Bits, std::size_t Width) const noexcept;
      ExecutionStatus storeBits(std::uint64_t Bits, std::size_t Width) noexcept;
      ExecutionStatus loadBytes(std::size_t Offset, const TypeDesc &Element, void *Destination) const;
      ExecutionStatus storeBytes(std::size_t Offset, const TypeDesc &Element, const void *Source);

    private:
      ExecutionCell(ExecutionMemoryManager &Owner, const TypeDesc &Layout, bool Writable, bool Runtime);
      bool isInitialized(std::size_t Offset, const TypeDesc &Element) const noexcept;
      void markInitialized(std::size_t Offset, const TypeDesc &Element);

      TypeDesc Layout;
      ExecutionMemoryManager &Owner;
      RuntimeBytes Bytes{nullptr, RuntimeBytesDeleter{}};
      struct InitializedRange
      {
          std::size_t Start;
          std::size_t End;
      };
      std::vector<InitializedRange> InitializedRanges;
      bool Writable;
      bool Runtime;
      bool Initialized = false;

      friend class ExecutionHeap;
      friend class ExecutionMemoryManager;
      friend class ExecutionMachine;
  };

  // Fixed byte storage includes the optional trailing NUL byte. It never moves
  // while allocated; a language pointer contains only its non-owning address.
  class ExecutionBuffer final : public ExecutionStorage
  {
    public:
      char *data() noexcept
      {
        return Bytes.data();
      }

      const char *data() const noexcept
      {
        return Bytes.data();
      }

      std::size_t size() const noexcept
      {
        return Bytes.size();
      }

    private:
      explicit ExecutionBuffer(std::size_t Size)
          : ExecutionStorage(ExecutionObjectKind::Buffer),
            Bytes(Size)
      {
      }

      ExecutionBuffer(std::string_view Source, bool Terminate)
          : ExecutionStorage(ExecutionObjectKind::Buffer),
            Bytes(Source.size() + static_cast<std::size_t>(Terminate))
      {
        if (!Source.empty())
        {
          std::copy(Source.begin(), Source.end(), Bytes.begin());
        }
      }

      std::vector<char> Bytes;

      friend class ExecutionMemoryManager;
  };
} // namespace ink::execution

#endif
