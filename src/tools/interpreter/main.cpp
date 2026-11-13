#include "ink/core/config/config_manager.h"

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/ostream_sink.h>

#include <iostream>
#include <memory>
#include <string>

namespace cl = llvm::cl;

int main(int ArgumentCount, char **ArgumentValues)
{
  // Initialize LLVM's common options before replacing them with this tool's options.
  cl::getRegisteredOptions();
  cl::ResetCommandLineParser();
  static cl::opt<std::string> InputFile(cl::desc("<input bitcode>"), cl::Positional, cl::init("-"));
  // String flags keep validation errors out of LLVM's direct stderr output paths.
  static cl::opt<std::string> Help("h", cl::desc("Print help"), cl::ValueOptional);
  static cl::opt<std::string> Version("v", cl::desc("Print version"), cl::ValueOptional);
  static cl::alias HelpAlias("help", cl::aliasopt(Help));
  static cl::alias VersionAlias("version", cl::aliasopt(Version));

  spdlog::logger Output("ink-interpreter-output", std::make_shared<spdlog::sinks::ostream_sink_st>(std::cout, true));
  spdlog::logger ErrorOutput("ink-interpreter-error", std::make_shared<spdlog::sinks::ostream_sink_st>(std::cerr, true));
  Output.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
  ErrorOutput.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
  std::string ParseErrors;
  llvm::raw_string_ostream Errors(ParseErrors);
  if (!cl::ParseCommandLineOptions(ArgumentCount, ArgumentValues, "Ink VM interpreter.\n", &Errors))
  {
    ErrorOutput.error("{}", ParseErrors);
    return 2;
  }
  const int HelpCount = Help.getNumOccurrences() + HelpAlias.getNumOccurrences();
  const int VersionCount = Version.getNumOccurrences() + VersionAlias.getNumOccurrences();

  if (HelpCount != 0)
  {
    Output.info("Ink VM interpreter.\nUsage:\n  ink-interpreter [options] [<input bitcode>]\n\n  <input bitcode>  Input file (default: -, standard input)\n  -h, --help       Print help\n  -v, --version    Print version\n");
    return std::cout ? 0 : 2;
  }
  if (VersionCount != 0)
  {
    Output.info("ink-interpreter {}\n", ink::core::ConfigManager::getVersion<ink::core::ConfigKind::InkVersion>().toString());
    return std::cout ? 0 : 2;
  }

  return 0;
}
