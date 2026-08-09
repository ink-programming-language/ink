#include "ink/cli/application.h"
#include "ink/cli/diagnostic.h"

#include <algorithm>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace ink::cli
{
  namespace
  {
    std::string failureMessage(const CLI::App *App, const CLI::Error &Error)
    {
      std::string Message = Error.what();
      const std::string ExistingPrefix = App->get_name() + ": ";
      if (Message.compare(0, ExistingPrefix.size(), ExistingPrefix) == 0)
      {
        Message.erase(0, ExistingPrefix.size());
      }
      return App->get_name() + ": error: " + Message + "\nTry '" + App->get_name() + " --help' for more information.\n";
    }

    bool isUtf8Continuation(unsigned char Byte) noexcept
    {
      return Byte >= 0x80 && Byte <= 0xBF;
    }

    bool isValidUtf8(std::string_view Text) noexcept
    {
      std::size_t Offset = 0;
      while (Offset < Text.size())
      {
        const unsigned char First = static_cast<unsigned char>(Text[Offset]);
        if (First <= 0x7F)
        {
          ++Offset;
          continue;
        }

        std::size_t Length = 0;
        if (First >= 0xC2 && First <= 0xDF)
        {
          Length = 2;
        }
        else if (First >= 0xE0 && First <= 0xEF)
        {
          Length = 3;
        }
        else if (First >= 0xF0 && First <= 0xF4)
        {
          Length = 4;
        }
        else
        {
          return false;
        }

        if (Offset + Length > Text.size())
        {
          return false;
        }
        for (std::size_t Index = 1; Index < Length; ++Index)
        {
          if (!isUtf8Continuation(static_cast<unsigned char>(Text[Offset + Index])))
          {
            return false;
          }
        }

        const unsigned char Second = static_cast<unsigned char>(Text[Offset + 1]);
        if ((First == 0xE0 && Second < 0xA0) || (First == 0xED && Second > 0x9F) || (First == 0xF0 && Second < 0x90) || (First == 0xF4 && Second > 0x8F))
        {
          return false;
        }
        Offset += Length;
      }
      return true;
    }

    void validateUtf8Arguments(const std::vector<std::string> &Arguments)
    {
      for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
      {
        if (!isValidUtf8(Arguments[Index]))
        {
          throw CLI::ValidationError("argument " + std::to_string(Index + 1), "is not valid UTF-8");
        }
      }
    }

    bool isLowerKebabCase(std::string_view Name) noexcept
    {
      if (Name.empty() || Name.front() < 'a' || Name.front() > 'z')
      {
        return false;
      }
      bool PreviousWasDash = false;
      for (char Character : Name)
      {
        if ((Character >= 'a' && Character <= 'z') || (Character >= '0' && Character <= '9'))
        {
          PreviousWasDash = false;
          continue;
        }
        if (Character != '-' || PreviousWasDash)
        {
          return false;
        }
        PreviousWasDash = true;
      }
      return !PreviousWasDash;
    }

    bool isAsciiAlphanumeric(char Character) noexcept
    {
      return (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z') || (Character >= '0' && Character <= '9');
    }

    [[noreturn]] void throwDefinitionError(std::string_view Subject, std::string_view Message)
    {
      throw std::logic_error("invalid command-line definition for '" + std::string(Subject) + "': " + std::string(Message));
    }

    void applyCommandPolicies(CLI::App &Command)
    {
      if (Command.get_config_ptr() != nullptr)
      {
        throwDefinitionError(Command.get_name().empty() ? "anonymous option group" : Command.get_name(), "CLI11 config files are not supported");
      }
      Command.option_defaults()->ignore_case(false)->ignore_underscore(false)->disable_flag_override(true)->multi_option_policy(CLI::MultiOptionPolicy::Throw);
      Command.allow_extras(false);
      Command.allow_non_standard_option_names(false);
      Command.allow_subcommand_prefix_matching(false);
      Command.allow_windows_style_options(false);
      Command.prefix_command(false);
      Command.ignore_case(false);
      Command.ignore_underscore(false);
      Command.positionals_at_end(false);
      Command.validate_positionals(true);
      Command.failure_message(failureMessage);
      for (CLI::Option *Option : Command.get_options())
      {
        Option->ignore_case(false)->ignore_underscore(false);
        if (!Option->get_envname().empty())
        {
          throwDefinitionError(Option->get_name(), "environment-backed option values are not supported");
        }
        for (const std::string &ShortName : Option->get_snames())
        {
          if (ShortName.size() != 1 || !isAsciiAlphanumeric(ShortName.front()))
          {
            throwDefinitionError("-" + ShortName, "short option names must contain exactly one ASCII letter or digit");
          }
        }
        for (const std::string &LongName : Option->get_lnames())
        {
          if (!isLowerKebabCase(LongName))
          {
            throwDefinitionError("--" + LongName, "long option names must use lowercase ASCII kebab-case");
          }
        }
        const CLI::MultiOptionPolicy Policy = Option->get_multi_option_policy();
        if (Option->get_expected_max() == 0)
        {
          if (Option->get_allow_extra_args() || Option->get_delimiter() != '\0' || Option->get_inject_separator())
          {
            throwDefinitionError(Option->get_name(), "Boolean flags cannot consume or synthesize values");
          }
          if (Policy != CLI::MultiOptionPolicy::Throw && Policy != CLI::MultiOptionPolicy::TakeLast)
          {
            throwDefinitionError(Option->get_name(), "Boolean flags must use the Single repeat policy");
          }
          Option->disable_flag_override()->multi_option_policy(CLI::MultiOptionPolicy::Throw);
        }
        else if (Option->nonpositional() && (Option->get_expected_min() != 1 || Option->get_expected_max() != 1 || Option->get_type_size_min() != 1 || Option->get_type_size_max() != 1 || Option->get_allow_extra_args() || Option->get_delimiter() != '\0' || Option->get_inject_separator()))
        {
          throwDefinitionError(Option->get_name(), "value-taking options must consume exactly one non-empty argv value per occurrence");
        }
        if (Policy != CLI::MultiOptionPolicy::Throw && Policy != CLI::MultiOptionPolicy::TakeAll && Policy != CLI::MultiOptionPolicy::TakeLast)
        {
          throwDefinitionError(Option->get_name(), "repeat policy must be Single, Append, or LastWins");
        }
      }
      for (CLI::App *ChildCommand : Command.get_subcommands([](CLI::App *)
      {
        return true;
      }))
      {
        if (!ChildCommand->get_name().empty() || !ChildCommand->get_aliases().empty())
        {
          const std::string &Name = ChildCommand->get_name().empty() ? ChildCommand->get_aliases().front() : ChildCommand->get_name();
          throwDefinitionError(Name, "named subcommands and aliases are not supported by ink::cli::Application v0");
        }
        applyCommandPolicies(*ChildCommand);
      }
    }

    const CLI::Option *findOption(const CLI::App &Command, std::string_view Name)
    {
      for (const CLI::Option *Option : Command.get_options())
      {
        for (const std::string &LongName : Option->get_lnames())
        {
          if (Name.size() == LongName.size() + 2 && Name.substr(0, 2) == "--" && Name.substr(2) == LongName)
          {
            return Option;
          }
        }
        for (const std::string &ShortName : Option->get_snames())
        {
          if (Name.size() == ShortName.size() + 1 && Name.front() == '-' && Name.substr(1) == ShortName)
          {
            return Option;
          }
        }
      }
      for (const CLI::App *OptionGroup : Command.get_subcommands([](const CLI::App *Candidate)
      {
        return Candidate->get_name().empty() && !Candidate->get_disabled();
      }))
      {
        if (const CLI::Option *Option = findOption(*OptionGroup, Name))
        {
          return Option;
        }
      }
      return nullptr;
    }

    void recordOption(const CLI::Option *Option, std::string_view Name, std::vector<const CLI::Option *> &SeenOptions)
    {
      if (std::find(SeenOptions.begin(), SeenOptions.end(), Option) != SeenOptions.end())
      {
        if (Option->get_multi_option_policy() == CLI::MultiOptionPolicy::Throw)
        {
          throw CLI::ValidationError(std::string(Name), "may be specified at most once");
        }
        return;
      }
      SeenOptions.push_back(Option);
    }

    void consumeRequiredValues(const CLI::Option &Option, std::string_view Name, std::size_t AttachedValueCount, const std::vector<std::string> &Arguments, std::size_t &ArgumentIndex)
    {
      const std::size_t RequiredValueCount = static_cast<std::size_t>((std::min)(Option.get_type_size_min(), Option.get_items_expected_min()));
      if (RequiredValueCount == 0)
      {
        throw CLI::ValidationError(std::string(Name), "optional option values are not supported");
      }
      for (std::size_t ValueIndex = AttachedValueCount; ValueIndex < RequiredValueCount; ++ValueIndex)
      {
        if (ArgumentIndex + 1 >= Arguments.size() || Arguments[ArgumentIndex + 1].empty() || (Arguments[ArgumentIndex + 1].front() == '-' && Arguments[ArgumentIndex + 1] != "-"))
        {
          throw CLI::ValidationError(std::string(Name), "is missing a value; dash-leading values must use attached form");
        }
        ++ArgumentIndex;
      }
    }

    void validateOptionSyntax(const CLI::App &Command, const std::vector<std::string> &Arguments)
    {
      std::vector<const CLI::Option *> SeenOptions;
      for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments.size(); ++ArgumentIndex)
      {
        const std::string &Argument = Arguments[ArgumentIndex];
        if (Argument == "--")
        {
          break;
        }
        if (Argument == "-" || Argument.empty() || Argument.front() != '-')
        {
          continue;
        }
        if (Argument.size() >= 2 && Argument[1] == '-')
        {
          const std::size_t Separator = Argument.find('=');
          const std::string_view Name(Argument.data(), Separator == std::string::npos ? Argument.size() : Separator);
          const CLI::Option *Option = findOption(Command, Name);
          if (Option == nullptr)
          {
            throw CLI::ExtrasError(std::vector<std::string>{Argument});
          }
          recordOption(Option, Name, SeenOptions);
          if (Separator != std::string::npos && Separator + 1 == Argument.size())
          {
            throw CLI::ValidationError(std::string(Name), "requires a non-empty value after '='");
          }
          if (Option->get_expected_max() == 0)
          {
            if (Separator != std::string::npos)
            {
              throw CLI::ValidationError(std::string(Name), "Boolean flags do not accept values");
            }
          }
          else
          {
            consumeRequiredValues(*Option, Name, Separator == std::string::npos ? 0 : 1, Arguments, ArgumentIndex);
          }
          continue;
        }

        for (std::size_t CharacterIndex = 1; CharacterIndex < Argument.size(); ++CharacterIndex)
        {
          const std::string Name = std::string("-") + Argument[CharacterIndex];
          const CLI::Option *Option = findOption(Command, Name);
          if (Option == nullptr)
          {
            throw CLI::ExtrasError(std::vector<std::string>{Argument});
          }
          recordOption(Option, Name, SeenOptions);
          if (Option->get_expected_max() != 0)
          {
            consumeRequiredValues(*Option, Name, CharacterIndex + 1 < Argument.size() ? 1 : 0, Arguments, ArgumentIndex);
            break;
          }
          if (CharacterIndex + 1 < Argument.size() && Argument[CharacterIndex + 1] == '=')
          {
            throw CLI::ValidationError(Name, "Boolean flags do not accept values");
          }
        }
      }
      for (const CLI::Option *Option : SeenOptions)
      {
        for (const CLI::Option *ExcludedOption : Option->get_excludes())
        {
          if (std::find(SeenOptions.begin(), SeenOptions.end(), ExcludedOption) != SeenOptions.end())
          {
            throw CLI::ValidationError(Option->get_name(), "conflicts with " + ExcludedOption->get_name());
          }
        }
      }
    }
  } // namespace

  Application::Application(ApplicationInfo Info) : App(std::move(Info.Description), Info.Name)
  {
    App.get_help_ptr()->disable_flag_override();
    CLI::Option *VersionOption = App.set_version_flag("-V,--version", Info.Name + " " + Info.Version, "Print version information and exit")->disable_flag_override();
    App.get_help_ptr()->excludes(VersionOption);
    applyCommandPolicies(App);
  }

  CLI::App &Application::app() noexcept
  {
    return App;
  }

  const CLI::App &Application::app() const noexcept
  {
    return App;
  }

  ParseResult Application::parse(int ArgumentCount, char **ArgumentValues)
  {
    return parse(ArgumentCount, ArgumentValues, std::cout, std::cerr);
  }

  ParseResult Application::parse(int ArgumentCount, char **ArgumentValues, std::ostream &Output, std::ostream &ErrorOutput)
  {
    try
    {
      ArgumentValues = App.ensure_utf8(ArgumentValues);
    }
    catch (const std::runtime_error &)
    {
      ErrorOutput << App.get_name() << ": error: cannot decode process arguments as UTF-8\nTry '" << App.get_name() << " --help' for more information.\n";
      return {true, ExitCode::InvocationError};
    }
    std::vector<std::string> Arguments;
    if (ArgumentCount > 1)
    {
      Arguments.assign(ArgumentValues + 1, ArgumentValues + ArgumentCount);
    }
    try
    {
      applyCommandPolicies(App);
      validateUtf8Arguments(Arguments);
      validateOptionSyntax(App, Arguments);
      App.parse(ArgumentCount, ArgumentValues);
      return {};
    }
    catch (const CLI::ParseError &Error)
    {
      return handleParseError(Error, Output, ErrorOutput);
    }
  }

  ParseResult Application::parseArguments(const std::vector<std::string> &Arguments, std::ostream &Output, std::ostream &ErrorOutput)
  {
    std::vector<std::string> ReversedArguments(Arguments.rbegin(), Arguments.rend());
    try
    {
      applyCommandPolicies(App);
      validateUtf8Arguments(Arguments);
      validateOptionSyntax(App, Arguments);
      App.parse(ReversedArguments);
      return {};
    }
    catch (const CLI::ParseError &Error)
    {
      return handleParseError(Error, Output, ErrorOutput);
    }
  }

  ParseResult Application::handleParseError(const CLI::ParseError &Error, std::ostream &Output, std::ostream &ErrorOutput) const
  {
    try
    {
      const int ParserExitCode = App.exit(Error, Output, ErrorOutput);
      Output.flush();
      ErrorOutput.flush();
      if (ParserExitCode == 0 && !Output)
      {
        return {true, ExitCode::InvocationError};
      }
      return {true, ParserExitCode == 0 ? ExitCode::Success : ExitCode::InvocationError};
    }
    catch (const std::ios_base::failure &)
    {
      return {true, ExitCode::InvocationError};
    }
  }

  CLI::Option &setRepeatPolicy(CLI::Option &Option, RepeatPolicy Policy)
  {
    CLI::MultiOptionPolicy CliPolicy = CLI::MultiOptionPolicy::Throw;
    switch (Policy)
    {
      case RepeatPolicy::Single:
        CliPolicy = CLI::MultiOptionPolicy::Throw;
        break;
      case RepeatPolicy::Append:
        CliPolicy = CLI::MultiOptionPolicy::TakeAll;
        break;
      case RepeatPolicy::LastWins:
        CliPolicy = CLI::MultiOptionPolicy::TakeLast;
        break;
      default:
        throw std::invalid_argument("unknown Ink repeat policy");
    }
    if (!Option.nonpositional())
    {
      throw std::invalid_argument("Repeat policies only apply to named options");
    }
    if (Option.get_expected_max() == 0 && Policy != RepeatPolicy::Single)
    {
      throw std::invalid_argument("Boolean flags must use the Single repeat policy");
    }
    if (Option.get_expected_max() == 0)
    {
      if (Option.get_allow_extra_args() || Option.get_delimiter() != '\0' || Option.get_inject_separator())
      {
        throw std::invalid_argument("Boolean flags cannot consume or synthesize values");
      }
    }
    else
    {
      // CLI11 uses this sentinel together with allow_extra_args for vector-bound options; normalize only that library default, never an explicit finite range.
      const bool HasDefaultContainerArity = Option.get_expected_min() == 1 && Option.get_expected_max() == CLI::detail::expected_max_vector_size && Option.get_type_size_min() == 1 && Option.get_type_size_max() == 1 && Option.get_allow_extra_args();
      if (Option.get_expected_min() != 1 || (Option.get_expected_max() != 1 && !HasDefaultContainerArity) || Option.get_type_size_min() != 1 || Option.get_type_size_max() != 1 || (Option.get_allow_extra_args() && !HasDefaultContainerArity) || Option.get_delimiter() != '\0' || Option.get_inject_separator())
      {
        throw std::invalid_argument("Value-taking options must consume exactly one argv value per occurrence");
      }
    }
    Option.multi_option_policy(CliPolicy);
    if (Option.get_expected_max() != 0)
    {
      Option.expected(1)->allow_extra_args(false);
    }
    return Option;
  }

  int runMain(std::string_view ProgramName, const std::function<int()> &Body)
  {
    return runMain(ProgramName, Body, std::cerr);
  }

  int runMain(std::string_view ProgramName, const std::function<int()> &Body, std::ostream &ErrorOutput)
  {
    try
    {
      return Body();
    }
    catch (const InternalCompilerError &Error)
    {
      ErrorOutput << ProgramName << ": internal error: " << Error.what() << '\n';
    }
    catch (const std::exception &Error)
    {
      ErrorOutput << ProgramName << ": internal error: " << Error.what() << '\n';
    }
    catch (...)
    {
      ErrorOutput << ProgramName << ": internal error: unknown exception\n";
    }
    return exitStatus(ExitCode::InternalError);
  }
} // namespace ink::cli
