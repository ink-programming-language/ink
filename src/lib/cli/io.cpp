#include "ink/cli/io.h"

#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/ostream_sink.h>

#include <cstdio>
#include <memory>
#include <ostream>
#include <string>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace ink::cli
{
  std::filesystem::path pathFromUtf8(std::string_view Path)
  {
    return std::filesystem::u8path(Path.begin(), Path.end());
  }

  bool useBinaryStandardInput() noexcept
  {
#ifdef _WIN32
    return _setmode(_fileno(stdin), _O_BINARY) != -1;
#else
    return true;
#endif
  }

  bool writeOutput(std::ostream &Output, std::string_view Message)
  {
    const auto Sink = std::make_shared<spdlog::sinks::ostream_sink_st>(Output, true);
    spdlog::logger Logger("ink-output", Sink);
    bool Failed = false;
    Logger.set_error_handler([&Failed](const std::string &)
    {
      Failed = true;
    });
    Logger.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
    Logger.log(spdlog::level::info, spdlog::string_view_t(Message.data(), Message.size()));
    return !Failed && static_cast<bool>(Output);
  }
} // namespace ink::cli
