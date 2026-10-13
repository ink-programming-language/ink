#include "ink/core/support/core_define.h"

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <cstdlib>
#include <memory>

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
#elif INK_PLATFORM_LINUX || INK_PLATFORM_ANDROID
#include <csignal>
#include <cstdio>
#include <cstring>
#elif INK_PLATFORM_MACOS || INK_PLATFORM_IOS
#include <csignal>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace ink::core
{
  namespace
  {
    bool isDebuggerAttached() noexcept
    {
#if INK_PLATFORM_WINDOWS
      return ::IsDebuggerPresent() != 0;
#elif INK_PLATFORM_LINUX || INK_PLATFORM_ANDROID
      const std::unique_ptr<std::FILE, decltype(&std::fclose)> Status(std::fopen("/proc/self/status", "r"), &std::fclose);
      if (!Status)
      {
        return false;
      }
      char Line[256];
      while (std::fgets(Line, sizeof(Line), Status.get()))
      {
        if (std::strncmp(Line, "TracerPid:", 10) == 0)
        {
          return std::strtol(Line + 10, nullptr, 10) > 0;
        }
      }
      return false;
#elif INK_PLATFORM_MACOS || INK_PLATFORM_IOS
      kinfo_proc Info{};
      std::size_t Size = sizeof(Info);
      int Query[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};
      return sysctl(Query, 4, &Info, &Size, nullptr, 0) == 0 && (Info.kp_proc.p_flag & P_TRACED) != 0;
#else
      return false;
#endif
    }

    void debugBreakIfAttached() noexcept
    {
      if (!isDebuggerAttached())
      {
        return;
      }
#if INK_PLATFORM_WINDOWS
#if INK_COMPILER_MSVC_ABI
      __debugbreak();
#else
      ::DebugBreak();
#endif
#elif INK_PLATFORM_LINUX || INK_PLATFORM_ANDROID || INK_PLATFORM_MACOS || INK_PLATFORM_IOS
#if INK_COMPILER_CLANG
      __builtin_debugtrap();
#else
      std::raise(SIGTRAP);
#endif
#endif
    }

    [[noreturn]] void panicImpl(std::string_view Message, const char *File, int Line, bool BreakIntoDebugger) noexcept
    {
      const auto Sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
      spdlog::logger Logger("ink-panic", Sink);
      Logger.set_pattern("%v");
      Logger.critical("{}\npanic at {}:{}", Message, File, Line);
      Logger.flush();
      if (BreakIntoDebugger)
      {
        debugBreakIfAttached();
      }
#if INK_COMPILER_MSVC_ABI
      // The message is already flushed; terminate without CRT dialogs or duplicate output.
      _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
      std::abort();
    }
  } // namespace

  [[noreturn]] void panic(std::string_view Message, const char *File, int Line) noexcept
  {
    panicImpl(Message, File, Line, false);
  }

  namespace detail
  {
    [[noreturn]] void checkFailed(const char *Expression, const char *File, int Line, std::string_view Message) noexcept
    {
      panicImpl(Message.empty() ? fmt::format("check failed: {}", Expression) : fmt::format("check failed: {}\n{}", Expression, Message), File, Line, true);
    }

    void ensureFailed(const char *Expression, const char *File, int Line, std::string_view Message) noexcept
    {
      const auto Sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
      spdlog::logger Logger("ink-ensure", Sink);
      Logger.set_pattern("%v");
      if (Message.empty())
      {
        Logger.error("ensure failed: {}\nensure at {}:{}", Expression, File, Line);
      }
      else
      {
        Logger.error("ensure failed: {}\n{}\nensure at {}:{}", Expression, Message, File, Line);
      }
      Logger.flush();
      debugBreakIfAttached();
    }
  } // namespace detail
} // namespace ink::core
