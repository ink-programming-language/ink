#ifndef INK_EXECUTION_FFI_FFI_VALUE_LAYOUT_H
#define INK_EXECUTION_FFI_FFI_VALUE_LAYOUT_H

#include "ink/execution/runtime/runtime_type.h"

namespace ink::execution
{
  // Prepared native marshalling decisions contain no semantic type references.
  struct FfiValueLayout
  {
      std::shared_ptr<const RuntimeTypeDomain> Domain;
      RuntimeTypeId Type = InvalidRuntimeType;
      RuntimeKind Kind = RuntimeKind::Invalid;
      std::uint32_t BitWidth = 0;
      bool Signed = false;
      RuntimeTypeId Pointee = InvalidRuntimeType;
      bool Writable = false;
      bool VoidPointer = false;
      bool BytePointer = false;
      bool ByteBufferPointer = false;
  };
} // namespace ink::execution

#endif
