#ifndef INK_EXECUTION_HYBRID_RUNTIME_H
#define INK_EXECUTION_HYBRID_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
  typedef void (*InkHybridThunk)(void *Result, const void *const *Arguments);

  struct InkHybridFunction
  {
      uint32_t Function;
      uint32_t Flags; // Bit 0: patchable. Bit 1: native C export. Bit 2: native C import.
      InkHybridThunk Invoke;
  };

  struct InkHybridModule
  {
      uint32_t Version;
      const char *Manifest;
      size_t ManifestSize;
      const struct InkHybridFunction *Functions;
      size_t FunctionCount;
  };

  // Stable C status values, independent of ExecutionStatus's representation.
  enum InkHybridStatus
  {
    InkHybridSuccess = 0,
    InkHybridInvalidArtifact = 1,
    InkHybridIncompatible = 2,
    InkHybridBusy = 3,
    InkHybridExecutionFailed = 4,
  };

  // Registrations and patches belong to the calling thread. Modules have static lifetime.
  uint32_t ink_hybrid_register(const struct InkHybridModule *Module);
  uint32_t ink_hybrid_apply(const char *Path);
  uint32_t ink_hybrid_apply_bytes(const char *Bytes, size_t Length);
  uint32_t ink_hybrid_clear(void);
  const char *ink_hybrid_last_error(void);

  // Compiler-only entry protocol. Every normal enter is paired with leave.
  void *ink_hybrid_enter(const struct InkHybridModule *Module, uint32_t Function);
  void ink_hybrid_leave(void);
  uint32_t ink_hybrid_invoke(void *Patch, void *Result, const void *const *Arguments);
  void ink_hybrid_panic(void);
#ifdef __cplusplus
}
#endif

#endif
