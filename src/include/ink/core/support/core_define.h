#ifndef INK_CORE_CORE_DEFINE_H
#define INK_CORE_CORE_DEFINE_H

#include "ink/core/config.h"

#include <spdlog/fmt/fmt.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <atomic>
#include <cstdlib>
#include <memory>
#include <string_view>

#if INK_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#if INK_COMPILER_MSVC_ABI
#include <intrin.h>
#endif
#define INK_DETAIL_IS_DEBUGGER_ATTACHED() (::IsDebuggerPresent() != 0)
#elif INK_PLATFORM_LINUX || INK_PLATFORM_ANDROID
#include <csignal>
#include <cstdio>
#include <cstring>
#define INK_DETAIL_IS_DEBUGGER_ATTACHED() \
  ([]() noexcept \
  { \
    const ::std::unique_ptr<::std::FILE, decltype(&::std::fclose)> Status(::std::fopen("/proc/self/status", "r"), &::std::fclose); \
    if (!Status) \
    { \
      return false; \
    } \
    char Line[256]; \
    while (::std::fgets(Line, sizeof(Line), Status.get())) \
    { \
      if (::std::strncmp(Line, "TracerPid:", 10) == 0) \
      { \
        return ::std::strtol(Line + 10, nullptr, 10) > 0; \
      } \
    } \
    return false; \
  }())
#elif INK_PLATFORM_MACOS || INK_PLATFORM_IOS
#include <csignal>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>
#define INK_DETAIL_IS_DEBUGGER_ATTACHED() \
  ([]() noexcept \
  { \
    ::kinfo_proc Info{}; \
    ::std::size_t Size = sizeof(Info); \
    int Query[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, ::getpid()}; \
    return ::sysctl(Query, 4, &Info, &Size, nullptr, 0) == 0 && (Info.kp_proc.p_flag & P_TRACED) != 0; \
  }())
#else
#define INK_DETAIL_IS_DEBUGGER_ATTACHED() false
#endif

#if INK_PLATFORM_WINDOWS
#if INK_COMPILER_MSVC_ABI
#define INK_DETAIL_DEBUG_BREAK() __debugbreak()
#else
#define INK_DETAIL_DEBUG_BREAK() ::DebugBreak()
#endif
#elif INK_PLATFORM_LINUX || INK_PLATFORM_ANDROID || INK_PLATFORM_MACOS || INK_PLATFORM_IOS
#if INK_COMPILER_CLANG
#define INK_DETAIL_DEBUG_BREAK() __builtin_debugtrap()
#else
#define INK_DETAIL_DEBUG_BREAK() static_cast<void>(::std::raise(SIGTRAP))
#endif
#else
#define INK_DETAIL_DEBUG_BREAK() static_cast<void>(0)
#endif

// Keep the trap at the macro expansion so the debugger stops at the failing call site.
#define INK_DETAIL_DEBUG_BREAK_IF_ATTACHED() (INK_DETAIL_IS_DEBUGGER_ATTACHED() ? INK_DETAIL_DEBUG_BREAK() : static_cast<void>(0))

#if INK_COMPILER_MSVC_ABI
// The message is already flushed; terminate without CRT dialogs or duplicate output.
#define INK_DETAIL_ABORT() (::_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT), ::std::abort())
#else
#define INK_DETAIL_ABORT() ::std::abort()
#endif

// Writes and flushes a fatal message before aborting, independently of installed loggers.
#define panic(Message) \
  ([](::std::string_view InkPanicMessage) noexcept \
  { \
    const auto InkPanicSink = ::std::make_shared<::spdlog::sinks::stderr_sink_mt>(); \
    ::spdlog::logger InkPanicLogger("ink-panic", InkPanicSink); \
    InkPanicLogger.set_pattern("%v"); \
    InkPanicLogger.critical("{}\npanic at {}:{}", InkPanicMessage, __FILE__, __LINE__); \
    InkPanicLogger.flush(); \
  }((Message)), INK_DETAIL_DEBUG_BREAK_IF_ATTACHED(), INK_DETAIL_ABORT())

