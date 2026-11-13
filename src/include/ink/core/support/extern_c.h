#ifndef INK_CORE_SUPPORT_EXTERN_C_H
#define INK_CORE_SUPPORT_EXTERN_C_H

// C headers use these paired macros to preserve C linkage when included from C++.
#ifdef __cplusplus
#define INK_EXTERN_C_BEGIN \
  extern "C" \
  {
#define INK_EXTERN_C_END }
#else
#define INK_EXTERN_C_BEGIN
#define INK_EXTERN_C_END
#endif

#endif
