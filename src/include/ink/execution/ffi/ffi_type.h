#ifndef INK_EXECUTION_FFI_FFI_TYPE_H
#define INK_EXECUTION_FFI_FFI_TYPE_H

struct _ffi_type;

namespace ink::ir
{
  class Type;
} // namespace ink::ir

namespace ink::execution
{
  struct StorageLayout;
  enum class FfiTypeUsage
  {
    Argument,
    Return,
  };

  // Maps a supported native C type, or returns null when no conversion exists.
  // Void is return-only; pointer payload validation belongs to FfiArgument.
  ::_ffi_type *ffiType(const ir::Type &Type, FfiTypeUsage Usage) noexcept;
  ::_ffi_type *ffiType(const StorageLayout &Layout, FfiTypeUsage Usage) noexcept;
} // namespace ink::execution

#endif
