#ifndef INK_EXECUTION_FFI_FFI_ARGUMENT_H
#define INK_EXECUTION_FFI_FFI_ARGUMENT_H

#include "ink/execution/memory/execution_heap.h"
#include "ink/execution/ffi/ffi_value_layout.h"
#include "ink/execution/runtime/runtime_value.h"

#include <cstdint>

namespace ink::ir
{
  class Type;
  class Value;
} // namespace ink::ir

namespace ink::execution
{
  // Keeps one native argument stationary until its native call returns. The heap
  // must outlive this holder. Strings use independent heap-owned writable buffers;
  // unpromoted temporary buffers are released when the holder is reset or destroyed.
  class FfiArgument final
  {
    public:
      FfiArgument() = default;
      ~FfiArgument();
      FfiArgument(const FfiArgument &) = delete;
      FfiArgument &operator=(const FfiArgument &) = delete;
      FfiArgument(FfiArgument &&) = delete;
      FfiArgument &operator=(FfiArgument &&) = delete;

      // Unsupported types, mismatched values and values awaiting evaluation are
      // reported explicitly. Every attempt invalidates the previous address;
      // a failed preparation leaves address() null.
      ExecutionStatus prepare(ExecutionHeap &Heap, const ir::Type &Type, const ExecutionValueRef &Value);
      // Constants adapt to execution values; other IR values require evaluation.
      ExecutionStatus prepare(ExecutionHeap &Heap, const ir::Type &Type, const ir::Value &Value);
      ExecutionStatus prepare(ExecutionMemoryManager &Memory, const FfiValueLayout &Layout, const RuntimeValue &Value);

      void *address() noexcept
      {
        return Address;
      }

      ExecutionBuffer *buffer() const noexcept
      {
        return Buffer.buffer();
      }

      const ExecutionStorageRef &bufferRef() const noexcept
      {
        return Buffer;
      }

      // A returned alias transfers cleanup of a temporary string buffer to the
      // heap's lifetime. Existing storage keeps its original lifetime unchanged.
      void promoteBuffer() noexcept;

    private:
      union NativeScalar
      {
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
          void *Pointer;
      };

      void reset() noexcept;
      ExecutionStatus prepareBoolean(const ExecutionValueRef &Value);
      ExecutionStatus prepareInteger(std::uint32_t BitWidth, bool Signed, std::uint64_t Bits);
      ExecutionStatus prepareFloat(std::uint32_t BitWidth, std::uint64_t Bits);
      ExecutionStatus prepareString(ExecutionHeap &Heap, std::string_view Value);
      ExecutionStatus prepareString(ExecutionMemoryManager &Memory, std::string_view Value);
      ExecutionStatus preparePointer(ExecutionHeap &Heap, const ExecutionValueRef &Value);
      ExecutionStatus preparePointer(ExecutionMemoryManager &Memory, const FfiValueLayout &Layout, const ExecutionPointer &Pointer);

      NativeScalar Scalar;
      ExecutionStorageRef Buffer;
      ExecutionMemoryManager *Owner = nullptr;
      bool TemporaryBuffer = false;
      void *Address = nullptr;
  };
} // namespace ink::execution

#endif
