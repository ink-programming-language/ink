#ifndef INKC_BYTECODE_COMMANDS_H
#define INKC_BYTECODE_COMMANDS_H

#include <string>
#include <vector>

namespace ink::ir
{
  class Module;
} // namespace ink::ir

namespace ink::semantic
{
  class SemanticContext;
} // namespace ink::semantic

namespace ink::tools
{
  struct BytecodeOptions
  {
      std::string Output;
      std::string PatchOutput;
      std::string PatchBase;
      std::vector<std::string> PatchFunctions;
  };

  int emitBytecode(semantic::SemanticContext &Context, const ir::Module &Module, const BytecodeOptions &Options);
  int linkBytecode(const std::vector<std::string> &Inputs, const std::string &Output, const std::string &Entry);
  int runBytecode(const std::string &Input);
} // namespace ink::tools

#endif
