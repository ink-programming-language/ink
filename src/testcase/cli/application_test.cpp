#include "ink/cli/application.h"
#include "ink/cli/io.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <functional>
#include <sstream>
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

    // Verifies that the shared process boundary forwards the body result without hidden control flow.
    TEST(ApplicationTest, ForwardsProcessBodyResult)
    {
      std::ostringstream ErrorOutput;
      EXPECT_EQ(runMain("ink-test", []()
      {
        return 7;
      }, ErrorOutput), 7);
      EXPECT_TRUE(ErrorOutput.str().empty());
    }

    // Verifies that an absent process body is reported explicitly instead of invoking std::function's exceptional empty state.
    TEST(ApplicationTest, RejectsAnEmptyProcessBody)
    {
      std::ostringstream ErrorOutput;
      const std::function<int()> EmptyBody;

      EXPECT_EQ(runMain("ink-test", EmptyBody, ErrorOutput), exitStatus(ExitCode::InternalError));
      EXPECT_EQ(ErrorOutput.str(), "ink-test: internal error: process body is empty\n");
    }

    // Verifies that help is successful, bypasses required operands, and documents the public invocation spelling.
    TEST(ApplicationTest, WritesHelpToStandardOutput)
    {
      Application Command = makeApplication();
      std::string Input;
      std::string OutputPath;
      Command.addOption("INPUT", Input, "Input file").required().typeName("FILE");
      Command.addOption("-o,--output", OutputPath, "Output file").typeName("FILE");
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"--help"}, Output, ErrorOutput);

      EXPECT_TRUE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::Success);
      EXPECT_NE(Output.str().find("Exercise the shared Ink command-line policy."), std::string::npos);
      EXPECT_NE(Output.str().find("Usage: ink-test [OPTIONS] INPUT"), std::string::npos);
      EXPECT_NE(Output.str().find("-o, --output FILE"), std::string::npos);
      EXPECT_TRUE(ErrorOutput.str().empty());
    }

    // Verifies that a failed help-output stream changes the informational result into an invocation error.
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

    // Verifies that version uses both reserved spellings and writes only to standard output.
    TEST(ApplicationTest, WritesVersionToStandardOutput)
    {
      for (const std::string &Spelling : {std::string("-V"), std::string("--version")})
      {
        Application Command = makeApplication();
        std::ostringstream Output;
        std::ostringstream ErrorOutput;
        const ParseResult Result = Command.parseArguments({Spelling}, Output, ErrorOutput);
        EXPECT_TRUE(Result.ShouldExit);
        EXPECT_EQ(Result.Code, ExitCode::Success);
        EXPECT_EQ(Output.str(), "ink-test development\n");
        EXPECT_TRUE(ErrorOutput.str().empty());
      }
    }

    // Verifies that argc zero with a null argv represents an empty invocation.
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

    // Verifies that unknown options are rejected before a later informational flag can hide them.
    TEST(ApplicationTest, RejectsUnknownOptionsBeforeHelp)
    {
      Application Command = makeApplication();
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"--unknown", "--help"}, Output, ErrorOutput);

      EXPECT_TRUE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::InvocationError);
      EXPECT_TRUE(Output.str().empty());
      EXPECT_NE(ErrorOutput.str().find("ink-test: error: unrecognized option '--unknown'"), std::string::npos);
      EXPECT_NE(ErrorOutput.str().find("Try 'ink-test --help'"), std::string::npos);
    }

    // Verifies that long names remain case-sensitive and reject underscore and prefix aliases.
    TEST(ApplicationTest, RejectsUnregisteredOptionSpellings)
    {
      const std::vector<std::string> InvalidSpellings = {"--Feature-name", "--feature_name", "--feature"};
      for (const std::string &Spelling : InvalidSpellings)
      {
        Application Command = makeApplication();
        bool FeatureEnabled = false;
        Command.addFlag("--feature-name", FeatureEnabled);
        std::ostringstream Output;
        std::ostringstream ErrorOutput;
        const ParseResult Result = Command.parseArguments({Spelling}, Output, ErrorOutput);
        EXPECT_TRUE(Result.ShouldExit) << Spelling;
        EXPECT_EQ(Result.Code, ExitCode::InvocationError) << Spelling;
        EXPECT_FALSE(FeatureEnabled) << Spelling;
      }
    }

    // Verifies separated, equals-attached, and short-attached values together with clustered flags.
    TEST(ApplicationTest, ParsesNamedOptionsAndFlags)
    {
      Application Command = makeApplication();
      std::string OutputPath;
      std::vector<std::string> IncludePaths;
      bool Verbose = false;
      bool Force = false;
      Command.addOption("-o,--output", OutputPath).required().typeName("FILE");
      Command.addOption("-I,--include", IncludePaths).typeName("DIRECTORY");
      Command.addFlag("-v,--verbose", Verbose);
      Command.addFlag("-f,--force", Force);
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"-Ifirst", "--include=second", "-vf", "--output", "result.ir"}, Output, ErrorOutput);

      EXPECT_FALSE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::Success);
      EXPECT_EQ(OutputPath, "result.ir");
      EXPECT_EQ(IncludePaths, (std::vector<std::string>{"first", "second"}));
      EXPECT_TRUE(Verbose);
      EXPECT_TRUE(Force);
      EXPECT_TRUE(Output.str().empty());
      EXPECT_TRUE(ErrorOutput.str().empty());
    }

    // Verifies that LastWins keeps the final scalar value while Single rejects duplicates atomically.
    TEST(ApplicationTest, AppliesScalarRepeatPolicies)
    {
      Application LastWinsCommand = makeApplication();
      std::string OptimizationLevel;
      LastWinsCommand.addOption("--optimization-level", OptimizationLevel).repeatPolicy(RepeatPolicy::LastWins);
      std::ostringstream LastWinsOutput;
      std::ostringstream LastWinsErrorOutput;
      const ParseResult LastWinsResult = LastWinsCommand.parseArguments({"--optimization-level=0", "--optimization-level", "3"}, LastWinsOutput, LastWinsErrorOutput);
      EXPECT_FALSE(LastWinsResult.ShouldExit);
      EXPECT_EQ(OptimizationLevel, "3");

      Application SingleCommand = makeApplication();
      std::string OutputPath = "unchanged";
      SingleCommand.addOption("-o,--output", OutputPath);
      std::ostringstream SingleOutput;
      std::ostringstream SingleErrorOutput;
      const ParseResult SingleResult = SingleCommand.parseArguments({"--output", "first", "-o", "second"}, SingleOutput, SingleErrorOutput);
      EXPECT_TRUE(SingleResult.ShouldExit);
      EXPECT_EQ(SingleResult.Code, ExitCode::InvocationError);
      EXPECT_EQ(OutputPath, "unchanged");
    }

    // Verifies that string-array options append one physical argv value per occurrence in encounter order.
    TEST(ApplicationTest, AppendsStringArrayValues)
    {
      Application Command = makeApplication();
      std::vector<std::string> IncludePaths = {"default"};
      Command.addOption("-I,--include", IncludePaths);
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"-Ifirst", "--include", "second"}, Output, ErrorOutput);

      EXPECT_FALSE(Result.ShouldExit);
      EXPECT_EQ(IncludePaths, (std::vector<std::string>{"default", "first", "second"}));
    }

    // Verifies attached dash-leading values and the separated standard-stream marker for value options.
    TEST(ApplicationTest, AcceptsExplicitDashLeadingOptionValues)
    {
      Application Command = makeApplication();
      std::string LongValue;
      std::string ShortValue;
      std::string StreamValue;
      Command.addOption("--long-value", LongValue);
      Command.addOption("-s", ShortValue);
      Command.addOption("--stream", StreamValue);
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"--long-value=--flag", "-s-2", "--stream", "-"}, Output, ErrorOutput);

      EXPECT_FALSE(Result.ShouldExit);
      EXPECT_EQ(LongValue, "--flag");
      EXPECT_EQ(ShortValue, "-2");
      EXPECT_EQ(StreamValue, "-");
    }

    // Verifies empty and ambiguous separated values are rejected before any bound storage changes.
    TEST(ApplicationTest, RejectsInvalidSeparatedValuesAtomically)
    {
      const std::vector<std::vector<std::string>> InvalidArguments = {
          {"--output="},
          {"--output", ""},
          {"--output", "--help"},
          {"--output", "--"},
      };
      for (const std::vector<std::string> &Arguments : InvalidArguments)
      {
        Application Command = makeApplication();
        std::string OutputPath = "unchanged";
        Command.addOption("-o,--output", OutputPath);
        std::ostringstream Output;
        std::ostringstream ErrorOutput;
        const ParseResult Result = Command.parseArguments(Arguments, Output, ErrorOutput);
        EXPECT_TRUE(Result.ShouldExit);
        EXPECT_EQ(Result.Code, ExitCode::InvocationError);
        EXPECT_EQ(OutputPath, "unchanged");
        EXPECT_TRUE(Output.str().empty());
      }
    }

    // Verifies flags reject explicit values, duplicate occurrences, and conflicts with version.
    TEST(ApplicationTest, RejectsInvalidInformationalAndFlagCombinations)
    {
      const std::vector<std::vector<std::string>> InvalidArguments = {
          {"--feature=true"},
          {"--feature", "--feature"},
          {"-h", "--help"},
          {"--help", "--version"},
      };
      for (const std::vector<std::string> &Arguments : InvalidArguments)
      {
        Application Command = makeApplication();
        bool Feature = false;
        Command.addFlag("--feature", Feature);
        std::ostringstream Output;
        std::ostringstream ErrorOutput;
        const ParseResult Result = Command.parseArguments(Arguments, Output, ErrorOutput);
        EXPECT_TRUE(Result.ShouldExit);
        EXPECT_EQ(Result.Code, ExitCode::InvocationError);
        EXPECT_FALSE(Feature);
        EXPECT_TRUE(Output.str().empty());
      }
    }

    // Verifies required named and positional values report invocation errors when absent.
    TEST(ApplicationTest, EnforcesRequiredOptionsAndOperands)
    {
      Application Command = makeApplication();
      std::string OutputPath;
      std::string Input;
      Command.addOption("-o", OutputPath).required();
      Command.addOption("INPUT", Input).required();
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"-o", "result"}, Output, ErrorOutput);

      EXPECT_TRUE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::InvocationError);
      EXPECT_NE(ErrorOutput.str().find("required argument 'INPUT' is missing"), std::string::npos);
      EXPECT_TRUE(OutputPath.empty());
    }

    // Verifies the option terminator admits dash-leading operands and options may follow ordinary operands.
    TEST(ApplicationTest, ParsesPositionalsAndOptionTerminator)
    {
      Application TerminatedCommand = makeApplication();
      std::string DashInput;
      TerminatedCommand.addOption("INPUT", DashInput).required();
      std::ostringstream TerminatedOutput;
      std::ostringstream TerminatedErrorOutput;
      const ParseResult TerminatedResult = TerminatedCommand.parseArguments({"--", "-generated.ink"}, TerminatedOutput, TerminatedErrorOutput);
      EXPECT_FALSE(TerminatedResult.ShouldExit);
      EXPECT_EQ(DashInput, "-generated.ink");

      Application IntermixedCommand = makeApplication();
      std::string Input;
      bool Verbose = false;
      IntermixedCommand.addOption("INPUT", Input).required();
      IntermixedCommand.addFlag("--verbose", Verbose);
      std::ostringstream IntermixedOutput;
      std::ostringstream IntermixedErrorOutput;
      const ParseResult IntermixedResult = IntermixedCommand.parseArguments({"source.ink", "--verbose"}, IntermixedOutput, IntermixedErrorOutput);
      EXPECT_FALSE(IntermixedResult.ShouldExit);
      EXPECT_EQ(Input, "source.ink");
      EXPECT_TRUE(Verbose);
    }

    // Verifies a string-array positional captures all remaining operands in order.
    TEST(ApplicationTest, CollectsStringArrayPositionals)
    {
      Application Command = makeApplication();
      std::vector<std::string> Inputs;
      Command.addOption("INPUTS", Inputs);
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"first.ink", "second.ink"}, Output, ErrorOutput);

      EXPECT_FALSE(Result.ShouldExit);
      EXPECT_EQ(Inputs, (std::vector<std::string>{"first.ink", "second.ink"}));
    }

    // Verifies explicitly excluded options fail before either bound flag is changed.
    TEST(ApplicationTest, EnforcesOptionExclusions)
    {
      Application Command = makeApplication();
      bool First = false;
      bool Second = false;
      Option &FirstOption = Command.addFlag("--first", First);
      Option &SecondOption = Command.addFlag("--second", Second);
      FirstOption.excludes(SecondOption);
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = Command.parseArguments({"--first", "--second"}, Output, ErrorOutput);

      EXPECT_TRUE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::InvocationError);
      EXPECT_FALSE(First);
      EXPECT_FALSE(Second);
    }

    // Verifies malformed names and incompatible repeat policies return explicit internal definition errors.
    TEST(ApplicationTest, ReportsInvalidDefinitionsWithoutNonlocalControlFlow)
    {
      Application NameCommand = makeApplication();
      bool Invalid = false;
      NameCommand.addFlag("--Invalid_name", Invalid);
      std::ostringstream NameOutput;
      std::ostringstream NameErrorOutput;
      const ParseResult NameResult = NameCommand.parseArguments({"--help"}, NameOutput, NameErrorOutput);
      EXPECT_TRUE(NameResult.ShouldExit);
      EXPECT_EQ(NameResult.Code, ExitCode::InternalError);
      EXPECT_TRUE(NameOutput.str().empty());
      EXPECT_NE(NameErrorOutput.str().find("invalid command-line definition"), std::string::npos);

      Application PolicyCommand = makeApplication();
      std::string Value;
      PolicyCommand.addOption("--value", Value).repeatPolicy(RepeatPolicy::Append);
      std::ostringstream PolicyOutput;
      std::ostringstream PolicyErrorOutput;
      const ParseResult PolicyResult = PolicyCommand.parseArguments({"--help"}, PolicyOutput, PolicyErrorOutput);
      EXPECT_TRUE(PolicyResult.ShouldExit);
      EXPECT_EQ(PolicyResult.Code, ExitCode::InternalError);
    }

    // Verifies exclusions cannot reference an option owned by another application.
    TEST(ApplicationTest, RejectsCrossApplicationExclusions)
    {
      Application FirstCommand = makeApplication();
      Application SecondCommand = makeApplication();
      bool First = false;
      bool Second = false;
      Option &FirstOption = FirstCommand.addFlag("--first", First);
      Option &SecondOption = SecondCommand.addFlag("--second", Second);
      FirstOption.excludes(SecondOption);
      std::ostringstream Output;
      std::ostringstream ErrorOutput;
      const ParseResult Result = FirstCommand.parseArguments({"--help"}, Output, ErrorOutput);

      EXPECT_TRUE(Result.ShouldExit);
      EXPECT_EQ(Result.Code, ExitCode::InternalError);
      EXPECT_TRUE(Output.str().empty());
    }

    // Verifies malformed UTF-8 is rejected without copying invalid bytes into the diagnostic.
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
        Command.addOption("INPUT", Input);
        std::ostringstream Output;
        std::ostringstream ErrorOutput;
        const ParseResult Result = Command.parseArguments({InvalidValue}, Output, ErrorOutput);
        EXPECT_TRUE(Result.ShouldExit);
        EXPECT_EQ(Result.Code, ExitCode::InvocationError);
        EXPECT_TRUE(Output.str().empty());
        EXPECT_NE(ErrorOutput.str().find("argument 1: is not valid UTF-8"), std::string::npos);
      }
    }

    // Verifies the option terminator preserves a dash-prefixed UTF-8 operand byte-for-byte.
    TEST(ApplicationTest, PreservesUtf8OperandsAfterOptionTerminator)
    {
      Application Command = makeApplication();
      std::string Input;
      Command.addOption("INPUT", Input).required();
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
      std::filesystem::path Converted;

      ASSERT_TRUE(pathFromUtf8(Expected, Converted));
      EXPECT_EQ(Converted.u8string(), Expected);
    }

    // Verifies invalid UTF-8 paths are rejected before invoking the filesystem conversion API.
    TEST(ApplicationIoTest, RejectsInvalidUtf8Paths)
    {
      std::filesystem::path Converted = "unchanged";

      EXPECT_FALSE(pathFromUtf8(std::string("\xC0\xAF", 2), Converted));
      EXPECT_EQ(Converted, std::filesystem::path("unchanged"));
    }
  } // namespace
} // namespace ink::cli
