#include "ink/backend/vm/executable_emitter/executable_emitter.h"

#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/support/core_define.h"

#include <bit>
#include <utility>

namespace ink::vm
{
  ExecutableEmitter::ExecutableEmitter(const core::DiagnosticEngine &Diagnostics) noexcept
      : Diagnostics(Diagnostics)
  {
  }

  ExecutableFile ExecutableEmitter::emit(const MachineModule &Module)
  {
    static_assert(std::endian::native == std::endian::little, "VM integer emission requires a little-endian host");
    Executable = ExecutableFile{};
    ResolvedSymbol.clear();
    scanUnResolvedSymbol(Module);
    padGlobals(Module);
    return std::move(Executable);
  }

  void ExecutableEmitter::scanUnResolvedSymbol(const MachineModule &Module)
  {
    UnresolvedSymbol.clear();
    for (const MachineFunction &Function : Module.functions())
    {
      UnresolvedSymbol.try_emplace(Function.name(), 0);
    }
    for (const MachineGlobalVariable &Global : Module.globalVariables())
    {
      UnresolvedSymbol.try_emplace(Global.name(), 0);
    }
  }

  void ExecutableEmitter::pad(const void *Src, std::size_t Count)
  {
    if (Count == 0)
    {
      return;
    }
    const auto *Bytes = static_cast<const std::uint8_t *>(Src);
    auto &Image = Executable.getImage();
    Image.insert(Image.end(), Bytes, Bytes + Count);
  }

  void ExecutableEmitter::padZero(std::size_t Count)
  {
    auto &Image = Executable.getImage();
    Image.insert(Image.end(), Count, std::uint8_t{0});
  }

  void ExecutableEmitter::padInteger(std::uint64_t Bits, std::uint32_t Width)
  {
    REPORT_ICE_IF(Width != 8 && Width != 16 && Width != 32 && Width != 64, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "executable emission", "integer encoding requires a supported width");
    const std::size_t Count = Width / 8;
    pad(&Bits, Count);
  }

  void ExecutableEmitter::padGlobals(const MachineModule &Module)
  {
    for (const MachineGlobalVariable &Global : Module.globalVariables())
    {
      const std::uint64_t Remainder = Executable.byteSize() % Global.alignment();
      const std::uint64_t Padding = Remainder == 0 ? 0 : Global.alignment() - Remainder;
      padZero(static_cast<std::size_t>(Padding));
      const std::uint64_t Offset = Executable.byteSize();
      const auto InitialBytes = Global.initialBytes();
      pad(InitialBytes.data(), InitialBytes.size());
      padZero(static_cast<std::size_t>(Global.byteSize() - InitialBytes.size()));
      ResolvedSymbol.insert_or_assign(Global.name(), Offset);
      UnresolvedSymbol.erase(Global.name());
    }
  }
} // namespace ink::vm
