#include "ink/execution/hybrid/runtime.h"
#include "ink/execution/hybrid/archive.h"
#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/engine/execution_linker.h"
#include "ink/execution/engine/execution_machine.h"
#include "ink/core/core_define.h"
#include "ink/ir/context.h"
#include <llvm/Support/ConvertUTF.h>

#include <filesystem>
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <unordered_set>

namespace ink::execution
{
  namespace
  {
    struct Session;

    struct PatchTarget
    {
        std::shared_ptr<Session> Owner;
        FunctionId Function;
    };

    struct NativeTarget
    {
        BytecodeSymbol Symbol;
        RuntimeFunctionDescriptor Descriptor;
        std::string BuildId;
        const InkHybridFunction *Native = nullptr;
        std::unique_ptr<PatchTarget> Patch;
    };

    struct BaseModule
    {
        HybridArtifact Artifact;
        std::unordered_map<FunctionId, std::unique_ptr<NativeTarget>> Targets;
    };

    struct Runtime;

    struct BindingContext
    {
        Runtime *Owner;
        NativeTarget *Target;
        const RuntimeTypeTable *Types;
        RuntimeTypeId Signature;
    };

    ExecutionImage copyImage(const ExecutionImage &Image)
    {
      ExecutionImage Copy;
      Copy.Layouts = Image.Layouts;
      Copy.Descriptors = Image.Descriptors;
      for (const auto &[Id, Function] : Image.Functions)
      {
        Copy.Functions.emplace(Id, std::make_unique<ExecutableFunction>(*Function));
      }
      return Copy;
    }

    struct Session
    {
        HybridArtifact Artifact;
        ExecutionLinker Linker;
        ExecutionMachine Machine;
        std::vector<std::unique_ptr<BindingContext>> Bindings;

        Session(HybridArtifact Artifact, ExecutionEngine &Engine)
            : Artifact(std::move(Artifact)),
              Linker(copyImage(this->Artifact.Bytecode->Image)),
              Machine(Engine, Linker)
        {
        }
    };

    bool bridgeable(const RuntimeTypeTable &Types, RuntimeTypeId Id, std::unordered_set<RuntimeTypeId> &Seen)
    {
      if (!Seen.insert(Id).second)
      {
        return true;
      }
      const auto *Type = Types.get(Id);
      if (!Type || Type->Kind == RuntimeKind::Function || Type->Kind == RuntimeKind::Invalid)
      {
        return false;
      }
      if (Type->Kind == RuntimeKind::Pointer)
      {
        return bridgeable(Types, Type->pointerDesc().Pointee, Seen);
      }
      if (Type->Kind == RuntimeKind::Array)
      {
        return bridgeable(Types, Type->arrayDesc().ElementType, Seen);
      }
      if (Type->Kind == RuntimeKind::Class)
      {
        for (const auto &Field : Type->classDesc().Fields)
        {
          if (!bridgeable(Types, Field.Type, Seen))
          {
            return false;
          }
        }
      }
      return true;
    }

    bool bridgeableSignature(const RuntimeTypeTable &Types, RuntimeTypeId Id)
    {
      const auto *Type = Types.get(Id);
      if (!Type || Type->Kind != RuntimeKind::Function)
      {
        return false;
      }
      std::unordered_set<RuntimeTypeId> Seen;
      if (!bridgeable(Types, Type->functionDesc().ReturnType, Seen))
      {
        return false;
      }
      for (const auto Parameter : Type->functionDesc().Parameters)
      {
        if (!bridgeable(Types, Parameter, Seen))
        {
          return false;
        }
      }
      return true;
    }

    struct Runtime
    {
        core::CompilationContext Compilation;
        ir::IRContext Context;
        ExecutionEngine Engine;
        ExecutionMemoryManager ReturnedMemory;
        std::unordered_map<const InkHybridModule *, std::unique_ptr<BaseModule>> Modules;
        std::unordered_map<std::string, NativeTarget *> Symbols;
        std::unordered_map<std::string, NativeTarget *> Exports;
        std::unordered_map<std::string, NativeTarget *> NativeImports;
        std::size_t NativeDepth = 0;
        std::size_t VmDepth = 0;
        std::string Error;

