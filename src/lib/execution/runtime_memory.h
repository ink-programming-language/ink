#ifndef INK_EXECUTION_RUNTIME_MEMORY_H
#define INK_EXECUTION_RUNTIME_MEMORY_H

#include "ink/execution/runtime_value.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ink::execution::detail
{
  RuntimeMemoryStatus computeElementByteOffset(const ir::Type &ElementType, std::uint64_t Index, const std::vector<std::uint32_t> &FieldIndices, const core::TargetContext &Target, std::uint64_t &ByteOffset);
  RuntimeMemoryStatus loadRuntimeMemoryValue(RuntimeValueArena &Values, const ir::Type &ValueType, const std::uint8_t *Data, const core::TargetContext &Target, RuntimeValueRef &Value);
  RuntimeMemoryStatus storeRuntimeMemoryValue(const RuntimeValue &Value, std::uint8_t *Data, const core::TargetContext &Target);
  RuntimeMemoryStatus loadRuntimeIntegerPayload(const ir::Type &ValueType, const std::uint8_t *Data, const core::TargetContext &Target, std::uint64_t &Value);
  RuntimeMemoryStatus storeRuntimeIntegerPayload(const ir::Type &ValueType, std::uint8_t *Data, const core::TargetContext &Target, std::uint64_t Value);
} // namespace ink::execution::detail

#endif
