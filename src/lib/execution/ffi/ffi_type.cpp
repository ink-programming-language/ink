#include "ink/execution/ffi/ffi_type.h"

#include "ink/ir/type/float_type.h"
#include "ink/ir/type/integer_type.h"

#include <ffi.h>

namespace ink::execution
{
  ::_ffi_type *ffiType(const ir::Type &Type, FfiTypeUsage Usage) noexcept
  {
    switch (Type.typeKind())
    {
    case ir::TypeKind::Void:
      return Usage == FfiTypeUsage::Return ? &ffi_type_void : nullptr;
    case ir::TypeKind::Bool:
      return &ffi_type_uint8;
    case ir::TypeKind::Integer:
    {
      const auto &Integer = static_cast<const ir::IntegerType &>(Type);
      switch (Integer.bitWidth())
      {
      case 8:
        return Integer.isSigned() ? &ffi_type_sint8 : &ffi_type_uint8;
      case 16:
        return Integer.isSigned() ? &ffi_type_sint16 : &ffi_type_uint16;
      case 32:
        return Integer.isSigned() ? &ffi_type_sint32 : &ffi_type_uint32;
      case 64:
        return Integer.isSigned() ? &ffi_type_sint64 : &ffi_type_uint64;
      default:
        return nullptr;
      }
    }
    case ir::TypeKind::Float:
    {
      const auto &Float = static_cast<const ir::FloatType &>(Type);
      switch (Float.bitWidth())
      {
      case 32:
        return &ffi_type_float;
      case 64:
        return &ffi_type_double;
      default:
        return nullptr;
      }
    }
    case ir::TypeKind::Pointer:
      return &ffi_type_pointer;
    default:
      return nullptr;
    }
  }
} // namespace ink::execution