        Runtime()
            : Context(Compilation),
              Engine(Context),
              ReturnedMemory(ExecutionLimits{}.MaxObjects, ExecutionLimits{}.MaxStorageBytes)
        {
        }

        uint32_t fail(uint32_t Status, std::string Message)
        {
          Error = std::move(Message);
          return Status;
        }

        uint32_t registerModule(const InkHybridModule *Module)
        {
          if (Modules.contains(Module))
          {
            return InkHybridSuccess;
          }
          if (NativeDepth || VmDepth)
          {
            return fail(InkHybridBusy, "Register hybrid modules before starting mixed execution");
          }
          if (!Module || Module->Version != 1 || !Module->Manifest || (Module->FunctionCount && !Module->Functions) || Module->FunctionCount > BytecodeLimits{}.MaxRecords)
          {
            return fail(InkHybridInvalidArtifact, "Invalid native hybrid module descriptor");
          }
          auto Read = deserializeHybridArtifact({Module->Manifest, Module->ManifestSize});
          if (!Read || Read.Artifact.Kind != HybridArtifactKind::Base)
          {
            return fail(InkHybridInvalidArtifact, Read ? "Expected a base manifest" : Read.Error);
          }
          auto Base = std::make_unique<BaseModule>();
          Base->Artifact = std::move(Read.Artifact);
          const auto &Object = *Base->Artifact.Bytecode;
          std::unordered_map<FunctionId, const BytecodeSymbol *> ById;
          std::unordered_set<std::string> NewExports;
          for (const auto &Symbol : Object.Symbols)
          {
            ById.emplace(Symbol.Function, &Symbol);
          }
          for (std::size_t Index = 0; Index < Module->FunctionCount; ++Index)
          {
            const auto &Native = Module->Functions[Index];
            const auto Found = ById.find(Native.Function);
            const bool Imported = (Native.Flags & 4) != 0;
            if (Found == ById.end() || Found->second->Kind != (Imported ? BytecodeSymbolKind::Native : BytecodeSymbolKind::Import) || !Native.Invoke || Native.Flags > 7 || (Imported && Native.Flags != 4) || Base->Targets.contains(Native.Function) || (!Imported && Symbols.contains(Found->second->Identity.LinkName)))
            {
              return fail(InkHybridIncompatible, "Invalid or duplicate native hybrid function");
            }
            auto Target = std::make_unique<NativeTarget>();
            Target->Symbol = *Found->second;
            Target->Descriptor = Object.Image.Descriptors.at(Native.Function);
            Target->BuildId = Base->Artifact.BuildId;
            Target->Native = &Native;
            if ((Native.Flags & 2) && (Exports.contains(Target->Descriptor.Symbol) || !NewExports.insert(Target->Descriptor.Symbol).second))
            {
              return fail(InkHybridIncompatible, "Duplicate native export in hybrid modules");
            }
            Base->Targets.emplace(Native.Function, std::move(Target));
          }
          for (const auto &Symbol : Object.Symbols)
          {
            if (!Base->Targets.contains(Symbol.Function))
            {
              return fail(InkHybridInvalidArtifact, "Missing native function in hybrid manifest");
            }
          }
          for (const auto &[Id, Target] : Base->Targets)
          {
            if (Target->Native->Flags & 4)
            {
              NativeImports.emplace(bytecodeSymbolKey(Target->Symbol.Identity), Target.get());
              continue;
            }
            Symbols.emplace(Target->Symbol.Identity.LinkName, Target.get());
            if (Target->Native->Flags & 2)
            {
              Exports.emplace(Target->Descriptor.Symbol, Target.get());
            }
          }
          Modules.emplace(Module, std::move(Base));
          Error.clear();
          return InkHybridSuccess;
        }

