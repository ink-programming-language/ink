#include <cxxopts.hpp>
#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/ostream_sink.h>

#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

int main(int ArgumentCount, char **ArgumentValues)
{
  spdlog::logger Output("inkc-output", std::make_shared<spdlog::sinks::ostream_sink_st>(std::cout, true));
  spdlog::logger ErrorOutput("inkc-error", std::make_shared<spdlog::sinks::ostream_sink_st>(std::cerr, true));
  Output.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
  ErrorOutput.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
  bool OutputFailed = false;
  const auto OutputError = [&OutputFailed](const std::string &)
  {
    OutputFailed = true;
  };
  Output.set_error_handler(OutputError);
  ErrorOutput.set_error_handler(OutputError);
  const auto Error = [&ErrorOutput](std::string_view Message)
  {
    ErrorOutput.info("inkc: error: {}\nTry 'inkc --help' for more information.\n", Message);
    return 2;
  };
  if (ArgumentCount == std::numeric_limits<int>::max())
  {
    return Error("too many arguments");
  }

  std::string InputFile;
  std::string HelpValue;
  std::string VersionValue;
  cxxopts::Options Options("inkc", "Ink compiler command line. Currently validates arguments only; compilation is not implemented.");
  Options.custom_help("-i FILE [options]");
  Options.allow_unrecognised_options();
  auto AddOption = Options.add_options();
  AddOption("i,input", "Input file path (required)", cxxopts::value<std::string>(InputFile), "FILE");
  AddOption("h,help", "Print help", cxxopts::value<std::string>(HelpValue)->implicit_value(""));
  AddOption("V,version", "Print version", cxxopts::value<std::string>(VersionValue)->implicit_value(""));

  // Keep argv bytes unchanged. A trailing empty argument lets missing input values
  // reach explicit validation instead of cxxopts' fatal no-exceptions error path.
  std::vector<const char *> ParserArguments(ArgumentValues, ArgumentValues + ArgumentCount);
  ParserArguments.push_back("");
  const cxxopts::ParseResult Parsed = Options.parse(ArgumentCount + 1, ParserArguments.data());
  const auto &Unmatched = Parsed.unmatched();
  const std::size_t UnmatchedCount = Unmatched.size() - (!Unmatched.empty() && Unmatched.back().empty() ? 1 : 0);
  if (UnmatchedCount != 0)
  {
    return Error("unexpected argument '" + Unmatched.front() + "'");
  }
  if (Parsed.count("input") > 1 || Parsed.count("help") > 1 || Parsed.count("version") > 1)
  {
    return Error("options must not be repeated");
  }
  if (!HelpValue.empty() || !VersionValue.empty())
  {
    return Error("--help and --version do not accept values");
  }
  if (Parsed.count("input") != 0 && InputFile.empty())
  {
    return Error("-i requires a non-empty input file path");
  }
  if (Parsed.count("help") != 0)
  {
    Output.info("{}", Options.help());
    return OutputFailed || !std::cout ? 2 : 0;
  }
  if (Parsed.count("version") != 0)
  {
    Output.info("inkc development\n");
    return OutputFailed || !std::cout ? 2 : 0;
  }
  if (Parsed.count("input") == 0)
  {
    return Error("required option -i FILE is missing");
  }

  // The compilation pipeline will consume InputFile when it is connected.
  return 0;
}
