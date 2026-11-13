#pragma once

#include "ink/backend/vm/vm_runtime/frame.h"

namespace ink::vm
{
  // Requires a valid frame and an eight-byte-aligned, writable 16-byte call area ending at SP.
  // Reads the i64 argument at SP - 8 and writes the i64 result at SP - 16 without changing SP.
  void identityStub(Frame *CallFrame) noexcept;
} // namespace ink::vm
