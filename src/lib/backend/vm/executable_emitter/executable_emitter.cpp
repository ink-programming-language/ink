#include "ink/backend/vm/executable_emitter/executable_emitter.h"

#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"

#include <unordered_set>

namespace ink::vm
{
  ExecutableEmitter::ExecutableEmitter(const core::DiagnosticEngine &Diagnostics) noexcept
      : Context(Diagnostics),
        FunctionEmitter(Context)
  {
  }

  ExecutableFile ExecutableEmitter::emit(const MachineModule &Module, core::Name EntryFunctionName, std::span<const core::Name> ExportNames)
  {
    Context.reset();
    scanUnResolvedSymbol(Module);
    padGlobals(Module);
    padFunction(Module);
    resolveStaticRelocations();
    resolveEntryOffset(Module, EntryFunctionName);
    generateExportSymbols(ExportNames);
    return Context.takeExecutable();
  }

  void ExecutableEmitter::scanUnResolvedSymbol(const MachineModule &Module)
  {
    Context.scanUnResolvedSymbol(Module);
  }

  void ExecutableEmitter::padGlobals(const MachineModule &Module)
  {
    for (const MachineGlobalVariable &Global : Module.globalVariables())
    {
      const std::uint64_t Remainder = Context.byteSize() % Global.alignment();
      const std::uint64_t Padding = Remainder == 0 ? 0 : Global.alignment() - Remainder;
      Context.padZero(static_cast<std::size_t>(Padding));
      const std::uint64_t Offset = Context.byteSize();
      const auto InitialBytes = Global.initialBytes();
      Context.pad(InitialBytes.data(), InitialBytes.size());
      Context.padZero(static_cast<std::size_t>(Global.byteSize() - InitialBytes.size()));
      Context.resolveSymbol(Global.name(), Offset);
    }
  }

  void ExecutableEmitter::padFunction(const MachineModule &Module)
  {
    for (const MachineFunction &Function : Module.functions())
    {
      Context.resolveSymbol(Function.name(), Context.byteSize());
      FunctionEmitter.emit(Function);
    }
  }

  void ExecutableEmitter::resolveStaticRelocations()
  {
    Context.resolveStaticRelocations();
  }

  void ExecutableEmitter::resolveEntryOffset(const MachineModule &Module, core::Name EntryFunctionName)
  {
    const auto &Diagnostics = Context.getDiagnostics();
    REPORT_ICE_IF(!core::NamePool::instance().contains(EntryFunctionName), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "entry point", "entry function name must be registered and nonempty");
    const MachineFunction *EntryFunction = nullptr;
    for (const MachineFunction &Function : Module.functions())
    {
      if (Function.name() == EntryFunctionName)
      {
        EntryFunction = &Function;
        break;
      }
    }
    REPORT_ICE_IF(!EntryFunction, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "entry point", "entry must name a defined VM function");
    bool HasInstructions = false;
    for (const MachineBasicBlock &Block : EntryFunction->basicBlocks())
    {
      if (Block.instructionCount() != 0)
      {
        HasInstructions = true;
        break;
      }
    }
    REPORT_ICE_IF(!HasInstructions, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "entry point", "entry function must contain at least one instruction");
    const auto Offset = Context.resolvedSymbolOffset(EntryFunctionName);
    REPORT_ICE_IF(!Offset || *Offset >= Context.byteSize(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "entry point", "entry function has no instruction offset in the image");
    Context.setEntryOffset(*Offset);
  }

  void ExecutableEmitter::generateExportSymbols(std::span<const core::Name> ExportNames)
  {
    const auto &Diagnostics = Context.getDiagnostics();
    std::unordered_set<core::Name> ExportedNames;
    for (const core::Name Name : ExportNames)
    {
      REPORT_ICE_IF(!core::NamePool::instance().contains(Name), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "export symbol name must be registered and nonempty");
      REPORT_ICE_IF(!ExportedNames.insert(Name).second, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "export symbol name is duplicated");
      const auto Offset = Context.resolvedSymbolOffset(Name);
      REPORT_ICE_IF(!Offset, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "export symbol has no VM image offset");
      REPORT_ICE_IF(*Offset >= Context.byteSize(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "export symbol", "export symbol offset is outside the image");
      Context.addExportSymbol({Name, *Offset});
    }
  }
} // namespace ink::vm
