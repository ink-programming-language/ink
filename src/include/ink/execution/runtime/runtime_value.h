#ifndef INK_EXECUTION_RUNTIME_RUNTIME_VALUE_H
#define INK_EXECUTION_RUNTIME_RUNTIME_VALUE_H

#include "ink/execution/memory/execution_pointer.h"
#include "ink/execution/runtime/runtime_type.h"
#include "ink/execution/value/execution_integer.h"

#include <memory>
#include <span>
#include <string_view>

namespace ink::execution
{
  struct RuntimePayload;

  // Owned values for archive constants and semantic/native boundary conversion.
  // The VM uses RuntimeSlot views over native-layout frame bytes; it does not
  // build RuntimeValue trees for loads, stores or internal calls.
  // Type is relative to its execution image's type table. Crossing independent
  // type domains requires semantic bridging, never copying numeric IDs alone.
  struct RuntimeValue
  {
      static RuntimeValue fromBits(std::uint64_t Bits, RuntimeTypeId Type = InvalidRuntimeType) noexcept;
      static RuntimeValue fromInteger(ExecutionInteger Value, RuntimeTypeId Type = InvalidRuntimeType);
      static RuntimeValue fromPointer(ExecutionPointer Value, RuntimeTypeId Type = InvalidRuntimeType);
      static RuntimeValue fromString(std::string_view Value, RuntimeTypeId Type = InvalidRuntimeType);
      static RuntimeValue fromArray(std::vector<RuntimeValue> Elements, RuntimeTypeId Type = InvalidRuntimeType);
      static RuntimeValue fromClass(std::vector<RuntimeValue> Fields, RuntimeTypeId Type = InvalidRuntimeType);
      const ExecutionInteger &integer() const noexcept;
      const ExecutionPointer &pointer() const noexcept;
      std::string_view string() const noexcept;
      std::span<const RuntimeValue> array() const noexcept;
      std::span<const RuntimeValue> fields() const noexcept;
      std::span<const RuntimeValue> aggregate() const noexcept;
      RuntimeKind kind() const noexcept;

      RuntimeTypeId Type = InvalidRuntimeType;
      std::uint64_t Bits = 0;
      std::shared_ptr<const RuntimePayload> Object;
      bool Initialized = false;
  };

  struct RuntimeValueResult
  {
      ExecutionStatus Status = ExecutionStatus::InvalidArguments;
      RuntimeValue Value;

      explicit operator bool() const noexcept
      {
        return Status == ExecutionStatus::Success && Value.Initialized;
      }
  };
} // namespace ink::execution

#endif