        ExecutionStatus invoke(PatchTarget &Patch, void *Result, const void *const *Arguments)
        {
          if (VmDepth == 0 && !Engine.beginInvocation())
          {
            Error = "Hybrid invocation started while the VM was active";
            return ExecutionStatus::InvalidFrame;
          }
          ++VmDepth;
          struct Guard
          {
              Runtime &Owner;
              ~Guard()
              {
                --Owner.VmDepth;
              }
          } Scope{*this};
          auto &Session = *Patch.Owner;
          const auto &Types = *Session.Artifact.Bytecode->Image.Layouts;
          const auto &Descriptor = Session.Artifact.Bytecode->Image.Descriptors.at(Patch.Function);
          const auto &Signature = Types.get(Descriptor.Signature)->functionDesc();
          std::vector<RuntimeValue> Values;
          for (std::size_t Index = 0; Index < Signature.Parameters.size(); ++Index)
          {
            if (!Arguments || !Arguments[Index])
            {
              Error = "Missing hybrid argument storage";
              return ExecutionStatus::InvalidArguments;
            }
            auto Value = readStorage(*Types.get(Signature.Parameters[Index]), Arguments[Index]);
            if (!Value)
            {
              Error = "Invalid hybrid argument representation";
              return Value.Status;
            }
            Values.push_back(std::move(Value.Value));
          }
          const auto Returned = Session.Machine.executeBytecode(Patch.Function, Values);
          if (!Returned)
          {
            Error = "Bytecode patch execution failed in " + Descriptor.Symbol + " (status " + std::to_string(static_cast<unsigned>(Returned.Status)) + ")";
            return Returned.Status;
          }
          const auto &Layout = *Types.get(Signature.ReturnType);
          if (Layout.Kind != RuntimeKind::Void && !Result)
          {
            Error = "Missing hybrid result storage";
            return ExecutionStatus::InvalidArguments;
          }
          // Returned strings are retained independently of replaceable code images.
          const auto Status = ReturnedMemory.writeValueBytes(Layout, Result, Returned.Value);
          if (Status != ExecutionStatus::Success)
          {
            Error = "Hybrid result storage budget exceeded";
          }
          return Status;
        }

        static ExecutionStatus dispatch(void *Opaque, void *Result, const void *const *Arguments)
        {
          auto &Binding = *static_cast<BindingContext *>(Opaque);
          if (Binding.Target->Patch)
          {
            return Binding.Owner->invoke(*Binding.Target->Patch, Result, Arguments);
          }
          Binding.Target->Native->Invoke(Result, Arguments);
          // Native constructors initialize the same storage that the VM allocated.
          // Publish that initialization only after the constructor returns normally.
          if (Binding.Target->Symbol.Identity.Name == "__init__")
          {
            const auto &Signature = Binding.Types->get(Binding.Signature)->functionDesc();
            if (!Signature.Parameters.empty())
            {
              const auto *Receiver = Binding.Types->get(Signature.Parameters.front());
              if (Receiver && Receiver->Kind == RuntimeKind::Pointer)
              {
                void *Address = nullptr;
                std::memcpy(&Address, Arguments[0], sizeof(Address));
                return Binding.Owner->Engine.heap().memoryManager().storeBytes(Address, *Binding.Types->get(Receiver->pointerDesc().Pointee), Address);
              }
            }
          }
          return ExecutionStatus::Success;
        }

