#include "ink/execution/ffi/ffi_type.h"
#include "ink/execution/runtime/runtime_type.h"

#include <ffi.h>

namespace ink::execution
{
  ::_ffi_type *ffiType(const TypeDesc &Layout, FfiTypeUsage Usage) noexcept
  {
    switch (Layout.Kind)
    {
    case RuntimeKind::Void:
      return Usage == FfiTypeUsage::Return ? &ffi_type_void : nullptr;
    case RuntimeKind::Boolean:
      return &ffi_type_uint8;
    case RuntimeKind::Integer:
      switch (Layout.bitWidth())
      {
      case 8:
        return Layout.isSigned() ? &ffi_type_sint8 : &ffi_type_uint8;
      case 16:
        return Layout.isSigned() ? &ffi_type_sint16 : &ffi_type_uint16;
      case 32:
        return Layout.isSigned() ? &ffi_type_sint32 : &ffi_type_uint32;
      case 64:
        return Layout.isSigned() ? &ffi_type_sint64 : &ffi_type_uint64;
      default:
        return nullptr;
      }
    case RuntimeKind::Float:
      switch (Layout.bitWidth())
      {
      case 32:
        return &ffi_type_float;
      case 64:
        return &ffi_type_double;
      default:
        return nullptr;
      }
    case RuntimeKind::Pointer:
      return &ffi_type_pointer;
    default:
      return nullptr;
    }
  }
} // namespace ink::execution
