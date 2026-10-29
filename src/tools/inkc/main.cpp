#include "compilation_context.h"

#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/frontend/context.h"
#include "ink/frontend/parser/parser.h"
#include "ink/frontend/tokenizer/tokenizer.h"

#include <cxxopts.hpp>
#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/ostream_sink.h>

#include <array>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
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
  cxxopts::Options Options("inkc", "Check an Ink source file for lexical and syntax errors.");
  Options.custom_help("-i FILE [options]");
  Options.allow_unrecognised_options();
  auto AddOption = Options.add_options();
  AddOption("i,input", "Input file path (required)", cxxopts::value<std::string>(InputFile), "FILE");
  AddOption("h,help", "Print help", cxxopts::value<std::string>(HelpValue)->implicit_value(""));
  AddOption("V,version", "Print version", cxxopts::value<std::string>(VersionValue)->implicit_value(""));

  // The sentinel routes missing required values through explicit validation.
  std::vector<const char *> ParserArguments(ArgumentValues, ArgumentValues + ArgumentCount);
  ParserArguments.push_back("");
  const cxxopts::ParseResult Parsed = Options.parse(static_cast<int>(ParserArguments.size()), ParserArguments.data());
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

  std::string Source;
  {
    std::ifstream Input(InputFile, std::ios::binary);
    if (!Input)
    {
      ErrorOutput.error("inkc: error: cannot open '{}'\n", InputFile);
      return 2;
    }
    std::array<char, 64 * 1024> Buffer;
    while (Input)
    {
      Input.read(Buffer.data(), static_cast<std::streamsize>(Buffer.size()));
      Source.append(Buffer.data(), static_cast<std::size_t>(Input.gcount()));
    }
    if (!Input.eof() || Input.bad())
    {
      ErrorOutput.error("inkc: error: cannot read '{}'\n", InputFile);
      return 2;
    }
  }

  ink::inkc::CompilationContext Compilation;
  ink::core::CollectingDiagnosticConsumer Diagnostics;
  Compilation.diagnosticEngine().addConsumer(Diagnostics);
  const auto ReportDiagnostics = [&Diagnostics, &ErrorOutput, &Compilation]()
  {
    bool HasErrors = false;
    for (const ink::core::Diagnostic &Diagnostic : Diagnostics.takeDiagnostics())
    {
      HasErrors = HasErrors || ink::core::diagnosticDefaultSeverity(Diagnostic.Kind) == ink::core::DiagnosticSeverity::Error;
      ErrorOutput.error("{}", ink::core::DiagnosticFormatter{}.format(Diagnostic, Compilation.sourceManager()));
    }
    return HasErrors;
  };
  ink::frontend::FrontendContext Frontend(Compilation.sourceManager(), Compilation.diagnosticEngine());
  const ink::core::SourceId SourceId = Compilation.sourceManager().addSource(InputFile, std::move(Source));
  auto Tokens = ink::frontend::tokenizer::tokenizeSource(Frontend, SourceId);
  const ink::frontend::parser::ParseResult Result = ink::frontend::parser::parse(Frontend, std::move(Tokens));
  const bool FrontendErrors = ReportDiagnostics();
  if (!Result.succeeded() || FrontendErrors)
  {
    return OutputFailed || !std::cerr ? 2 : 1;
  }

  return OutputFailed || !std::cerr ? 2 : 0;
}
