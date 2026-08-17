#include "ink/cli/application.h"
#include "ink/cli/io.h"

#include <algorithm>
#include <cstddef>
#include <cwchar>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#define INK_CLI_UNDEFINE_NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#ifdef INK_CLI_UNDEFINE_NOMINMAX
#undef INK_CLI_UNDEFINE_NOMINMAX
#undef NOMINMAX
#endif
#endif

namespace ink::cli
{
  namespace
  {
    std::vector<std::string> splitNames(std::string_view Names)
    {
      std::vector<std::string> Result;
      std::size_t Start = 0;
      while (Start <= Names.size())
      {
        const std::size_t Separator = Names.find(',', Start);
        const std::size_t End = Separator == std::string_view::npos ? Names.size() : Separator;
        Result.emplace_back(Names.substr(Start, End - Start));
        if (Separator == std::string_view::npos)
        {
          break;
        }
        Start = Separator + 1;
      }
      return Result;
    }

    bool isAsciiAlphanumeric(char Character) noexcept
    {
      return (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z') || (Character >= '0' && Character <= '9');
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

    bool isPositionalName(std::string_view Name) noexcept
    {
      if (Name.empty() || Name.front() < 'A' || Name.front() > 'Z')
      {
        return false;
      }
      for (char Character : Name)
      {
        if ((Character >= 'A' && Character <= 'Z') || (Character >= '0' && Character <= '9') || Character == '_' || Character == '-')
        {
          continue;
        }
        return false;
      }
      return true;
    }

    bool isKnownRepeatPolicy(RepeatPolicy Policy) noexcept
    {
      switch (Policy)
      {
      case RepeatPolicy::Single:
      case RepeatPolicy::Append:
      case RepeatPolicy::LastWins:
        return true;
      }
      return false;
    }

#ifdef _WIN32
    struct LocalArgumentDeleter
    {
        void operator()(wchar_t **Arguments) const noexcept
        {
          if (Arguments != nullptr)
          {
            LocalFree(Arguments);
          }
        }
    };

    bool processArguments(std::vector<std::string> &Arguments)
    {
      int WideArgumentCount = 0;
      std::unique_ptr<wchar_t *, LocalArgumentDeleter> WideArguments(CommandLineToArgvW(GetCommandLineW(), &WideArgumentCount));
      if (WideArguments == nullptr || WideArgumentCount < 1)
      {
        return false;
      }
      Arguments.reserve(static_cast<std::size_t>(WideArgumentCount - 1));
      for (int Index = 1; Index < WideArgumentCount; ++Index)
      {
        const int WideLength = static_cast<int>(std::wcslen(WideArguments.get()[Index]));
        if (WideLength == 0)
        {
          Arguments.emplace_back();
          continue;
        }
        const int Utf8Length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, WideArguments.get()[Index], WideLength, nullptr, 0, nullptr, nullptr);
        if (Utf8Length <= 0)
        {
          return false;
        }
        std::string Argument(static_cast<std::size_t>(Utf8Length), '\0');
        if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, WideArguments.get()[Index], WideLength, Argument.data(), Utf8Length, nullptr, nullptr) != Utf8Length)
        {
          return false;
        }
        Arguments.push_back(std::move(Argument));
      }
      return true;
    }
#else
    bool processArguments(int ArgumentCount, char **ArgumentValues, std::vector<std::string> &Arguments)
    {
      if (ArgumentCount < 0 || (ArgumentCount > 0 && ArgumentValues == nullptr))
      {
        return false;
      }
      if (ArgumentCount <= 1)
      {
        return true;
      }
      Arguments.reserve(static_cast<std::size_t>(ArgumentCount - 1));
      for (int Index = 1; Index < ArgumentCount; ++Index)
      {
        if (ArgumentValues[Index] == nullptr)
        {
          return false;
        }
        Arguments.emplace_back(ArgumentValues[Index]);
      }
      return true;
    }
#endif
  } // namespace

  class CommandLineParser
  {
    public:
      CommandLineParser(Application &Command, const std::vector<std::string> &Arguments, std::ostream &Output, std::ostream &ErrorOutput)
          : Command(Command),
            Arguments(Arguments),
            Output(Output),
            ErrorOutput(ErrorOutput)
      {
      }

      ParseResult run()
      {
        std::string DefinitionSubject;
        std::string DefinitionMessage;
        if (!validateDefinitions(DefinitionSubject, DefinitionMessage))
        {
          return definitionError(DefinitionSubject, DefinitionMessage);
        }
        for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
        {
          if (!isValidUtf8(Arguments[Index]))
          {
            return invocationError("argument " + std::to_string(Index + 1) + ": is not valid UTF-8");
          }
        }
        for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
        {
          if (!parseArgument(Index))
          {
            return invocationError(ErrorMessage);
          }
        }
        if (!validateParsedOptions())
        {
          return invocationError(ErrorMessage);
        }
        if (HelpSeen)
        {
          return informationalResult(helpText());
        }
        if (VersionSeen)
        {
          return informationalResult(Command.Info.Name + " " + Command.Info.Version + "\n");
        }
        applyValues();
        return {};
      }

    private:
      struct Occurrence
      {
          Option *Definition;
          std::string Spelling;
          std::string Value;
          bool HasValue;
      };

      bool validateDefinitions(std::string &Subject, std::string &Message) const
      {
        if (Command.Info.Name.empty())
        {
          Subject = "application";
          Message = "the application name must not be empty";
          return false;
        }
        if (!isValidUtf8(Command.Info.Name) || !isValidUtf8(Command.Info.Description) || !isValidUtf8(Command.Info.Version))
        {
          Subject = Command.Info.Name;
          Message = "application metadata must be valid UTF-8";
          return false;
        }

        bool SawListPositional = false;
        for (std::size_t OptionIndex = 0; OptionIndex < Command.Options.size(); ++OptionIndex)
        {
          const Option &Definition = *Command.Options[OptionIndex];
          const std::string Label = Definition.Spellings.empty() ? Definition.Names : Definition.Spellings.front();
          if (Definition.Spellings.empty())
          {
            Subject = Label;
            Message = "an option must have at least one name";
            return false;
          }

          const bool IsPositional = Definition.Spellings.front().empty() || Definition.Spellings.front().front() != '-';
          if (IsPositional && Definition.Spellings.size() != 1)
          {
            Subject = Label;
            Message = "positional arguments cannot have aliases";
            return false;
          }
          if (IsPositional && Definition.Kind == Option::ValueKind::Flag)
          {
            Subject = Label;
            Message = "positional arguments must accept a value";
            return false;
          }
          if (IsPositional && SawListPositional)
          {
            Subject = Label;
            Message = "a positional argument cannot follow a string-array positional argument";
            return false;
          }
          if (IsPositional && Definition.Kind == Option::ValueKind::StringList)
          {
            SawListPositional = true;
          }

          for (const std::string &Spelling : Definition.Spellings)
          {
            if (!validateSpelling(Spelling, IsPositional, Subject, Message))
            {
              return false;
            }
            if (Spelling == "-h" || Spelling == "--help" || Spelling == "-V" || Spelling == "--version")
            {
              Subject = Spelling;
              Message = "the help and version option names are reserved";
              return false;
            }
            for (std::size_t PreviousIndex = 0; PreviousIndex < OptionIndex; ++PreviousIndex)
            {
              const Option &Previous = *Command.Options[PreviousIndex];
              if (std::find(Previous.Spellings.begin(), Previous.Spellings.end(), Spelling) != Previous.Spellings.end())
              {
                Subject = Spelling;
                Message = "option names must be unique";
                return false;
              }
            }
          }

          if (!isKnownRepeatPolicy(Definition.Policy))
          {
            Subject = Label;
            Message = "repeat policy must be Single, Append, or LastWins";
            return false;
          }
          if (Definition.Kind == Option::ValueKind::Flag && Definition.Policy != RepeatPolicy::Single)
          {
            Subject = Label;
            Message = "Boolean flags must use the Single repeat policy";
            return false;
          }
          if (Definition.Kind == Option::ValueKind::String && Definition.Policy == RepeatPolicy::Append)
          {
            Subject = Label;
            Message = "Append repeat policy requires string-array storage";
            return false;
          }
          if (IsPositional && Definition.Policy == RepeatPolicy::LastWins)
          {
            Subject = Label;
            Message = "positional arguments cannot use the LastWins repeat policy";
            return false;
          }
          if (Definition.Kind == Option::ValueKind::Flag && Definition.IsRequired)
          {
            Subject = Label;
            Message = "Boolean flags cannot be required";
            return false;
          }
          for (const Option *Excluded : Definition.Exclusions)
          {
            if (Excluded == &Definition || Excluded == nullptr || Excluded->Owner != &Command)
            {
              Subject = Label;
              Message = "excluded options must be distinct options from the same application";
              return false;
            }
          }
        }
        return true;
      }

      bool validateSpelling(const std::string &Spelling, bool IsPositional, std::string &Subject, std::string &Message) const
      {
        Subject = Spelling.empty() ? "empty option name" : Spelling;
        if (IsPositional)
        {
          if (!isPositionalName(Spelling))
          {
            Message = "positional argument names must use uppercase ASCII identifier spelling";
            return false;
          }
          return true;
        }
        if (Spelling.size() >= 3 && Spelling.substr(0, 2) == "--")
        {
          if (!isLowerKebabCase(std::string_view(Spelling).substr(2)))
          {
            Message = "long option names must use lowercase ASCII kebab-case";
            return false;
          }
          return true;
        }
        if (Spelling.size() == 2 && Spelling.front() == '-' && isAsciiAlphanumeric(Spelling[1]))
        {
          return true;
        }
        if (Spelling.size() > 2 && Spelling.front() == '-' && isLowerKebabCase(std::string_view(Spelling).substr(1)))
        {
          return true;
        }
        Message = "single-dash option names must be one ASCII letter or digit, or a lowercase ASCII name";
        return false;
      }

      bool parseArgument(std::size_t &Index)
      {
        const std::string &Argument = Arguments[Index];
        if (!AfterOptionTerminator && Argument == "--")
        {
          AfterOptionTerminator = true;
          return true;
        }
        if (!AfterOptionTerminator && Argument != "-" && !Argument.empty() && Argument.front() == '-')
        {
          return parseNamedArgument(Index);
        }
        return parsePositionalArgument(Argument);
      }

      bool parseNamedArgument(std::size_t &Index)
      {
        const std::string &Argument = Arguments[Index];
        const std::size_t Equals = Argument.find('=');
        if (Equals != std::string::npos)
        {
          const std::string Name = Argument.substr(0, Equals);
          if (Name == "-h" || Name == "--help" || Name == "-V" || Name == "--version")
          {
            ErrorMessage = "option '" + Name + "' does not accept a value";
            return false;
          }
          if (Option *Definition = findOption(Name))
          {
            return recordOption(*Definition, Name, Argument.substr(Equals + 1), true);
          }
          if (Argument.size() >= 2 && Argument[1] == '-')
          {
            ErrorMessage = "unrecognized option '" + Name + "'";
            return false;
          }
        }

        if (Argument == "-h" || Argument == "--help")
        {
          return recordInformationalOption(true, Argument);
        }
        if (Argument == "-V" || Argument == "--version")
        {
          return recordInformationalOption(false, Argument);
        }
        if (Option *Definition = findOption(Argument))
        {
          return recordNamedOption(*Definition, Argument, Index, {});
        }
        if (Argument.size() >= 2 && Argument[1] == '-')
        {
          ErrorMessage = "unrecognized option '" + Argument + "'";
          return false;
        }
        return parseShortOptionCluster(Index);
      }

      bool parseShortOptionCluster(std::size_t &Index)
      {
        const std::string &Argument = Arguments[Index];
        for (std::size_t CharacterIndex = 1; CharacterIndex < Argument.size(); ++CharacterIndex)
        {
          const std::string Name = std::string("-") + Argument[CharacterIndex];
          if (Name == "-h")
          {
            if (!recordInformationalOption(true, Name))
            {
              return false;
            }
            continue;
          }
          if (Name == "-V")
          {
            if (!recordInformationalOption(false, Name))
            {
              return false;
            }
            continue;
          }
          Option *Definition = findOption(Name);
          if (Definition == nullptr)
          {
            ErrorMessage = "unrecognized option '" + Argument + "'";
            return false;
          }
          if (Definition->Kind == Option::ValueKind::Flag)
          {
            if (!recordOption(*Definition, Name, {}, false))
            {
              return false;
            }
            continue;
          }
          std::string AttachedValue;
          if (CharacterIndex + 1 < Argument.size())
          {
            AttachedValue = Argument.substr(CharacterIndex + 1);
            if (!AttachedValue.empty() && AttachedValue.front() == '=')
            {
              AttachedValue.erase(0, 1);
            }
          }
          return recordNamedOption(*Definition, Name, Index, std::move(AttachedValue));
        }
        return true;
      }

      bool parsePositionalArgument(const std::string &Argument)
      {
        Option *Definition = nextPositionalOption();
        if (Definition == nullptr)
        {
          ErrorMessage = "unexpected argument '" + Argument + "'";
          return false;
        }
        if (!recordOption(*Definition, Definition->Spellings.front(), Argument, true))
        {
          return false;
        }
        if (Definition->Kind != Option::ValueKind::StringList)
        {
          ++PositionalIndex;
        }
        return true;
      }

      bool recordNamedOption(Option &Definition, const std::string &Spelling, std::size_t &Index, std::string AttachedValue)
      {
        if (Definition.Kind == Option::ValueKind::Flag)
        {
          if (!AttachedValue.empty())
          {
            ErrorMessage = "option '" + Spelling + "' does not accept a value";
            return false;
          }
          return recordOption(Definition, Spelling, {}, false);
        }
        if (!AttachedValue.empty())
        {
          return recordOption(Definition, Spelling, std::move(AttachedValue), true);
        }
        if (Index + 1 >= Arguments.size())
        {
          ErrorMessage = "option '" + Spelling + "' is missing a value";
          return false;
        }
        const std::string &Value = Arguments[Index + 1];
        if (Value.empty() || Value == "--" || (Value.size() > 1 && Value.front() == '-'))
        {
          ErrorMessage = "option '" + Spelling + "' is missing a value; dash-leading values must use attached form";
          return false;
        }
        ++Index;
        return recordOption(Definition, Spelling, Value, true);
      }

      bool recordOption(Option &Definition, std::string Spelling, std::string Value, bool HasValue)
      {
        if (Definition.Kind == Option::ValueKind::Flag && HasValue)
        {
          ErrorMessage = "option '" + Spelling + "' does not accept a value";
          return false;
        }
        if (Definition.Kind != Option::ValueKind::Flag && (!HasValue || Value.empty()))
        {
          ErrorMessage = "option '" + Spelling + "' is missing a value";
          return false;
        }
        if (Definition.Policy == RepeatPolicy::Single && hasOccurrence(Definition))
        {
          ErrorMessage = "option '" + Spelling + "' may be specified at most once";
          return false;
        }
        Occurrences.push_back({&Definition, std::move(Spelling), std::move(Value), HasValue});
        return true;
      }

      bool recordInformationalOption(bool IsHelp, const std::string &Spelling)
      {
        bool &Seen = IsHelp ? HelpSeen : VersionSeen;
        if (Seen)
        {
          ErrorMessage = "option '" + Spelling + "' may be specified at most once";
          return false;
        }
        Seen = true;
        return true;
      }

      bool validateParsedOptions()
      {
        if (HelpSeen && VersionSeen)
        {
          ErrorMessage = "option '--help' conflicts with '--version'";
          return false;
        }
        for (const std::unique_ptr<Option> &Definition : Command.Options)
        {
          if (!hasOccurrence(*Definition))
          {
            if (!HelpSeen && !VersionSeen && Definition->IsRequired)
            {
              ErrorMessage = "required " + std::string(isPositional(*Definition) ? "argument '" : "option '") + Definition->Spellings.front() + "' is missing";
              return false;
            }
            continue;
          }
          for (const Option *Excluded : Definition->Exclusions)
          {
            if (hasOccurrence(*Excluded))
            {
              ErrorMessage = "option '" + Definition->Spellings.front() + "' conflicts with '" + Excluded->Spellings.front() + "'";
              return false;
            }
          }
        }
        return true;
      }

      void applyValues()
      {
        for (const std::unique_ptr<Option> &Definition : Command.Options)
        {
          if (!hasOccurrence(*Definition))
          {
            continue;
          }
          if (Definition->Kind == Option::ValueKind::Flag)
          {
            *Definition->FlagStorage = true;
            continue;
          }
          if (Definition->Kind == Option::ValueKind::String)
          {
            for (const Occurrence &Entry : Occurrences)
            {
              if (Entry.Definition == Definition.get())
              {
                *Definition->StringStorage = Entry.Value;
              }
            }
            continue;
          }

          if (Definition->Policy == RepeatPolicy::LastWins)
          {
            Definition->StringListStorage->clear();
            for (auto Iterator = Occurrences.rbegin(); Iterator != Occurrences.rend(); ++Iterator)
            {
              if (Iterator->Definition == Definition.get())
              {
                Definition->StringListStorage->push_back(Iterator->Value);
                break;
              }
            }
            continue;
          }
          for (const Occurrence &Entry : Occurrences)
          {
            if (Entry.Definition == Definition.get())
            {
              Definition->StringListStorage->push_back(Entry.Value);
            }
          }
        }
      }

      std::string helpText() const
      {
        std::ostringstream Text;
        Text << Command.Info.Description << "\n\nUsage: " << Command.Info.Name << " [OPTIONS]";
        for (const std::unique_ptr<Option> &Definition : Command.Options)
        {
          if (isPositional(*Definition))
          {
            Text << (Definition->IsRequired ? " " : " [") << Definition->Spellings.front();
            if (!Definition->IsRequired)
            {
              Text << ']';
            }
          }
        }
        Text << "\n\nOptions:\n  -h, --help  Print help information and exit\n  -V, --version  Print version information and exit\n";
        for (const std::unique_ptr<Option> &Definition : Command.Options)
        {
          if (isPositional(*Definition))
          {
            continue;
          }
          Text << "  ";
          for (std::size_t Index = 0; Index < Definition->Spellings.size(); ++Index)
          {
            if (Index != 0)
            {
              Text << ", ";
            }
            Text << Definition->Spellings[Index];
          }
          if (Definition->Kind != Option::ValueKind::Flag)
          {
            Text << ' ' << (Definition->ValueTypeName.empty() ? "TEXT" : Definition->ValueTypeName);
          }
          if (Definition->IsRequired)
          {
            Text << " REQUIRED";
          }
          if (!Definition->Description.empty())
          {
            Text << "  " << Definition->Description;
          }
          Text << '\n';
        }
        return Text.str();
      }

      Option *findOption(std::string_view Spelling) const noexcept
      {
        for (const std::unique_ptr<Option> &Definition : Command.Options)
        {
          if (!isPositional(*Definition) && std::find(Definition->Spellings.begin(), Definition->Spellings.end(), Spelling) != Definition->Spellings.end())
          {
            return Definition.get();
          }
        }
        return nullptr;
      }

      Option *nextPositionalOption() const noexcept
      {
        std::size_t Current = 0;
        for (const std::unique_ptr<Option> &Definition : Command.Options)
        {
          if (!isPositional(*Definition))
          {
            continue;
          }
          if (Current == PositionalIndex)
          {
            return Definition.get();
          }
          ++Current;
        }
        return nullptr;
      }

      bool hasOccurrence(const Option &Definition) const noexcept
      {
        return std::any_of(Occurrences.begin(), Occurrences.end(), [&Definition](const Occurrence &Entry)
                           {
                             return Entry.Definition == &Definition;
                           });
      }

      bool isPositional(const Option &Definition) const noexcept
      {
        return !Definition.Spellings.empty() && !Definition.Spellings.front().empty() && Definition.Spellings.front().front() != '-';
      }

      ParseResult informationalResult(const std::string &Text) const
      {
        return {true, writeOutput(Output, Text) ? ExitCode::Success : ExitCode::InvocationError};
      }

      ParseResult invocationError(const std::string &Message) const
      {
        writeOutput(ErrorOutput, Command.Info.Name + ": error: " + Message + "\nTry '" + Command.Info.Name + " --help' for more information.\n");
        return {true, ExitCode::InvocationError};
      }

      ParseResult definitionError(const std::string &Subject, const std::string &Message) const
      {
        writeOutput(ErrorOutput, Command.Info.Name + ": internal error: invalid command-line definition for '" + Subject + "': " + Message + "\n");
        return {true, ExitCode::InternalError};
      }

      Application &Command;
      const std::vector<std::string> &Arguments;
      std::ostream &Output;
      std::ostream &ErrorOutput;
      std::vector<Occurrence> Occurrences;
      std::string ErrorMessage;
      std::size_t PositionalIndex = 0;
      bool AfterOptionTerminator = false;
      bool HelpSeen = false;
      bool VersionSeen = false;
  };

  Option::Option(Application &OwnerValue, std::string NamesValue, bool &Storage, std::string DescriptionValue)
      : Owner(&OwnerValue),
        Names(std::move(NamesValue)),
        Description(std::move(DescriptionValue)),
        Kind(ValueKind::Flag),
        FlagStorage(&Storage)
  {
    Spellings = splitNames(Names);
  }

  Option::Option(Application &OwnerValue, std::string NamesValue, std::string &Storage, std::string DescriptionValue)
      : Owner(&OwnerValue),
        Names(std::move(NamesValue)),
        Description(std::move(DescriptionValue)),
        Kind(ValueKind::String),
        StringStorage(&Storage)
  {
    Spellings = splitNames(Names);
  }

  Option::Option(Application &OwnerValue, std::string NamesValue, std::vector<std::string> &Storage, std::string DescriptionValue)
      : Owner(&OwnerValue),
        Names(std::move(NamesValue)),
        Description(std::move(DescriptionValue)),
        Kind(ValueKind::StringList),
        StringListStorage(&Storage),
        Policy(RepeatPolicy::Append)
  {
    Spellings = splitNames(Names);
  }

  Option &Option::required() noexcept
  {
    IsRequired = true;
    return *this;
  }

  Option &Option::typeName(std::string Name)
  {
    ValueTypeName = std::move(Name);
    return *this;
  }

  Option &Option::repeatPolicy(RepeatPolicy PolicyValue) noexcept
  {
    Policy = PolicyValue;
    return *this;
  }

  Option &Option::excludes(Option &Other)
  {
    Exclusions.push_back(&Other);
    return *this;
  }

  Application::Application(ApplicationInfo InfoValue)
      : Info(std::move(InfoValue))
  {
  }

  Option &Application::addFlag(std::string Names, bool &Storage, std::string Description)
  {
    Options.push_back(std::unique_ptr<Option>(new Option(*this, std::move(Names), Storage, std::move(Description))));
    return *Options.back();
  }

  Option &Application::addOption(std::string Names, std::string &Storage, std::string Description)
  {
    Options.push_back(std::unique_ptr<Option>(new Option(*this, std::move(Names), Storage, std::move(Description))));
    return *Options.back();
  }

  Option &Application::addOption(std::string Names, std::vector<std::string> &Storage, std::string Description)
  {
    Options.push_back(std::unique_ptr<Option>(new Option(*this, std::move(Names), Storage, std::move(Description))));
    return *Options.back();
  }

  ParseResult Application::parse(int ArgumentCount, char **ArgumentValues)
  {
    return parse(ArgumentCount, ArgumentValues, std::cout, std::cerr);
  }

  ParseResult Application::parse(int ArgumentCount, char **ArgumentValues, std::ostream &Output, std::ostream &ErrorOutput)
  {
    if (ArgumentCount == 0 && ArgumentValues == nullptr)
    {
      return parseArguments({}, Output, ErrorOutput);
    }
    std::vector<std::string> Arguments;
#ifdef _WIN32
    if (ArgumentCount < 0 || ArgumentValues == nullptr || !processArguments(Arguments))
#else
    if (!processArguments(ArgumentCount, ArgumentValues, Arguments))
#endif
    {
      writeOutput(ErrorOutput, Info.Name + ": error: cannot decode process arguments as UTF-8\nTry '" + Info.Name + " --help' for more information.\n");
      return {true, ExitCode::InvocationError};
    }
    return parseArguments(Arguments, Output, ErrorOutput);
  }

  ParseResult Application::parseArguments(const std::vector<std::string> &Arguments, std::ostream &Output, std::ostream &ErrorOutput)
  {
    return CommandLineParser(*this, Arguments, Output, ErrorOutput).run();
  }

  int runMain(std::string_view ProgramName, const std::function<int()> &Body)
  {
    return runMain(ProgramName, Body, std::cerr);
  }

  int runMain(std::string_view ProgramName, const std::function<int()> &Body, std::ostream &ErrorOutput)
  {
    if (!Body)
    {
      writeOutput(ErrorOutput, std::string(ProgramName) + ": internal error: process body is empty\n");
      return exitStatus(ExitCode::InternalError);
    }
    return Body();
  }
} // namespace ink::cli