        uint32_t apply(HybridArtifact Artifact)
        {
          if (NativeDepth || VmDepth)
          {
            return fail(InkHybridBusy, "Install patches at a quiescent host boundary");
          }
          if (Artifact.Kind != HybridArtifactKind::Patch || !Artifact.Bytecode)
          {
            return fail(InkHybridInvalidArtifact, "Expected a bytecode patch package");
          }
          bool MatchingBuild = false;
          for (const auto &[Module, Base] : Modules)
          {
            MatchingBuild = MatchingBuild || Base->Artifact.BuildId == Artifact.BuildId;
            if (const auto Check = compatibleHybridTypes(*Base->Artifact.Bytecode->Image.Layouts, *Artifact.Bytecode->Image.Layouts); !Check)
            {
              return fail(InkHybridIncompatible, Check.Message);
            }
          }
          if (!MatchingBuild)
          {
            return fail(InkHybridIncompatible, "Patch does not match a registered native build");
          }
          auto Prepared = std::make_shared<Session>(std::move(Artifact), Engine);
          std::vector<std::pair<NativeTarget *, FunctionId>> Replacements;
          const auto &Object = *Prepared->Artifact.Bytecode;
          for (const auto &Symbol : Object.Symbols)
          {
            const auto &Descriptor = Object.Image.Descriptors.at(Symbol.Function);
            NativeTarget *Target = nullptr;
            if (Symbol.Kind == BytecodeSymbolKind::Native)
            {
              const auto Found = Exports.find(Descriptor.Symbol);
              Target = Found == Exports.end() ? nullptr : Found->second;
              if (!Target)
              {
                const auto Imported = NativeImports.find(bytecodeSymbolKey(Symbol.Identity));
                Target = Imported == NativeImports.end() ? nullptr : Imported->second;
                if (!Target)
                {
                  if (!Descriptor.Supported || !findNativeSymbol(Descriptor.Symbol))
                  {
                    return fail(InkHybridIncompatible, "Patch has an unavailable native import: " + Descriptor.Symbol);
                  }
                  continue;
                }
              }
            }
            else
            {
              const auto Found = Symbols.find(Symbol.Identity.LinkName);
              Target = Found == Symbols.end() ? nullptr : Found->second;
            }
            if (!Target)
            {
              const bool ExistingName = std::any_of(Symbols.begin(), Symbols.end(), [&](const auto &Existing)
              {
                return Existing.second->Symbol.Identity.Module == Symbol.Identity.Module && Existing.second->Symbol.Identity.Name == Symbol.Identity.Name;
              });
              if (!ExistingName && Symbol.Kind == BytecodeSymbolKind::Definition && Symbol.Visibility == BytecodeVisibility::Private)
              {
                continue;
              }
              return fail(InkHybridIncompatible, "Patch has an unresolved or incompatible function: " + Symbol.Identity.Module + "#" + Symbol.Identity.Name);
            }
            if (Target->Symbol.Identity.Signature != Symbol.Identity.Signature || Target->Descriptor.CAbi != Descriptor.CAbi || !bridgeableSignature(*Object.Image.Layouts, Descriptor.Signature))
            {
              return fail(InkHybridIncompatible, "Incompatible hybrid signature: " + Symbol.Identity.Name);
            }
            if (Symbol.Kind == BytecodeSymbolKind::Definition)
            {
              if (Target->BuildId != Prepared->Artifact.BuildId)
              {
                return fail(InkHybridIncompatible, "Patch attempts to replace a function from another native build");
              }
              if (!(Target->Native->Flags & 1))
              {
                return fail(InkHybridIncompatible, "Function was not built for hot reload: " + Symbol.Identity.Name);
              }
              Replacements.emplace_back(Target, Symbol.Function);
            }
            auto Binding = std::make_unique<BindingContext>(BindingContext{this, Target, Object.Image.Layouts.get(), Descriptor.Signature});
            Prepared->Linker.bindNative(Symbol.Function, {dispatch, Binding.get()});
            Prepared->Bindings.push_back(std::move(Binding));
          }
          if (Replacements.empty())
          {
            return fail(InkHybridIncompatible, "Patch does not replace any native function");
          }
          // Validate and prepare every body before publishing any replacement.
          for (const auto &[Id, Function] : Object.Image.Functions)
          {
            ExecutionStatus Status = ExecutionStatus::Success;
            if (!Prepared->Linker.prepare(Id, Status))
            {
              return fail(InkHybridInvalidArtifact, "Patch contains an invalid bytecode function");
            }
          }
          for (const auto &[Target, Id] : Replacements)
          {
            Target->Patch = std::make_unique<PatchTarget>(PatchTarget{Prepared, Id});
          }
          Error.clear();
          return InkHybridSuccess;
        }
    };

