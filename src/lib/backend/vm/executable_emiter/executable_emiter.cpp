#include "ink/backend/vm/executable_emiter/executable_emiter.h"

#include "ink/backend/vm/module/machine_module.h"

#include <utility>

namespace ink::vm
{
  ExecutableEmiter::ExecutableEmiter(const core::DiagnosticEngine &Diagnostics) noexcept
      : Diagnostics(Diagnostics)
  {
  }

  ExecutableFile ExecutableEmiter::emit(const MachineModule &Module)
  {
    Executable = ExecutableFile{};
    ResolvedSymbol.clear();
    scanUnResolvedSymbol(Module);
    padGlobals(Module);
    return std::move(Executable);
  }

  void ExecutableEmiter::scanUnResolvedSymbol(const MachineModule &Module)
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

  void ExecutableEmiter::pad(const void *Src, std::size_t Count)
  {
    if (Count == 0)
    {
      return;
    }
    const auto *Bytes = static_cast<const std::uint8_t *>(Src);
    auto &Image = Executable.getImage();
    Image.insert(Image.end(), Bytes, Bytes + Count);
  }

  void ExecutableEmiter::padZero(std::size_t Count)
  {
    auto &Image = Executable.getImage();
    Image.insert(Image.end(), Count, std::uint8_t{0});
  }

  void ExecutableEmiter::padGlobals(const MachineModule &Module)
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
