#ifndef INK_CORE_CORE_DEFINE_H
#define INK_CORE_CORE_DEFINE_H

#include <string_view>

namespace ink::core
{
  // Writes and flushes a fatal message before aborting, independently of installed loggers.
  [[noreturn]] void panic(std::string_view Message, const char *File, int Line) noexcept;
} // namespace ink::core

#define PANIC(Message) ::ink::core::panic((Message), __FILE__, __LINE__)

// Keep compiler-specific force-inlining behind a shared macro, as in UE's FORCEINLINE.
#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#elif defined(__clang__) || defined(__GNUC__)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#define FORCE_INLINE inline
#endif

#endif
