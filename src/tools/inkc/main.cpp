#include "compilation_context.h"

#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/name/name_pool.h"
#include "ink/frontend/context.h"
#include "ink/frontend/parser/parser.h"
#include "ink/frontend/tokenizer/tokenizer.h"
#include "ink/ir/module/module.h"
#include "ink/ir/serialization/serialization.h"
#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"

#include <cxxopts.hpp>
#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/ostream_sink.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
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
  std::string IRFile;
  std::string HelpValue;
  std::string VersionValue;
  cxxopts::Options Options("inkc", "Compile an Ink source file into an IR module. Semantic lowering is still under development.");
  Options.custom_help("-i FILE [-S [FILE]] [options]");
  Options.allow_unrecognised_options();
  auto AddOption = Options.add_options();
  AddOption("i,input", "Input file path (required)", cxxopts::value<std::string>(InputFile), "FILE");
  AddOption("S", "Write text IR (default: input path with .ir extension)", cxxopts::value<std::string>(IRFile), "[FILE]");
  AddOption("h,help", "Print help", cxxopts::value<std::string>(HelpValue)->implicit_value(""));
  AddOption("V,version", "Print version", cxxopts::value<std::string>(VersionValue)->implicit_value(""));

  // cxxopts does not consume separate values for options with implicit values.
  // Supply an empty value for bare -S while preserving required -i values and argv bytes.
  std::vector<const char *> ParserArguments;
  for (int Index = 0; Index < ArgumentCount; ++Index)
  {
    const std::string_view Argument(ArgumentValues[Index]);
    ParserArguments.push_back(ArgumentValues[Index]);
    if (Index == 0)
    {
      continue;
    }
    if (Argument == "--")
    {
      ParserArguments.insert(ParserArguments.end(), ArgumentValues + Index + 1, ArgumentValues + ArgumentCount);
      break;
    }
    char ValueOption = 0;
    if (Argument == "--input")
    {
      ValueOption = 'i';
    }
    else if (Argument == "--S")
    {
      ValueOption = 'S';
    }
    else if (Argument.size() > 1 && Argument.front() == '-' && Argument[1] != '-')
    {
      const auto ValueIndex = Argument.find_first_of("iS", 1);
      if (ValueIndex != std::string_view::npos && ValueIndex + 1 == Argument.size())
      {
        ValueOption = Argument[ValueIndex];
      }
    }
    if (ValueOption == 'S' && (Index + 1 == ArgumentCount || ArgumentValues[Index + 1][0] == '-'))
    {
      ParserArguments.push_back("");
    }
    else if (ValueOption != 0 && Index + 1 < ArgumentCount)
    {
      ParserArguments.push_back(ArgumentValues[++Index]);
    }
  }
  // The sentinel routes missing required values through explicit validation.
  ParserArguments.push_back("");
  if (ParserArguments.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
  {
    return Error("too many arguments");
  }
  const cxxopts::ParseResult Parsed = Options.parse(static_cast<int>(ParserArguments.size()), ParserArguments.data());
  const auto &Unmatched = Parsed.unmatched();
  const std::size_t UnmatchedCount = Unmatched.size() - (!Unmatched.empty() && Unmatched.back().empty() ? 1 : 0);
  if (UnmatchedCount != 0)
  {
    return Error("unexpected argument '" + Unmatched.front() + "'");
  }
  if (Parsed.count("input") > 1 || Parsed.count("S") > 1 || Parsed.count("help") > 1 || Parsed.count("version") > 1)
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

  // The single-input driver uses a logical entry name until source module resolution is implemented.
  ink::semantic::SemanticContext Semantic(Compilation.sourceManager(), Compilation.diagnosticEngine(), Compilation.targetContext());
  const auto Module = Semantic.moduleManager().createModule({{ink::core::NamePool::instance().intern("main")}});
  if (!Module)
  {
    ErrorOutput.error("inkc: error: cannot create entry IR module\n");
    return 2;
  }
  const bool Succeeded = ink::semantic::Analyzer{}.analyze(Semantic, Result, **Module);
  const bool SemanticErrors = ReportDiagnostics();
  if (!Succeeded && !SemanticErrors)
  {
    ErrorOutput.error("inkc: error: semantic analysis failed for '{}'\n", InputFile);
  }
  if (OutputFailed || !std::cerr)
  {
    return 2;
  }
  if (!Succeeded || SemanticErrors)
  {
    return 1;
  }
  if (Parsed.count("S") != 0)
  {
    const auto TextIR = ink::ir::writeText(**Module);
    if (!TextIR)
    {
      ErrorOutput.error("inkc: error: cannot serialize text IR: {}\n", TextIR.error().Message);
      return 2;
    }
    const std::filesystem::path InputPath(InputFile);
    const std::filesystem::path IRPath = IRFile.empty() ? std::filesystem::path(InputPath).replace_extension(".ir") : std::filesystem::path(IRFile);
    std::error_code PathError;
    if (std::filesystem::equivalent(InputPath, IRPath, PathError))
    {
      ErrorOutput.error("inkc: error: IR output '{}' would overwrite the input file\n", IRPath.string());
      return 2;
    }
    if (PathError && PathError != std::errc::no_such_file_or_directory)
    {
      ErrorOutput.error("inkc: error: cannot inspect IR output '{}': {}\n", IRPath.string(), PathError.message());
      return 2;
    }
    std::ofstream IRStream(IRPath, std::ios::binary | std::ios::trunc);
    if (!IRStream)
    {
      ErrorOutput.error("inkc: error: cannot open IR output '{}'\n", IRPath.string());
      return 2;
    }
    IRStream.write(TextIR->data(), static_cast<std::streamsize>(TextIR->size()));
    IRStream.close();
    if (!IRStream)
    {
      ErrorOutput.error("inkc: error: cannot write IR output '{}'\n", IRPath.string());
      return 2;
    }
  }
  return 0;
}
