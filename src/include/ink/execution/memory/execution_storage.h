#ifndef INK_EXECUTION_MEMORY_EXECUTION_STORAGE_H
#define INK_EXECUTION_MEMORY_EXECUTION_STORAGE_H

#include "ink/execution/runtime/runtime_value.h"
#include "ink/execution/support/execution_object.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::execution
{
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

      const StorageLayout &layout() const noexcept
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
      const StorageLayout *elementLayout(std::size_t Offset, RuntimeTypeId Type, bool AllowOnePast = false) const noexcept;
      RuntimeValueResult loadElement(std::size_t Offset, RuntimeTypeId Type) const;
      ExecutionStatus storeElement(std::size_t Offset, const RuntimeValue &Value);
      ExecutionStatus loadBits(std::uint64_t &Bits) const noexcept;
      ExecutionStatus storeBits(std::uint64_t Bits) noexcept;
      ExecutionStatus loadBits(std::uint64_t &Bits, std::size_t Width) const noexcept;
      ExecutionStatus storeBits(std::uint64_t Bits, std::size_t Width) noexcept;

    private:
      enum class NativeKind
      {
        None,
        Boolean,
        Signed8,
        Unsigned8,
        Signed16,
        Unsigned16,
        Signed32,
        Unsigned32,
        Signed64,
        Unsigned64,
        Float32,
        Float64,
      };

      union NativeScalar
      {
          bool Boolean;
          std::int8_t Signed8;
          std::uint8_t Unsigned8;
          std::int16_t Signed16;
          std::uint16_t Unsigned16;
          std::int32_t Signed32;
          std::uint32_t Unsigned32;
          std::int64_t Signed64;
          std::uint64_t Unsigned64 = 0;
          float Float32;
          double Float64;
      };

      ExecutionCell(const StorageLayout &Layout, bool Writable, bool Runtime);
      void initializeNative() noexcept;

      StorageLayout Layout;
      // Only types without native storage retain a value object.
      RuntimeValue Value;
      NativeScalar Native;
      std::unique_ptr<std::byte[]> NativeArray;
      NativeKind NativeType = NativeKind::None;
      std::size_t NativeSize = 0;
      bool Writable;
      bool Runtime;
      bool Initialized = false;

      friend class ExecutionHeap;
      friend class ExecutionMemoryManager;
      friend class ExecutionMachine;
  };

  // Fixed byte storage includes the optional trailing NUL byte. It never moves
  // while allocated; a language pointer contains only its non-owning identity.
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
