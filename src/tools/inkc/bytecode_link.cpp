#include "bytecode_commands.h"
#include "bytecode_command_support.h"

#include "ink/execution/artifact/bytecode_archive.h"
#include "ink/execution/artifact/bytecode_linker.h"

#include <utility>

namespace ink::tools
{
  int linkBytecode(const std::vector<std::string> &Inputs, const std::string &Output, const std::string &Entry)
  {
    using namespace execution;
    std::vector<std::unique_ptr<BytecodeArtifact>> Owned;
    std::vector<const BytecodeArtifact *> Objects;
    const BytecodeSymbolIdentity *EntryIdentity = nullptr;
    for (const std::string &Input : Inputs)
    {
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
      for (const BytecodeSymbol &Symbol : Read.Artifact->Symbols)
      {
        if (Symbol.Kind != BytecodeSymbolKind::Definition || Symbol.Visibility != BytecodeVisibility::Public || (Symbol.Identity.Name != Entry && Symbol.Identity.Module + "::" + Symbol.Identity.Name != Entry && Symbol.Identity.Module + "#" + Symbol.Identity.Name != Entry))
        {
          continue;
        }
        const RuntimeFunctionDescriptor &Descriptor = Read.Artifact->Image.Descriptors.find(Symbol.Function)->second;
        const StorageLayout &Signature = *Read.Artifact->Image.Layouts->get(Descriptor.Signature);
        const StorageLayout &Return = *Read.Artifact->Image.Layouts->get(Signature.ReturnType);
        if (!Signature.Parameters.empty() || !Symbol.Identity.GenericArguments.empty() || (Return.Kind != RuntimeKind::Void && !(Return.Kind == RuntimeKind::Integer && Return.BitWidth == 32 && Return.Signed)))
        {
          continue;
        }
        if (EntryIdentity)
        {
          return bytecodeError("ambiguous public bytecode entry: " + Entry);
        }
        EntryIdentity = &Symbol.Identity;
      }
      Objects.push_back(Read.Artifact.get());
      Owned.push_back(std::move(Read.Artifact));
    }
    if (!EntryIdentity)
    {
      return bytecodeError("entry requires a public non-generic function taking no arguments and returning void or i32: " + Entry);
    }
    BytecodeArtifactResult Linked = linkBytecodeArtifacts(Objects, EntryIdentity);
    if (!Linked)
    {
      return bytecodeError(Linked.Message, cli::ExitCode::SourceError);
    }
    std::filesystem::path Path;
    if (!cli::pathFromUtf8(Output, Path))
    {
      return bytecodeError("output path is not valid UTF-8");
    }
    const BytecodeResult Written = writeBytecodeFile(Path, *Linked.Artifact);
    return Written ? 0 : bytecodeError(Written.Message);
  }
} // namespace ink::tools
