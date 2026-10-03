#ifndef INKC_BYTECODE_COMMAND_SUPPORT_H
#define INKC_BYTECODE_COMMAND_SUPPORT_H

#include "ink/cli/application.h"
#include "ink/cli/io.h"

#include <iostream>
#include <string>
#include <string_view>

namespace ink::tools
{
  inline int bytecodeError(std::string_view Message, cli::ExitCode Code = cli::ExitCode::InvocationError)
  {
    if (!cli::writeOutput(std::cerr, "inkc: error: " + std::string(Message) + "\n"))
    {
      return cli::exitStatus(cli::ExitCode::InvocationError);
    }
    return cli::exitStatus(Code);
  }
} // namespace ink::tools

#endif
