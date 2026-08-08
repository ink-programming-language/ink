#ifndef INK_CLI_APPLICATION_H
#define INK_CLI_APPLICATION_H

#include <CLI/CLI.hpp>

#include <functional>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace ink::cli
{
  enum class ExitCode : int
  {
    Success = 0,
    SourceError = 1,
    InvocationError = 2,
    InternalError = 3,
  };

  enum class RepeatPolicy
  {
    Single,
    Append,
    LastWins,
  };

  struct ApplicationInfo
  {
    std::string Name;
    std::string Description;
    std::string Version;
  };

  struct ParseResult
  {
    bool ShouldExit = false;
    ExitCode Code = ExitCode::Success;
  };

  class Application
  {
  public:
    explicit Application(ApplicationInfo Info);

    CLI::App &app() noexcept;
    const CLI::App &app() const noexcept;

    // User invocation errors are returned as ParseResult; invalid command definitions throw std::logic_error for runMain to report as internal errors.
    ParseResult parse(int ArgumentCount, char **ArgumentValues);
    ParseResult parse(int ArgumentCount, char **ArgumentValues, std::ostream &Output, std::ostream &ErrorOutput);
    ParseResult parseArguments(const std::vector<std::string> &Arguments, std::ostream &Output, std::ostream &ErrorOutput);

  private:
    ParseResult handleParseError(const CLI::ParseError &Error, std::ostream &Output, std::ostream &ErrorOutput) const;

    CLI::App App;
  };

  CLI::Option &setRepeatPolicy(CLI::Option &Option, RepeatPolicy Policy);
  int runMain(std::string_view ProgramName, const std::function<int()> &Body);
  int runMain(std::string_view ProgramName, const std::function<int()> &Body, std::ostream &ErrorOutput);

  constexpr int exitStatus(ExitCode Code) noexcept
  {
    return static_cast<int>(Code);
  }
} // namespace ink::cli

#endif
