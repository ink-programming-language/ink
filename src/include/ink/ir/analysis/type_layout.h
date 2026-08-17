#ifndef INK_IR_TYPE_LAYOUT_H
#define INK_IR_TYPE_LAYOUT_H

#include "ink/core/target_context.h"
#include "ink/ir/model/type.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace ink::ir
{
  // Describes Ink's canonical in-memory representation for the selected pointer width and byte order, not a platform C ABI layout.
  struct TypeLayout
  {
      std::size_t Size = 0;
      std::size_t Alignment = 1;
      std::size_t StrideSize = 0;
      std::vector<std::size_t> FieldOffsets;
  };

  std::optional<TypeLayout> computeTypeLayout(const Type &TypeValue, const core::TargetContext &Target);

  // Reports whether typed Ink memory operations currently support the value type; pointer and slice representations are intentionally excluded.
  bool isMemoryValueType(const Type &TypeValue) noexcept;
} // namespace ink::ir

#endif
