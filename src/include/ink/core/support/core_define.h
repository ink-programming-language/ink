#ifndef INK_CORE_CORE_DEFINE_H
#define INK_CORE_CORE_DEFINE_H

#include "ink/core/config.h"

#include <spdlog/fmt/fmt.h>

#include <atomic>
#include <string_view>

namespace ink::core
{
  // Writes and flushes a fatal message before aborting, independently of installed loggers.
  [[noreturn]] void panic(std::string_view Message, const char *File, int Line) noexcept;

  namespace detail
  {
    [[noreturn]] void checkFailed(const char *Expression, const char *File, int Line, std::string_view Message = {}) noexcept;
    void ensureFailed(const char *Expression, const char *File, int Line, std::string_view Message = {}) noexcept;
  } // namespace detail
} // namespace ink::core

#define PANIC(Message) ::ink::core::panic((Message), __FILE__, __LINE__)

// One switch controls check and ensure reporting and debugger breaks, independently of NDEBUG.
#define INK_ENABLE_ASSERTIONS (INK_BUILD_DEBUG || INK_BUILD_DEVELOPMENT)

#define INK_DETAIL_CHECK(Expression, ...) \
  do \
  { \
    if (!static_cast<bool>(Expression)) \
    { \
      ::ink::core::detail::checkFailed(#Expression, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (false)

// Disabled checks discard the condition and message arguments.
// Formatted variants accept a format string followed by optional arguments: CHECK_MSGF(Ready, "id={}", Id).
// Every enabled check failure is fatal, so the AlwaysMsgF form has the same behavior as MsgF.
#if INK_ENABLE_ASSERTIONS
#define CHECK(Expression) INK_DETAIL_CHECK(Expression, ::std::string_view{})
#define CHECK_MSGF(Expression, ...) INK_DETAIL_CHECK(Expression, ::fmt::format(__VA_ARGS__))
#define CHECK_ALWAYS_MSGF(Expression, ...) CHECK_MSGF(Expression, __VA_ARGS__)
#else
#define CHECK(Expression) static_cast<void>(0)
#define CHECK_MSGF(Expression, ...) static_cast<void>(0)
#define CHECK_ALWAYS_MSGF(Expression, ...) static_cast<void>(0)
#endif

#if INK_ENABLE_ASSERTIONS
// Each macro expansion owns its flag; concurrent failures at that call site report only once.
#define INK_DETAIL_ENSURE_ONCE() \
  ([]() \
  { \
    static ::std::atomic_flag Reported = ATOMIC_FLAG_INIT; \
    return !Reported.test_and_set(::std::memory_order_relaxed); \
  }())

#define INK_DETAIL_ENSURE(Expression, Always, ...) (static_cast<bool>(Expression) || (((Always) || INK_DETAIL_ENSURE_ONCE()) && (::ink::core::detail::ensureFailed(#Expression, __FILE__, __LINE__, __VA_ARGS__), false)))

// All ensures return bool and evaluate their condition exactly once, including after a prior failure.
// Message arguments are evaluated only when a failure is actually reported. Normal ensures report
// and break once per call site (per template specialization); AlwaysMsgF reports and breaks every failure.
#define ENSURE(Expression) INK_DETAIL_ENSURE(Expression, false, ::std::string_view{})
#define ENSURE_MSGF(Expression, ...) INK_DETAIL_ENSURE(Expression, false, ::fmt::format(__VA_ARGS__))
#define ENSURE_ALWAYS_MSGF(Expression, ...) INK_DETAIL_ENSURE(Expression, true, ::fmt::format(__VA_ARGS__))
#else
#define ENSURE(Expression) static_cast<bool>(Expression)
#define ENSURE_MSGF(Expression, ...) static_cast<bool>(Expression)
#define ENSURE_ALWAYS_MSGF(Expression, ...) static_cast<bool>(Expression)
#endif

// Keep compiler-specific force-inlining behind a shared macro, as in UE's FORCEINLINE.
#if INK_COMPILER_MSVC_ABI
#define FORCE_INLINE __forceinline
#elif INK_COMPILER_CLANG || INK_COMPILER_GCC
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#define FORCE_INLINE inline
#endif

#endif
