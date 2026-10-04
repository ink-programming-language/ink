#include "bytecode_commands.h"
#include "bytecode_command_support.h"

#include "ink/execution/artifact/bytecode_archive.h"
#include "ink/execution/artifact/bytecode_builder.h"
#include "ink/ir/function/function.h"
#include "ink/semantic/context.h"

#include <unordered_set>
#include <utility>

namespace ink::tools
{
  namespace
  {
    struct NamedFunction
    {
        const ir::Function *Function;
        bool Local;
    };

    void collectFunctions(const ir::BasicBlock &Block, bool Local, std::vector<NamedFunction> &Functions)
    {
      for (const auto &Value : Block.values())
      {
        if (ir::Function::classof(Value.get()))
        {
          const auto &Function = static_cast<const ir::Function &>(*Value);
          Functions.push_back({&Function, Local});
          for (const auto &Body : Function.blocks())
          {
            collectFunctions(*Body, true, Functions);
          }
        }
        else if (ir::Module::classof(Value.get()))
        {
          const auto &Module = static_cast<const ir::Module &>(*Value);
          collectFunctions(Module.entryBlock(), Local, Functions);
        }
      }
    }

    const ir::Module *owningModule(const ir::Function &Function)
    {
      for (const ir::Value *Outer = Function.outer(); Outer; Outer = Outer->outer())
      {
        if (ir::Module::classof(Outer))
        {
          return static_cast<const ir::Module *>(Outer);
        }
      }
      return nullptr;
    }
  } // namespace

  int emitBytecode(semantic::SemanticContext &Context, const ir::Module &Module, const BytecodeOptions &Options)
  {
    using namespace execution;
    SemanticValueBridge Bridge(Context.irContext());
    const std::string ModuleName(Context.namePool().text(Module.name()));
    std::vector<NamedFunction> Named;
    collectFunctions(Module.entryBlock(), false, Named);
    std::vector<BytecodeFunctionInput> Inputs;
    std::unordered_set<const ir::Function *> Seen;
    for (const NamedFunction &NamedValue : Named)
    {
      const ir::Function &Function = *NamedValue.Function;
      const auto Definition = Context.comptimeState().Functions.find(&Function);
      if (Definition != Context.comptimeState().Functions.end() && Definition->second.Comptime)
      {
        continue;
      }
      BytecodeFunctionInput Input;
      Input.Function = &Function;
      Input.Identity.Name = Context.namePool().text(Function.name());
      const bool Native = Function.isNativeImport();
      Input.Kind = Native ? BytecodeSymbolKind::Native : BytecodeSymbolKind::Definition;
      Input.Identity.Module = Native ? "C" : ModuleName;
      Input.Visibility = !NamedValue.Local && Function.visibility() == core::VisibilityKind::Public ? BytecodeVisibility::Public : BytecodeVisibility::Private;
      Inputs.push_back(std::move(Input));
      Seen.insert(&Function);
    }
    for (const ir::Function *Function : Context.moduleImports(Module))
    {
      if (!Seen.insert(Function).second)
      {
        continue;
      }
      const auto Definition = Context.comptimeState().Functions.find(Function);
      if (Definition != Context.comptimeState().Functions.end() && Definition->second.Comptime)
      {
        continue;
      }
      const ir::Module *Owner = owningModule(*Function);
      if (!Owner)
      {
        return bytecodeError("imported function has no owning source module");
      }
      const bool Native = Function->isNativeImport();
      BytecodeFunctionInput Input;
      Input.Function = Function;
      Input.Kind = Native ? BytecodeSymbolKind::Native : BytecodeSymbolKind::Import;
      Input.Identity.Module = Native ? "C" : std::string(Context.namePool().text(Owner->name()));
      Input.Identity.Name = Context.namePool().text(Function->name());
      Input.Visibility = Function->visibility() == core::VisibilityKind::Public ? BytecodeVisibility::Public : BytecodeVisibility::Private;
      Inputs.push_back(std::move(Input));
    }
    BytecodeArtifactResult Built = buildBytecodeObject(ModuleName, Bridge, Inputs);
    if (!Built)
    {
      return bytecodeError(Built.Message, cli::ExitCode::SourceError);
    }
    std::filesystem::path Path;
    if (!cli::pathFromUtf8(Options.Output, Path))
    {
      return bytecodeError("output path is not valid UTF-8");
    }
    const BytecodeResult Written = writeBytecodeFile(Path, *Built.Artifact);
    return Written ? 0 : bytecodeError(Written.Message);
  }
} // namespace ink::tools
