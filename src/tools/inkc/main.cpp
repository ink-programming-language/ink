#include "ink/cli/application.h"

#include <string>

namespace
{
  int runCompiler(int ArgumentCount, char **ArgumentValues)
  {
    ink::cli::Application Command({"inkc", "Compile Ink source files.", "development"});
    std::string InputFile;
    std::string IrOutputFile;
    Command.addOption("-i", InputFile, "Input Ink source file").required().typeName("FILE");
    Command.addOption("-oir", IrOutputFile, "Output IR file").required().typeName("FILE");
    const ink::cli::ParseResult ParsedArguments = Command.parse(ArgumentCount, ArgumentValues);
    if (ParsedArguments.ShouldExit)
    {
      return ink::cli::exitStatus(ParsedArguments.Code);
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
