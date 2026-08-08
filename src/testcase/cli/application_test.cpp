#include "ink/cli/application.h"
#include "ink/cli/io.h"

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ink::cli
{
  namespace
  {
    Application makeApplication()
    {
      return Application({"ink-test", "Exercise the shared Ink command-line policy.", "development"});
    }

    // Verifies that every public Ink process uses the stable 0/1/2/3 exit status contract.
    TEST(ApplicationTest, ExposesStableExitStatusValues)
    {
      EXPECT_EQ(exitStatus(ExitCode::Success), 0);
      EXPECT_EQ(exitStatus(ExitCode::SourceError), 1);
      EXPECT_EQ(exitStatus(ExitCode::InvocationError), 2);
      EXPECT_EQ(exitStatus(ExitCode::InternalError), 3);
    }

    // Verifies that the shared process boundary maps unexpected exceptions to a concise internal error and exit status 3.
    TEST(ApplicationTest, MapsUnhandledExceptionsToInternalErrors)
    {
      std::ostringstream ErrorOutput;
      const int Result = runMain("ink-test", []() -> int
      {
        throw std::runtime_error("broken invariant");
      }, ErrorOutput);

      EXPECT_EQ(Result, exitStatus(ExitCode::InternalError));
      EXPECT_EQ(ErrorOutput.str(), "ink-test: internal error: broken invariant\n");
    }

    // Verifies that help is a successful primary result written only to stdout.
    TEST(ApplicationTest, WritesHelpToStandardOutput)
    {
      Application Command = makeApplication();
      std::string RequiredInput;
      Command.app().add_option("INPUT", RequiredInput)->required();
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"--help"}, Output, ErrorOutput);

      EXPECT_TRUE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::Success);
      EXPECT_NE(Output.str().find("Exercise the shared Ink command-line policy."), std::string::npos);
      EXPECT_NE(Output.str().find("--help"), std::string::npos);
      EXPECT_TRUE(ErrorOutput.str().empty());
    }

    // Verifies that a failed help-output stream changes the successful informational result into an I/O invocation error.
    TEST(ApplicationTest, RejectsFailedInformationalOutput)
    {
      Application Command = makeApplication();
      std::ostringstream Output;
      Output.setstate(std::ios::badbit);
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"--help"}, Output, ErrorOutput);

      EXPECT_TRUE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::InvocationError);
      EXPECT_TRUE(ErrorOutput.str().empty());
    }

    // Verifies that version uses the reserved -V/--version spelling and exits successfully.
    TEST(ApplicationTest, WritesVersionToStandardOutput)
    {
      Application Command = makeApplication();
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"-V"}, Output, ErrorOutput);

      EXPECT_TRUE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::Success);
      EXPECT_EQ(Output.str(), "ink-test development\n");
      EXPECT_TRUE(ErrorOutput.str().empty());
    }

    // Verifies that the process entry point safely accepts the argc-zero form permitted by CLI11.
    TEST(ApplicationTest, AcceptsAnEmptyProcessArgumentVector)
    {
      Application Command = makeApplication();
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parse(0, nullptr, Output, ErrorOutput);

      EXPECT_FALSE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::Success);
      EXPECT_TRUE(Output.str().empty());
      EXPECT_TRUE(ErrorOutput.str().empty());
    }

    // Verifies that unknown options become Ink invocation errors with the shared concise diagnostic.
    TEST(ApplicationTest, MapsUnknownOptionsToInvocationErrors)
    {
      Application Command = makeApplication();
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"--unknown"}, Output, ErrorOutput);

      EXPECT_TRUE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::InvocationError);
      EXPECT_TRUE(Output.str().empty());
      EXPECT_NE(ErrorOutput.str().find("ink-test: error:"), std::string::npos);
      EXPECT_NE(ErrorOutput.str().find("Try 'ink-test --help'"), std::string::npos);
    }

    // Verifies that Windows slash options are disabled so parsing policy is identical on every host.
    TEST(ApplicationTest, RejectsWindowsStyleOptionsWhenNoOperandAcceptsThem)
    {
      Application Command = makeApplication();
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"/help"}, Output, ErrorOutput);

      EXPECT_TRUE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::InvocationError);
    }

    // Verifies that option names are case-sensitive and do not accept underscore or prefix aliases.
    TEST(ApplicationTest, RejectsUnregisteredOptionSpellings)
    {
      const std::vector<std::string> InvalidSpellings = {"--Feature-name", "--feature_name", "--feature"};
      for (const std::string &Spelling : InvalidSpellings)
      {
        Application Command = makeApplication();
        bool FeatureEnabled = false;
        Command.app().add_flag("--feature-name", FeatureEnabled);
        std::ostringstream Output;
        std::ostringstream ErrorOutput;
        const ParseResult Result = Command.parseArguments({Spelling}, Output, ErrorOutput);
        EXPECT_TRUE(Result.ShouldExit) << Spelling;
        EXPECT_EQ(Result.Code, ExitCode::InvocationError) << Spelling;
      }
    }

    // Verifies that direct CLI11 customization cannot re-enable permissive parsing or register nonstandard option names.
    TEST(ApplicationTest, EnforcesPoliciesAfterDirectParserCustomization)
    {
      Application SpellingCommand = makeApplication();
      bool FeatureEnabled = false;
      CLI::Option *FeatureOption = SpellingCommand.app().add_flag("--feature-name", FeatureEnabled);
      SpellingCommand.app().ignore_case(true);
      FeatureOption->ignore_case(true);
      std::ostringstream SpellingOutput;
      std::ostringstream SpellingErrorOutput;
      const ParseResult SpellingResult = SpellingCommand.parseArguments({"--FEATURE-NAME"}, SpellingOutput, SpellingErrorOutput);
      EXPECT_TRUE(SpellingResult.ShouldExit);
      EXPECT_EQ(SpellingResult.Code, ExitCode::InvocationError);

      Application DefinitionCommand = makeApplication();
      bool InvalidEnabled = false;
      DefinitionCommand.app().add_flag("--Invalid_name", InvalidEnabled);
      std::ostringstream DefinitionOutput;
      std::ostringstream DefinitionErrorOutput;
      EXPECT_THROW(DefinitionCommand.parseArguments({"--help"}, DefinitionOutput, DefinitionErrorOutput), std::logic_error);
      EXPECT_TRUE(DefinitionOutput.str().empty());

      Application ShortNameCommand = makeApplication();
      bool ShortNameEnabled = false;
      ShortNameCommand.app().allow_non_standard_option_names();
      ShortNameCommand.app().add_flag("-feature", ShortNameEnabled);
      std::ostringstream ShortNameOutput;
      std::ostringstream ShortNameErrorOutput;
      EXPECT_THROW(ShortNameCommand.parseArguments({"--help"}, ShortNameOutput, ShortNameErrorOutput), std::logic_error);
      EXPECT_TRUE(ShortNameOutput.str().empty());

      Application PrefixCommand = makeApplication();
      PrefixCommand.app().prefix_command();
      std::ostringstream PrefixOutput;
      std::ostringstream PrefixErrorOutput;
      const ParseResult PrefixResult = PrefixCommand.parseArguments({"unexpected", "arguments"}, PrefixOutput, PrefixErrorOutput);
      EXPECT_TRUE(PrefixResult.ShouldExit);
      EXPECT_EQ(PrefixResult.Code, ExitCode::InvocationError);
    }

    // Verifies that invalid command definitions cross the shared process boundary as internal errors instead of user invocation errors.
    TEST(ApplicationTest, MapsInvalidDefinitionsToInternalErrors)
    {
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const int Result = runMain("ink-test", [&Output, &ErrorOutput]()
      {
        Application Command = makeApplication();
        bool InvalidEnabled = false;
        Command.app().add_flag("--Invalid_name", InvalidEnabled);
        Command.parseArguments({"--help"}, Output, ErrorOutput);
        return exitStatus(ExitCode::Success);
      }, ErrorOutput);

      EXPECT_EQ(Result, exitStatus(ExitCode::InternalError));
      EXPECT_TRUE(Output.str().empty());
      EXPECT_NE(ErrorOutput.str().find("ink-test: internal error: invalid command-line definition"), std::string::npos);
      EXPECT_EQ(ErrorOutput.str().find("Try 'ink-test --help'"), std::string::npos);
    }

    // Verifies that scalar options reject duplicates instead of silently selecting one value.
    TEST(ApplicationTest, RejectsRepeatedScalarOptions)
    {
      Application Command = makeApplication();
      std::string OutputPath;
      Command.app().add_option("-o,--output", OutputPath)->type_name("FILE");
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"--output", "first", "-o", "second"}, Output, ErrorOutput);

      EXPECT_TRUE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::InvocationError);
    }

    // Verifies that Ink Append preserves order while LastWins keeps the final value of a scalar option.
    TEST(ApplicationTest, AppliesInkRepeatPolicies)
    {
      Application Command = makeApplication();
      std::vector<std::string> IncludePaths;
      std::string OptimizationLevel;
      setRepeatPolicy(*Command.app().add_option("-I,--include", IncludePaths), RepeatPolicy::Append);
      setRepeatPolicy(*Command.app().add_option("--optimization-level", OptimizationLevel), RepeatPolicy::LastWins);
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"-Ifirst", "--include", "second", "--optimization-level=0", "--optimization-level", "3"}, Output, ErrorOutput);

      EXPECT_FALSE(Result.ShouldExit);
      EXPECT_EQ(IncludePaths, (std::vector<std::string>{"first", "second"}));
      EXPECT_EQ(OptimizationLevel, "3");
    }

    // Verifies that raw CLI11 repeat policies outside Ink's three public policies are rejected.
    TEST(ApplicationTest, RejectsUnsupportedRepeatPolicies)
    {
      Application Command = makeApplication();
      std::string Value;
      Command.app().add_option("--value", Value)->take_first();
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      EXPECT_THROW(Command.parseArguments({"--value", "first"}, Output, ErrorOutput), std::logic_error);
      EXPECT_TRUE(Value.empty());
    }

    // Verifies that the Ink repeat-policy API only permits Single for Boolean flags.
    TEST(ApplicationTest, RejectsRepeatableBooleanFlags)
    {
      Application Command = makeApplication();
      bool FeatureEnabled = false;
      CLI::Option *FeatureOption = Command.app().add_flag("--feature-name", FeatureEnabled);

      EXPECT_THROW(setRepeatPolicy(*FeatureOption, RepeatPolicy::Append), std::invalid_argument);
      EXPECT_THROW(setRepeatPolicy(*FeatureOption, RepeatPolicy::LastWins), std::invalid_argument);

      Application RawCommand = makeApplication();
      bool RawFeatureEnabled = false;
      RawCommand.app().add_flag("--feature-name", RawFeatureEnabled)->take_first();
      std::ostringstream RawOutput;
      std::ostringstream RawErrorOutput;
      EXPECT_THROW(RawCommand.parseArguments({"--help"}, RawOutput, RawErrorOutput), std::logic_error);
    }

    // Verifies that a required option value cannot consume an empty value, a following option, or the option terminator.
    TEST(ApplicationTest, RejectsInvalidSeparatedOptionValues)
    {
      const std::vector<std::vector<std::string>> InvalidArguments = {
          {"--output="},
          {"--output=", "--help"},
          {"--output", ""},
          {"-o", ""},
          {"--output", "--unknown"},
          {"--output", "--feature-name"},
          {"-o", "--help"},
          {"--output", "--help", "--version"},
          {"--output", "--", "--unknown", "--help"},
      };
      for (const std::vector<std::string> &Arguments : InvalidArguments)
      {
        Application Command = makeApplication();
        std::string OutputPath;
        bool FeatureEnabled = false;
        Command.app().add_option("-o,--output", OutputPath);
        Command.app().add_flag("--feature-name", FeatureEnabled);
        std::ostringstream Output;
        std::ostringstream ErrorOutput;
        const ParseResult Result = Command.parseArguments(Arguments, Output, ErrorOutput);
        EXPECT_TRUE(Result.ShouldExit);
        EXPECT_EQ(Result.Code, ExitCode::InvocationError);
        EXPECT_TRUE(Output.str().empty());
        EXPECT_TRUE(OutputPath.empty());
        EXPECT_FALSE(FeatureEnabled);
      }
    }

    // Verifies that v0 rejects optional-value options instead of guessing whether the next token is their value.
    TEST(ApplicationTest, RejectsOptionalOptionValues)
    {
      Application Command = makeApplication();
      std::vector<std::string> Values;
      Command.app().add_option("--maybe", Values)->expected(0, 1);
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      EXPECT_THROW(Command.parseArguments({"--help"}, Output, ErrorOutput), std::logic_error);
      EXPECT_TRUE(Output.str().empty());
      EXPECT_TRUE(Values.empty());
    }

    // Verifies that every value-taking non-positional option consumes exactly one physical argv token per occurrence.
    TEST(ApplicationTest, RejectsNonUnitOptionValueDefinitions)
    {
      Application VectorCommand = makeApplication();
      std::vector<std::string> VectorValues;
      VectorCommand.app().add_option("--value", VectorValues);
      std::ostringstream VectorOutput;
      std::ostringstream VectorErrorOutput;
      EXPECT_THROW(VectorCommand.parseArguments({"--help"}, VectorOutput, VectorErrorOutput), std::logic_error);

      Application RangeCommand = makeApplication();
      std::string RangeValue;
      RangeCommand.app().add_option("--value", RangeValue)->expected(1, 2);
      std::ostringstream RangeOutput;
      std::ostringstream RangeErrorOutput;
      EXPECT_THROW(RangeCommand.parseArguments({"--help"}, RangeOutput, RangeErrorOutput), std::logic_error);

      Application ExtraCommand = makeApplication();
      std::string ExtraValue;
      ExtraCommand.app().add_option("--value", ExtraValue)->allow_extra_args();
      std::ostringstream ExtraOutput;
      std::ostringstream ExtraErrorOutput;
      EXPECT_THROW(ExtraCommand.parseArguments({"--help"}, ExtraOutput, ExtraErrorOutput), std::logic_error);

      Application DelimiterCommand = makeApplication();
      std::string DelimiterValue;
      DelimiterCommand.app().add_option("--value", DelimiterValue)->delimiter(',');
      std::ostringstream DelimiterOutput;
      std::ostringstream DelimiterErrorOutput;
      EXPECT_THROW(DelimiterCommand.parseArguments({"--help"}, DelimiterOutput, DelimiterErrorOutput), std::logic_error);

      Application SeparatorCommand = makeApplication();
      std::string SeparatorValue;
      SeparatorCommand.app().add_option("--value", SeparatorValue)->inject_separator();
      std::ostringstream SeparatorOutput;
      std::ostringstream SeparatorErrorOutput;
      EXPECT_THROW(SeparatorCommand.parseArguments({"--help"}, SeparatorOutput, SeparatorErrorOutput), std::logic_error);

      Application PolicyCommand = makeApplication();
      std::string PolicyValue;
      CLI::Option *PolicyOption = PolicyCommand.app().add_option("--value", PolicyValue)->expected(0, 1);
      EXPECT_THROW(setRepeatPolicy(*PolicyOption, RepeatPolicy::LastWins), std::invalid_argument);
      EXPECT_EQ(PolicyOption->get_expected_min(), 0);
      EXPECT_EQ(PolicyOption->get_expected_max(), 1);

      Application RangePolicyCommand = makeApplication();
      std::string RangePolicyValue;
      CLI::Option *RangePolicyOption = RangePolicyCommand.app().add_option("--value", RangePolicyValue)->expected(1, 2);
      EXPECT_THROW(setRepeatPolicy(*RangePolicyOption, RepeatPolicy::Append), std::invalid_argument);
      EXPECT_EQ(RangePolicyOption->get_expected_min(), 1);
      EXPECT_EQ(RangePolicyOption->get_expected_max(), 2);
    }

    // Verifies that CLI11 environment and config-file injection cannot become implicit Ink argument sources.
    TEST(ApplicationTest, RejectsImplicitConfigurationSources)
    {
      Application EnvironmentCommand = makeApplication();
      std::string EnvironmentValue;
      EnvironmentCommand.app().add_option("--value", EnvironmentValue)->envname("INK_TEST_VALUE");
      std::ostringstream EnvironmentOutput;
      std::ostringstream EnvironmentErrorOutput;
      EXPECT_THROW(EnvironmentCommand.parseArguments({"--help"}, EnvironmentOutput, EnvironmentErrorOutput), std::logic_error);

      Application ConfigCommand = makeApplication();
      ConfigCommand.app().set_config("--config");
      std::ostringstream ConfigOutput;
      std::ostringstream ConfigErrorOutput;
      EXPECT_THROW(ConfigCommand.parseArguments({"--help"}, ConfigOutput, ConfigErrorOutput), std::logic_error);
    }

    // Verifies that every Append occurrence consumes one value and leaves the following source operand for the command.
    TEST(ApplicationTest, LimitsEachAppendOccurrenceToOneValue)
    {
      Application AcceptedCommand = makeApplication();
      std::vector<std::string> IncludePaths;
      std::string Input;
      setRepeatPolicy(*AcceptedCommand.app().add_option("--include", IncludePaths), RepeatPolicy::Append);
      AcceptedCommand.app().add_option("INPUT", Input)->required();
      std::ostringstream AcceptedOutput;
      std::ostringstream AcceptedErrorOutput;
      const ParseResult AcceptedResult = AcceptedCommand.parseArguments({"--include", "first", "source.ink"}, AcceptedOutput, AcceptedErrorOutput);
      EXPECT_FALSE(AcceptedResult.ShouldExit);
      EXPECT_EQ(IncludePaths, (std::vector<std::string>{"first"}));
      EXPECT_EQ(Input, "source.ink");

      Application RejectedCommand = makeApplication();
      std::vector<std::string> RejectedIncludePaths;
      setRepeatPolicy(*RejectedCommand.app().add_option("--include", RejectedIncludePaths), RepeatPolicy::Append);
      std::ostringstream RejectedOutput;
      std::ostringstream RejectedErrorOutput;
      const ParseResult RejectedResult = RejectedCommand.parseArguments({"--include", "first", ""}, RejectedOutput, RejectedErrorOutput);
      EXPECT_TRUE(RejectedResult.ShouldExit);
      EXPECT_EQ(RejectedResult.Code, ExitCode::InvocationError);
      EXPECT_EQ(RejectedIncludePaths, (std::vector<std::string>{"first"}));
    }

    // Verifies attached dash-leading values and the separated standard-stream marker for long and short options.
    TEST(ApplicationTest, AcceptsExplicitDashLeadingOptionValues)
    {
      Application LongCommand = makeApplication();
      std::string LongOutputPath;
      bool FeatureEnabled = false;
      LongCommand.app().add_option("-o,--output", LongOutputPath);
      LongCommand.app().add_flag("--feature-name", FeatureEnabled);
      std::ostringstream LongOutput;
      std::ostringstream LongErrorOutput;
      const ParseResult LongResult = LongCommand.parseArguments({"--output=--feature-name"}, LongOutput, LongErrorOutput);
      EXPECT_FALSE(LongResult.ShouldExit);
      EXPECT_EQ(LongOutputPath, "--feature-name");
      EXPECT_FALSE(FeatureEnabled);

      Application ShortCommand = makeApplication();
      std::string ShortOutputPath;
      ShortCommand.app().add_option("-o,--output", ShortOutputPath);
      std::ostringstream ShortOutput;
      std::ostringstream ShortErrorOutput;
      const ParseResult ShortResult = ShortCommand.parseArguments({"-o-"}, ShortOutput, ShortErrorOutput);
      EXPECT_FALSE(ShortResult.ShouldExit);
      EXPECT_EQ(ShortOutputPath, "-");

      Application StreamCommand = makeApplication();
      std::string StreamOutputPath;
      StreamCommand.app().add_option("-o,--output", StreamOutputPath);
      std::ostringstream StreamOutput;
      std::ostringstream StreamErrorOutput;
      const ParseResult StreamResult = StreamCommand.parseArguments({"-o", "-"}, StreamOutput, StreamErrorOutput);
      EXPECT_FALSE(StreamResult.ShouldExit);
      EXPECT_EQ(StreamOutputPath, "-");
    }

    // Verifies that Boolean flags are valueless and use the default single-occurrence policy.
    TEST(ApplicationTest, RejectsFlagValuesAndRepeatedFlags)
    {
      const std::vector<std::vector<std::string>> InvalidArguments = {
          {"--feature-name=true"},
          {"--feature-name", "--feature-name"},
          {"-h", "--help"},
          {"-V", "--version"},
          {"--help", "--version"},
      };
      for (const std::vector<std::string> &Arguments : InvalidArguments)
      {
        Application Command = makeApplication();
        bool FeatureEnabled = false;
        Command.app().add_flag("--feature-name", FeatureEnabled);
        std::ostringstream Output;
        std::ostringstream ErrorOutput;
        const ParseResult Result = Command.parseArguments(Arguments, Output, ErrorOutput);
        EXPECT_TRUE(Result.ShouldExit);
        EXPECT_EQ(Result.Code, ExitCode::InvocationError);
      }
    }

    // Verifies that informational flags do not conceal unknown option spelling mistakes.
    TEST(ApplicationTest, RejectsUnknownOptionsEvenWhenHelpOrVersionIsPresent)
    {
      const std::vector<std::vector<std::string>> InvalidArguments = {
          {"--unknown", "--help"},
          {"--unknown", "--version"},
      };
      for (const std::vector<std::string> &Arguments : InvalidArguments)
      {
        Application Command = makeApplication();
        std::ostringstream Output;
        std::ostringstream ErrorOutput;
        const ParseResult Result = Command.parseArguments(Arguments, Output, ErrorOutput);
        EXPECT_TRUE(Result.ShouldExit);
        EXPECT_EQ(Result.Code, ExitCode::InvocationError);
        EXPECT_TRUE(Output.str().empty());
      }
    }

    // Verifies that help cannot hide repeated scalar options or explicitly excluded option pairs.
    TEST(ApplicationTest, ValidatesSingleAndExcludedOptionsBeforeHelp)
    {
      Application ScalarCommand = makeApplication();
      std::string OutputPath;
      ScalarCommand.app().add_option("-o,--output", OutputPath);
      std::ostringstream ScalarOutput;
      std::ostringstream ScalarErrorOutput;
      const ParseResult ScalarResult = ScalarCommand.parseArguments({"--output", "first", "--output", "second", "--help"}, ScalarOutput, ScalarErrorOutput);
      EXPECT_TRUE(ScalarResult.ShouldExit);
      EXPECT_EQ(ScalarResult.Code, ExitCode::InvocationError);
      EXPECT_TRUE(ScalarOutput.str().empty());

      Application ConflictCommand = makeApplication();
      bool FirstEnabled = false;
      bool SecondEnabled = false;
      CLI::Option *FirstOption = ConflictCommand.app().add_flag("--first", FirstEnabled);
      CLI::Option *SecondOption = ConflictCommand.app().add_flag("--second", SecondEnabled);
      FirstOption->excludes(SecondOption);
      std::ostringstream ConflictOutput;
      std::ostringstream ConflictErrorOutput;
      const ParseResult ConflictResult = ConflictCommand.parseArguments({"--first", "--second", "--help"}, ConflictOutput, ConflictErrorOutput);
      EXPECT_TRUE(ConflictResult.ShouldExit);
      EXPECT_EQ(ConflictResult.Code, ExitCode::InvocationError);
      EXPECT_TRUE(ConflictOutput.str().empty());
    }

    // Verifies that dash-leading operands require the option terminator while a lone dash remains available for stdin.
    TEST(ApplicationTest, RequiresOptionTerminatorForDashLeadingOperands)
    {
      Application RejectedCommand = makeApplication();
      std::string RejectedInput;
      RejectedCommand.app().add_option("INPUT", RejectedInput);
      std::ostringstream RejectedOutput;
      std::ostringstream RejectedErrorOutput;
      const ParseResult RejectedResult = RejectedCommand.parseArguments({"-1"}, RejectedOutput, RejectedErrorOutput);
      EXPECT_TRUE(RejectedResult.ShouldExit);
      EXPECT_EQ(RejectedResult.Code, ExitCode::InvocationError);

      Application AcceptedCommand = makeApplication();
      std::string AcceptedInput;
      AcceptedCommand.app().add_option("INPUT", AcceptedInput);
      std::ostringstream AcceptedOutput;
      std::ostringstream AcceptedErrorOutput;
      const ParseResult AcceptedResult = AcceptedCommand.parseArguments({"--", "-1"}, AcceptedOutput, AcceptedErrorOutput);
      EXPECT_FALSE(AcceptedResult.ShouldExit);
      EXPECT_EQ(AcceptedInput, "-1");
    }

    // Verifies that registered options remain valid after an ordinary positional operand.
    TEST(ApplicationTest, AcceptsOptionsAfterOperands)
    {
      Application Command = makeApplication();
      std::string Input;
      bool FeatureEnabled = false;
      Command.app().add_option("INPUT", Input)->required();
      Command.app().add_flag("--feature-name", FeatureEnabled);
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"source.ink", "--feature-name"}, Output, ErrorOutput);

      EXPECT_FALSE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::Success);
      EXPECT_EQ(Input, "source.ink");
      EXPECT_TRUE(FeatureEnabled);
    }

    // Verifies that shared lookup and single-occurrence policies apply to options placed in help groups.
    TEST(ApplicationTest, AppliesPoliciesInsideOptionGroups)
    {
      Application AcceptedCommand = makeApplication();
      std::string OutputPath;
      bool FeatureEnabled = false;
      CLI::App *AcceptedGroup = AcceptedCommand.app().add_option_group("Output");
      AcceptedGroup->add_option("-o,--output", OutputPath);
      AcceptedGroup->add_flag("--feature-name", FeatureEnabled);
      std::ostringstream AcceptedOutput;
      std::ostringstream AcceptedErrorOutput;
      const ParseResult AcceptedResult = AcceptedCommand.parseArguments({"--output", "result", "--feature-name"}, AcceptedOutput, AcceptedErrorOutput);
      EXPECT_FALSE(AcceptedResult.ShouldExit);
      EXPECT_EQ(OutputPath, "result");
      EXPECT_TRUE(FeatureEnabled);

      Application RejectedCommand = makeApplication();
      bool RepeatedFeatureEnabled = false;
      CLI::App *RejectedGroup = RejectedCommand.app().add_option_group("Features");
      RejectedGroup->add_flag("--feature-name", RepeatedFeatureEnabled);
      std::ostringstream RejectedOutput;
      std::ostringstream RejectedErrorOutput;
      const ParseResult RejectedResult = RejectedCommand.parseArguments({"--feature-name", "--feature-name"}, RejectedOutput, RejectedErrorOutput);
      EXPECT_TRUE(RejectedResult.ShouldExit);
      EXPECT_EQ(RejectedResult.Code, ExitCode::InvocationError);
    }

    // Verifies that v0 rejects named subcommands instead of partially relying on CLI11 subcommand behavior.
    TEST(ApplicationTest, RejectsNamedSubcommands)
    {
      Application Command = makeApplication();
      Command.app().add_subcommand("build");
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      EXPECT_THROW(Command.parseArguments({"build"}, Output, ErrorOutput), std::logic_error);

      Application AliasCommand = makeApplication();
      AliasCommand.app().add_option_group("Output")->alias("build");
      std::ostringstream AliasOutput;
      std::ostringstream AliasErrorOutput;
      EXPECT_THROW(AliasCommand.parseArguments({"--help"}, AliasOutput, AliasErrorOutput), std::logic_error);
    }

    // Verifies that malformed UTF-8 argv is rejected before option parsing without echoing invalid bytes.
    TEST(ApplicationTest, RejectsInvalidUtf8Arguments)
    {
      const std::vector<std::string> InvalidValues = {
          std::string("\x80", 1),
          std::string("\xC0\x80", 2),
          std::string("\xED\xA0\x80", 3),
          std::string("\xF4\x90\x80\x80", 4),
          std::string("\xE2\x82", 2),
      };
      for (const std::string &InvalidValue : InvalidValues)
      {
        Application Command = makeApplication();
        std::string Input;
        Command.app().add_option("INPUT", Input);
        std::ostringstream Output;
        std::ostringstream ErrorOutput;
        const ParseResult Result = Command.parseArguments({InvalidValue}, Output, ErrorOutput);
        EXPECT_TRUE(Result.ShouldExit);
        EXPECT_EQ(Result.Code, ExitCode::InvocationError);
        EXPECT_TRUE(Output.str().empty());
        EXPECT_NE(ErrorOutput.str().find("argument 1: is not valid UTF-8"), std::string::npos);
      }
    }

    // Verifies that the option terminator preserves a dash-prefixed UTF-8 operand byte-for-byte.
    TEST(ApplicationTest, PreservesUtf8OperandsAfterOptionTerminator)
    {
      Application Command = makeApplication();
      std::string Input;
      Command.app().add_option("INPUT", Input)->required();
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const std::string Expected = u8"-源文件.ink";
      const ParseResult Result = Command.parseArguments({"--", Expected}, Output, ErrorOutput);

      EXPECT_FALSE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::Success);
      EXPECT_EQ(Input, Expected);
      EXPECT_TRUE(Output.str().empty());
      EXPECT_TRUE(ErrorOutput.str().empty());
    }

    // Verifies that the UTF-8 path bridge round-trips non-ASCII source file names.
    TEST(ApplicationIoTest, ConvertsUtf8PathsWithoutLosingText)
    {
      const std::string Expected = u8"源文件.ink";
      EXPECT_EQ(pathFromUtf8(Expected).u8string(), Expected);
    }
  } // namespace
} // namespace ink::cli