// One switch controls check and ensure reporting and debugger breaks, independently of NDEBUG.
#define INK_ENABLE_ASSERTIONS (INK_BUILD_DEBUG || INK_BUILD_DEVELOPMENT)

#define INK_DETAIL_CHECK(Expression, ...) \
  do \
  { \
    if (!static_cast<bool>(Expression)) \
    { \
      panic([](::std::string_view InkCheckMessage) noexcept \
      { \
        return InkCheckMessage.empty() ? ::fmt::format("check failed: {}", #Expression) : ::fmt::format("check failed: {}\n{}", #Expression, InkCheckMessage); \
      }(__VA_ARGS__)); \
    } \
  } while (false)

// Disabled checks discard the condition and message arguments.
// Formatted variants accept a format string followed by optional arguments: checkMsgf(Ready, "id={}", Id).
// Every enabled check failure is fatal, so Always variants have the same behavior as ordinary checks.
#if INK_ENABLE_ASSERTIONS
#define check(Expression) INK_DETAIL_CHECK(Expression, ::std::string_view{})
#define checkMsgf(Expression, ...) INK_DETAIL_CHECK(Expression, ::fmt::format(__VA_ARGS__))
#define checkAlways(Expression) check(Expression)
#define checkAlwaysMsgf(Expression, ...) checkMsgf(Expression, __VA_ARGS__)
#else
#define check(Expression) static_cast<void>(0)
#define checkMsgf(Expression, ...) static_cast<void>(0)
#define checkAlways(Expression) static_cast<void>(0)
#define checkAlwaysMsgf(Expression, ...) static_cast<void>(0)
#endif

#if INK_ENABLE_ASSERTIONS
// Each macro expansion owns its flag; concurrent failures at that call site report only once.
#define INK_DETAIL_ENSURE_ONCE() \
  ([]() \
  { \
    static ::std::atomic_flag Reported = ATOMIC_FLAG_INIT; \
    return !Reported.test_and_set(::std::memory_order_relaxed); \
  }())

#define INK_DETAIL_ENSURE(Expression, Always, ...) \
  (static_cast<bool>(Expression) || (((Always) || INK_DETAIL_ENSURE_ONCE()) && ([](::std::string_view InkEnsureMessage) noexcept \
  { \
    const auto InkEnsureSink = ::std::make_shared<::spdlog::sinks::stderr_sink_mt>(); \
    ::spdlog::logger InkEnsureLogger("ink-ensure", InkEnsureSink); \
    InkEnsureLogger.set_pattern("%v"); \
    if (InkEnsureMessage.empty()) \
    { \
      InkEnsureLogger.error("ensure failed: {}\nensure at {}:{}", #Expression, __FILE__, __LINE__); \
    } \
    else \
    { \
      InkEnsureLogger.error("ensure failed: {}\n{}\nensure at {}:{}", #Expression, InkEnsureMessage, __FILE__, __LINE__); \
    } \
    InkEnsureLogger.flush(); \
  }(__VA_ARGS__), INK_DETAIL_DEBUG_BREAK_IF_ATTACHED(), false)))

// All ensures return bool and evaluate their condition exactly once, including after a prior failure.
// Message arguments are evaluated only when a failure is actually reported. Normal ensures report
// and break once per call site (per template specialization); Always variants report and break every failure.
#define ensure(Expression) INK_DETAIL_ENSURE(Expression, false, ::std::string_view{})
#define ensureMsgf(Expression, ...) INK_DETAIL_ENSURE(Expression, false, ::fmt::format(__VA_ARGS__))
#define ensureAlways(Expression) INK_DETAIL_ENSURE(Expression, true, ::std::string_view{})
#define ensureAlwaysMsgf(Expression, ...) INK_DETAIL_ENSURE(Expression, true, ::fmt::format(__VA_ARGS__))
#else
#define ensure(Expression) static_cast<bool>(Expression)
#define ensureMsgf(Expression, ...) static_cast<bool>(Expression)
#define ensureAlways(Expression) static_cast<bool>(Expression)
#define ensureAlwaysMsgf(Expression, ...) static_cast<bool>(Expression)
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
