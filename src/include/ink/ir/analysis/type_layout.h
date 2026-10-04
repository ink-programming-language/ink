#ifndef INK_IR_ANALYSIS_TYPE_LAYOUT_H
#define INK_IR_ANALYSIS_TYPE_LAYOUT_H

#include "ink/core/target_context.h"
#include "ink/ir/type/type.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace ink::ir
{
  // Internal Ink object ABI, independent of VM payloads and platform C aggregate classification.
  // Size includes tail padding; array elements are separated by Stride bytes.
  struct TypeLayout
  {
      std::uint64_t Size = 0;
      std::uint64_t Alignment = 1;
      std::uint64_t Stride = 0;
      std::vector<std::uint64_t> FieldOffsets;
      // Absent for ordinary value classes. Future polymorphic layouts can
      // describe their hidden header/base without renumbering source fields.
      std::optional<std::uint64_t> BaseOffset;
      std::optional<std::uint64_t> VPtrOffset;
  };

  std::optional<TypeLayout> computeTypeLayout(const Type &ValueType, const core::TargetContext &Target);
} // namespace ink::ir

#endif
