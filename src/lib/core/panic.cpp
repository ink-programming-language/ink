#include "ink/core/core_define.h"

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <cstdlib>
#include <memory>

namespace ink::core
{
  [[noreturn]] void panic(std::string_view Message, const char *File, int Line) noexcept
  {
    const auto Sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
    spdlog::logger Logger("ink-panic", Sink);
    Logger.set_pattern("%v");
    Logger.critical("{}\npanic at {}:{}", Message, File, Line);
    Logger.flush();
#ifdef _MSC_VER
    // The message is already flushed; terminate without CRT dialogs or duplicate output.
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
    std::abort();
  }
} // namespace ink::core
