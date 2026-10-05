#include "bytecode_commands.h"
#include "bytecode_command_support.h"

#include "ink/execution/artifact/bytecode_archive.h"
#include "ink/execution/artifact/bytecode_builder.h"
#include "ink/execution/hybrid/archive.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/class_type.h"
#include "ink/semantic/context.h"

#include <algorithm>
#include <unordered_map>
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

    int emitPatch(semantic::SemanticContext &Context, const ir::Module &Module, const BytecodeOptions &Options)
    {
      using namespace execution;
      std::filesystem::path BasePath;
      std::filesystem::path OutputPath;
      if (!cli::pathFromUtf8(Options.PatchBase, BasePath) || !cli::pathFromUtf8(Options.PatchOutput, OutputPath))
      {
        return bytecodeError("patch path is not valid UTF-8");
      }
      auto Base = readHybridFile(BasePath);
      if (!Base || Base.Artifact.Kind != HybridArtifactKind::Base)
      {
        return bytecodeError(Base ? "--patch-base requires a native base manifest" : Base.Error);
      }
      std::vector<NamedFunction> Functions;
      for (const auto &Root : Context.irContext().modules())
      {
        collectFunctions(Root->entryBlock(), false, Functions);
      }
      std::unordered_set<std::string> Requested(Options.PatchFunctions.begin(), Options.PatchFunctions.end());
      std::unordered_set<std::string> Matched;
      std::vector<BytecodeFunctionInput> Inputs;
      for (const auto &Named : Functions)
      {
        const auto &Function = *Named.Function;
        const auto Definition = Context.comptimeState().Functions.find(&Function);
        if (Definition != Context.comptimeState().Functions.end() && Definition->second.Comptime)
        {
          continue;
        }
        const auto *Owner = owningModule(Function);
        if (!Owner || (!Function.hasBody() && !Function.isNativeImport()))
        {
          continue;
        }
        BytecodeFunctionInput Input;
        Input.Function = &Function;
        Input.Identity.Module = Function.isNativeImport() ? "C" : std::string(Context.namePool().text(Owner->name()));
        Input.Identity.Name = Context.namePool().text(Function.name());
        const std::string FunctionName = Function.classOwner() ? std::string(Context.namePool().text(Function.classOwner()->name())) + "." + Input.Identity.Name : Input.Identity.Name;
        const std::string Key = Input.Identity.Module + "#" + FunctionName;
        const bool Selected = Requested.contains(Key) && Function.hasBody();
        if (Selected)
        {
          Matched.insert(Key);
        }
        Input.Kind = Function.isNativeImport() ? BytecodeSymbolKind::Native : Selected ? BytecodeSymbolKind::Definition : BytecodeSymbolKind::Import;
        Input.Visibility = !Named.Local && Function.visibility() == core::VisibilityKind::Public ? BytecodeVisibility::Public : BytecodeVisibility::Private;
        Inputs.push_back(std::move(Input));
      }
      for (const auto &Name : Requested)
      {
        if (!Matched.contains(Name))
        {
          return bytecodeError("patch function was not found: " + Name, cli::ExitCode::SourceError);
        }
      }
      SemanticValueBridge Bridge(Context.irContext());
      auto Built = buildBytecodeObject(Context.namePool().text(Module.name()), Bridge, Inputs);
      if (!Built)
      {
        return bytecodeError(Built.Message, cli::ExitCode::SourceError);
      }
      const auto &Baseline = *Base.Artifact.Bytecode;
      if (const auto Check = compatibleHybridTypes(*Baseline.Image.Layouts, *Built.Artifact->Image.Layouts); !Check)
      {
        return bytecodeError(Check.Message, cli::ExitCode::SourceError);
      }
      std::unordered_map<std::string, const BytecodeSymbol *> Known;
      for (const auto &Symbol : Baseline.Symbols)
      {
        Known.emplace(Symbol.Identity.LinkName, &Symbol);
      }
      for (const auto &Symbol : Built.Artifact->Symbols)
      {
        if (Symbol.Kind == BytecodeSymbolKind::Native)
        {
          continue;
        }
        const auto Found = Known.find(Symbol.Identity.LinkName);
        if (Found != Known.end())
        {
          if (Found->second->Identity.Signature != Symbol.Identity.Signature)
          {
            return bytecodeError("patch signature differs from the native build: " + Symbol.Identity.Name, cli::ExitCode::SourceError);
          }
          continue;
        }
        const bool ExistingName = std::any_of(Baseline.Symbols.begin(), Baseline.Symbols.end(), [&](const BytecodeSymbol &Existing)
        {
          return Existing.Identity.Module == Symbol.Identity.Module && Existing.Identity.Name == Symbol.Identity.Name;
        });
        if (ExistingName || Symbol.Kind != BytecodeSymbolKind::Definition || Symbol.Visibility != BytecodeVisibility::Private)
        {
          return bytecodeError("patch has an unknown or changed function: " + Symbol.Identity.Module + "#" + Symbol.Identity.Name, cli::ExitCode::SourceError);
        }
      }
      auto Bytes = serializeHybridArtifact(HybridArtifactKind::Patch, Base.Artifact.BuildId, *Built.Artifact);
      if (!Bytes)
      {
        return bytecodeError(Bytes.Message);
      }
      const auto Written = writeHybridFile(OutputPath, Bytes.Bytes);
      return Written ? 0 : bytecodeError(Written.Message);
    }
  } // namespace

  int emitBytecode(semantic::SemanticContext &Context, const ir::Module &Module, const BytecodeOptions &Options)
  {
    using namespace execution;
    if (!Options.PatchOutput.empty())
    {
      return emitPatch(Context, Module, Options);
    }
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
