#include "ink/cli/application.h"
#include "ink/cli/io.h"

#include <CLI/CLI.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
  int runCompiler(int ArgumentCount, char **ArgumentValues)
  {
    CLI::App Command("Compile Ink source files.", "inkc");
    Command.allow_non_standard_option_names();
    Command.set_version_flag("-V,--version", "inkc development", "Print version information and exit");

    std::string InputFile;
    std::string IrOutputFile;
    Command.add_option("-i", InputFile, "Input Ink source file")->required()->type_name("FILE");
    Command.add_option("-oir", IrOutputFile, "Output IR file")->required()->type_name("FILE");

    try
    {
      ArgumentValues = Command.ensure_utf8(ArgumentValues);
      Command.parse(ArgumentCount, ArgumentValues);
    }
    catch (const CLI::ParseError &Error)
    {
      std::ostringstream BufferedOutput;
      std::ostringstream BufferedErrorOutput;
      const int ParserExitCode = Command.exit(Error, BufferedOutput, BufferedErrorOutput);
      const bool OutputSucceeded = ink::cli::writeOutput(std::cout, BufferedOutput.str());
      const bool ErrorOutputSucceeded = ink::cli::writeOutput(std::cerr, BufferedErrorOutput.str());
      if ((ParserExitCode == 0 && !OutputSucceeded) || !ErrorOutputSucceeded)
      {
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
      return ink::cli::exitStatus(ParserExitCode == 0 ? ink::cli::ExitCode::Success : ink::cli::ExitCode::InvocationError);
    }
    catch (const std::runtime_error &)
    {
      ink::cli::writeOutput(std::cerr, "inkc: error: cannot decode process arguments as UTF-8\nTry 'inkc --help' for more information.\n");
      return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
    }

    return ink::cli::exitStatus(ink::cli::ExitCode::Success);
  }
} // namespace

int main(int ArgumentCount, char **ArgumentValues)
{
  return ink::cli::runMain("inkc", [ArgumentCount, ArgumentValues]()
  {
    return runCompiler(ArgumentCount, ArgumentValues);
  });
}
