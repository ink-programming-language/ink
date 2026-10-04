#include "bytecode_commands.h"
#include "bytecode_command_support.h"

#include "ink/execution/artifact/bytecode_archive.h"
#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/engine/execution_linker.h"
#include "ink/execution/engine/execution_machine.h"
#include "ink/execution/support/execution_diagnostic.h"
#include "ink/ir/context.h"

#include <bit>
#include <cstdint>
#include <utility>

namespace ink::tools
{
  int runBytecode(const std::string &Input)
  {
    using namespace execution;
    std::filesystem::path Path;
    if (!cli::pathFromUtf8(Input, Path))
    {
      return bytecodeError("input path is not valid UTF-8");
    }
    BytecodeArtifactResult Read = readBytecodeFile(Path);
    if (!Read)
    {
      return bytecodeError(Read.Message);
    }
    BytecodeArtifact &Artifact = *Read.Artifact;
    if (Artifact.Kind != BytecodeArtifactKind::Executable || Artifact.Entry == InvalidFunction)
    {
      return bytecodeError("running bytecode requires a linked executable with an entry");
    }
    const auto Descriptor = Artifact.Image.Descriptors.find(Artifact.Entry);
    if (Descriptor == Artifact.Image.Descriptors.end())
    {
      return bytecodeError("invalid bytecode entry");
    }
    const TypeDesc &Signature = *Artifact.Image.Layouts->get(Descriptor->second.Signature);
    const TypeDesc &Return = *Artifact.Image.Layouts->get(Signature.functionDesc().ReturnType);
    if (!Signature.functionDesc().Parameters.empty() || (Return.Kind != RuntimeKind::Void && !(Return.Kind == RuntimeKind::Integer && Return.bitWidth() == 32 && Return.isSigned())))
    {
      return bytecodeError("bytecode entry must take no arguments and return void or i32");
    }
    const FunctionId Entry = Artifact.Entry;
    const RuntimeKind ReturnKind = Return.Kind;
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionEngine Engine(Context);
    ExecutionLinker Linker(std::move(Artifact.Image));
    ExecutionMachine Machine(Engine, Linker);
    const RuntimeValueResult Result = Machine.execute(Entry, {});
    if (!Result)
    {
      if (const auto Diagnostic = makeExecutionDiagnostic(Result.Status, {}, {}, "bytecode execution failed"))
      {
        Compilation.diagnosticEngine().report(*Diagnostic);
        const core::FormattedDiagnostic Formatted = core::DiagnosticFormatter{}.format(*Diagnostic);
        if (!cli::writeOutput(std::cerr, "inkc: error[" + std::string(Diagnostic->code()) + "]: " + Formatted.Message + "\n"))
        {
          return cli::exitStatus(cli::ExitCode::InvocationError);
        }
        return cli::exitStatus(cli::ExitCode::SourceError);
      }
      return bytecodeError("bytecode execution was cancelled", cli::ExitCode::SourceError);
    }
    return ReturnKind == RuntimeKind::Void ? 0 : static_cast<int>(std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(Result.Value.Bits)));
  }
} // namespace ink::tools
