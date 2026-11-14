#pragma once

#include "ink/backend/vm/vm_runtime/frame.h"
#include "ink/core/config.h"

#if INK_PLATFORM_WINDOWS
namespace ink::vm
{
  // Both stubs require a valid frame and an eight-byte-aligned, writable call area, and leave SP unchanged.
  // Reads the u32 standard-handle selector and writes the pointer result in a 16-byte call area ending at SP.
  void getStdHandleStub(Frame *CallFrame) noexcept;

  // Forwards the handle, buffer, u32 byte count, bytes-written pointer, and OVERLAPPED pointer from a 48-byte call area ending at SP.
  // Writes the i32 BOOL result without changing SP, argument slots, or return-slot padding; pointers must satisfy Win32 requirements.
  void writeFileStub(Frame *CallFrame) noexcept;
} // namespace ink::vm
#endif
