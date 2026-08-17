#ifndef INK_CLI_APPLICATION_H
#define INK_CLI_APPLICATION_H

#include <functional>
#include <iosfwd>
#include <memory>
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

  class Application;
  class CommandLineParser;

  class Option
  {
    public:
      Option &required() noexcept;
      Option &typeName(std::string Name);
      Option &repeatPolicy(RepeatPolicy Policy) noexcept;
      Option &excludes(Option &Other);

    private:
      enum class ValueKind
      {
        Flag,
        String,
        StringList,
      };

      Option(Application &Owner, std::string Names, bool &Storage, std::string Description);
      Option(Application &Owner, std::string Names, std::string &Storage, std::string Description);
      Option(Application &Owner, std::string Names, std::vector<std::string> &Storage, std::string Description);

      friend class Application;
      friend class CommandLineParser;

      Application *Owner;
      std::string Names;
      std::vector<std::string> Spellings;
      std::string Description;
      std::string ValueTypeName;
      ValueKind Kind;
      bool *FlagStorage = nullptr;
      std::string *StringStorage = nullptr;
      std::vector<std::string> *StringListStorage = nullptr;
      RepeatPolicy Policy = RepeatPolicy::Single;
      bool IsRequired = false;
      std::vector<Option *> Exclusions;
  };

  class Application
  {
    public:
      explicit Application(ApplicationInfo Info);
      Application(const Application &) = delete;
      Application &operator=(const Application &) = delete;
      Application(Application &&) = delete;
      Application &operator=(Application &&) = delete;

      Option &addFlag(std::string Names, bool &Storage, std::string Description = {});
      Option &addOption(std::string Names, std::string &Storage, std::string Description = {});
      Option &addOption(std::string Names, std::vector<std::string> &Storage, std::string Description = {});

      ParseResult parse(int ArgumentCount, char **ArgumentValues);
      ParseResult parse(int ArgumentCount, char **ArgumentValues, std::ostream &Output, std::ostream &ErrorOutput);
      ParseResult parseArguments(const std::vector<std::string> &Arguments, std::ostream &Output, std::ostream &ErrorOutput);

    private:
      friend class CommandLineParser;

      ApplicationInfo Info;
      std::vector<std::unique_ptr<Option>> Options;
  };

  int runMain(std::string_view ProgramName, const std::function<int()> &Body);
  int runMain(std::string_view ProgramName, const std::function<int()> &Body, std::ostream &ErrorOutput);

  constexpr int exitStatus(ExitCode Code) noexcept
  {
    return static_cast<int>(Code);
  }
} // namespace ink::cli

#endif