    Runtime &runtime()
    {
      thread_local Runtime Value;
      return Value;
    }
  } // namespace
} // namespace ink::execution

extern "C"
{
  uint32_t ink_hybrid_register(const InkHybridModule *Module)
  {
    return ink::execution::runtime().registerModule(Module);
  }

  uint32_t ink_hybrid_apply(const char *Path)
  {
    auto &Runtime = ink::execution::runtime();
    if (!Path)
    {
      return Runtime.fail(InkHybridInvalidArtifact, "Missing patch path");
    }
    const std::string_view Text(Path);
    const auto *Begin = reinterpret_cast<const llvm::UTF8 *>(Text.data());
    if (!llvm::isLegalUTF8String(&Begin, Begin + Text.size()))
    {
      return Runtime.fail(InkHybridInvalidArtifact, "Patch path is not valid UTF-8");
    }
    auto Read = ink::execution::readHybridFile(std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t *>(Text.data()), Text.size())));
    return Read ? Runtime.apply(std::move(Read.Artifact)) : Runtime.fail(InkHybridInvalidArtifact, Read.Error);
  }

  uint32_t ink_hybrid_apply_bytes(const char *Bytes, size_t Length)
  {
    auto &Runtime = ink::execution::runtime();
    if (!Bytes)
    {
      return Runtime.fail(InkHybridInvalidArtifact, "Missing patch bytes");
    }
    auto Read = ink::execution::deserializeHybridArtifact({Bytes, Length});
    return Read ? Runtime.apply(std::move(Read.Artifact)) : Runtime.fail(InkHybridInvalidArtifact, Read.Error);
  }

  uint32_t ink_hybrid_clear(void)
  {
    auto &Runtime = ink::execution::runtime();
    if (Runtime.NativeDepth || Runtime.VmDepth)
    {
      return Runtime.fail(InkHybridBusy, "Remove patches at a quiescent host boundary");
    }
    for (auto &[Name, Target] : Runtime.Symbols)
    {
      Target->Patch.reset();
    }
    Runtime.Error.clear();
    return InkHybridSuccess;
  }

  const char *ink_hybrid_last_error(void)
  {
    return ink::execution::runtime().Error.c_str();
  }

  void *ink_hybrid_enter(const InkHybridModule *Module, uint32_t Function)
  {
    auto &Runtime = ink::execution::runtime();
    if (Runtime.registerModule(Module) != InkHybridSuccess)
    {
      ink_hybrid_panic();
    }
    const auto &Targets = Runtime.Modules.at(Module)->Targets;
    const auto Found = Targets.find(Function);
    if (Found == Targets.end())
    {
      Runtime.Error = "Invalid hot function identity";
      ink_hybrid_panic();
    }
    ++Runtime.NativeDepth;
    return Found->second->Patch.get();
  }

  void ink_hybrid_leave(void)
  {
    auto &Runtime = ink::execution::runtime();
    if (Runtime.NativeDepth == 0)
    {
      PANIC("Unbalanced hybrid function exit");
    }
    --Runtime.NativeDepth;
  }

  uint32_t ink_hybrid_invoke(void *Patch, void *Result, const void *const *Arguments)
  {
    if (!Patch)
    {
      return InkHybridInvalidArtifact;
    }
    return ink::execution::runtime().invoke(*static_cast<ink::execution::PatchTarget *>(Patch), Result, Arguments) == ink::execution::ExecutionStatus::Success ? InkHybridSuccess : InkHybridExecutionFailed;
  }

  void ink_hybrid_panic(void)
  {
    PANIC(ink_hybrid_last_error());
  }
}
