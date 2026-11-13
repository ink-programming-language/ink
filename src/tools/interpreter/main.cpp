#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/core/config/config_manager.h"

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/ostream_sink.h>

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace cl = llvm::cl;

namespace
{
  static cl::opt<std::string> InputFile(cl::desc("<input bitcode>"), cl::Positional, cl::init("-"));
  // String flags keep validation errors out of LLVM's direct stderr output paths.
  static cl::opt<std::string> Help("h", cl::desc("Print help"), cl::ValueOptional);
  static cl::opt<std::string> Version("v", cl::desc("Print version"), cl::ValueOptional);
  // Defer aliases until LLVM's built-in options have been removed.
  static cl::alias HelpAlias("help", cl::aliasopt(Help), cl::DefaultOption);
  static cl::alias VersionAlias("version", cl::aliasopt(Version), cl::DefaultOption);

  static spdlog::logger Output("ink-interpreter-output", std::make_shared<spdlog::sinks::ostream_sink_st>(std::cout, true));
  static spdlog::logger ErrorOutput("ink-interpreter-error", std::make_shared<spdlog::sinks::ostream_sink_st>(std::cerr, true));

  // An exit code handles help, version, or errors; no value continues to input loading.
  std::optional<int> parseArgument(int ArgumentCount, char **ArgumentValues)
  {
    Output.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
    ErrorOutput.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));

    // Replace LLVM's common options with the file-scope interpreter options.
    cl::getRegisteredOptions();
    cl::ResetCommandLineParser();
    InputFile.addArgument();
    Help.addArgument();
    Version.addArgument();
    HelpAlias.addArgument();
    VersionAlias.addArgument();

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

    return std::nullopt;
  }

  std::optional<ink::vm::ExecutableFile> prepareInputBitCode()
  {
    const auto Input = llvm::MemoryBuffer::getFileOrSTDIN(InputFile.getValue(), false, false);
    if (!Input)
    {
      ErrorOutput.error("ink-interpreter: error: cannot read '{}': {}\n", InputFile.getValue(), Input.getError().message());
      return std::nullopt;
    }
    // Until an executable file decoder exists, input contains only the raw image bytes.
    ink::vm::ExecutableFile Executable;
    const auto Image = (*Input)->getBuffer();
    Executable.getImage().assign(Image.bytes_begin(), Image.bytes_end());
    return Executable;
  }
} // namespace

int main(int ArgumentCount, char **ArgumentValues)
{
  llvm::InitLLVM LLVM(ArgumentCount, ArgumentValues);
  if (const auto ExitCode = parseArgument(ArgumentCount, ArgumentValues))
  {
    return *ExitCode;
  }

  auto Executable = prepareInputBitCode();
  if (!Executable)
  {
    return 2;
  }

  ink::vm::Interpreter Interpreter;
  if (Interpreter.loadLibrary(std::move(*Executable)) == ink::vm::Interpreter::InvalidLibraryHandle)
  {
    ErrorOutput.error("ink-interpreter: error: cannot load '{}'\n", InputFile.getValue());
    return 2;
  }

  return 0;
}
