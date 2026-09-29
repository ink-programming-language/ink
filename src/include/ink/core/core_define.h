#ifndef INK_CORE_CORE_DEFINE_H
#define INK_CORE_CORE_DEFINE_H

// Keep compiler-specific force-inlining behind a shared macro, as in UE's FORCEINLINE.
#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#elif defined(__clang__) || defined(__GNUC__)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#define FORCE_INLINE inline
#endif

#endif
