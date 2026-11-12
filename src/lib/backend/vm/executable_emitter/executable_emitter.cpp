#include "ink/backend/vm/executable_emitter/executable_emitter.h"

#include "ink/backend/vm/module/machine_module.h"

namespace ink::vm
{
  ExecutableEmitter::ExecutableEmitter(const core::DiagnosticEngine &Diagnostics) noexcept
      : Context(Diagnostics),
        FunctionEmitter(Context)
  {
  }

  ExecutableFile ExecutableEmitter::emit(const MachineModule &Module)
  {
    Context.reset();
    scanUnResolvedSymbol(Module);
    padGlobals(Module);
    padFunction(Module);
    resolveStaticRelocations();
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
} // namespace ink::vm
