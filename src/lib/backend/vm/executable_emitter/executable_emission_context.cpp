#include "ink/backend/vm/executable_emitter/executable_emission_context.h"

#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"

#include <bit>
#include <cstring>
#include <limits>
#include <utility>

namespace ink::vm
{
  ExecutableEmissionContext::ExecutableEmissionContext(const core::DiagnosticEngine &Diagnostics) noexcept
      : Diagnostics(Diagnostics)
  {
    static_assert(std::endian::native == std::endian::little, "VM integer emission requires a little-endian host");
  }

  ExecutableFile ExecutableEmissionContext::takeExecutable()
  {
    return std::move(Executable);
  }

  void ExecutableEmissionContext::reset()
  {
    Executable = ExecutableFile{};
    ResolvedSymbol.clear();
    PendingStaticRelocationSymbol.clear();
  }

  void ExecutableEmissionContext::scanUnResolvedSymbol(const MachineModule &Module)
  {
    UnresolvedSymbol.clear();
    NativeSymbols.clear();
    const auto AddUnResolvedSymbol = [&](core::Name Name)
    {
      const auto [Position, Inserted] = UnresolvedSymbol.try_emplace(Name, 0);
      REPORT_ICE_IF(!Inserted && Position->first != core::NAME_None, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "scanUnResolvedSymbol", "VM symbol name has multiple definitions");
    };
    for (const MachineFunction &Function : Module.functions())
    {
      AddUnResolvedSymbol(Function.name());
    }
    for (const MachineGlobalVariable &Global : Module.globalVariables())
    {
      AddUnResolvedSymbol(Global.name());
    }
    for (const MachineSymbolDeclaration &Declaration : Module.symbolDeclarations())
    {
      if (!UnresolvedSymbol.contains(Declaration.Name))
      {
        NativeSymbols.insert(Declaration.Name);
      }
    }
  }

  void ExecutableEmissionContext::resolveSymbol(core::Name Name, std::uint64_t ImageOffset)
  {
    ResolvedSymbol.insert_or_assign(Name, ImageOffset);
    UnresolvedSymbol.erase(Name);
  }

  std::optional<std::uint64_t> ExecutableEmissionContext::resolvedSymbolOffset(core::Name Name) const noexcept
  {
    const auto Position = ResolvedSymbol.find(Name);
    if (Position == ResolvedSymbol.end())
    {
      return std::nullopt;
    }
    return Position->second;
  }

  bool ExecutableEmissionContext::hasUnResolvedSymbol(core::Name Name) const noexcept
  {
    return UnresolvedSymbol.contains(Name);
  }

  bool ExecutableEmissionContext::isNativeSymbol(core::Name Name) const noexcept
  {
    return NativeSymbols.contains(Name);
  }

  void ExecutableEmissionContext::pad(const void *Src, std::size_t Count)
  {
    if (Count == 0)
    {
      return;
    }
    const auto *Bytes = static_cast<const std::uint8_t *>(Src);
    auto &Image = Executable.getImage();
    Image.insert(Image.end(), Bytes, Bytes + Count);
  }

  void ExecutableEmissionContext::padZero(std::size_t Count)
  {
    auto &Image = Executable.getImage();
    Image.insert(Image.end(), Count, std::uint8_t{0});
  }

  void ExecutableEmissionContext::padInteger(std::uint64_t Bits, std::uint32_t Width)
  {
    REPORT_ICE_IF(Width != 8 && Width != 16 && Width != 32 && Width != 64, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "executable emission", "integer encoding requires a supported width");
    const std::size_t Count = Width / 8;
    pad(&Bits, Count);
  }

  void ExecutableEmissionContext::patchInteger(std::uint64_t ImageOffset, std::uint64_t Bits, std::uint32_t Width)
  {
    REPORT_ICE_IF(Width != 8 && Width != 16 && Width != 32 && Width != 64, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "executable emission", "integer encoding requires a supported width");
    const std::size_t Count = Width / 8;
    std::memcpy(Executable.getImage().data() + ImageOffset, &Bits, Count);
  }

  std::uint64_t ExecutableEmissionContext::resolveSymbolOffset(std::uint64_t SymbolOffset, std::int64_t Addend) const
  {
    std::uint64_t Target = SymbolOffset;
    if (Addend >= 0)
    {
      const auto Increment = static_cast<std::uint64_t>(Addend);
      REPORT_ICE_IF(Increment > std::numeric_limits<std::uint64_t>::max() - Target, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "VM symbol address addition overflows");
      Target += Increment;
    }
    else
    {
      const auto Decrement = std::uint64_t{0} - static_cast<std::uint64_t>(Addend);
      REPORT_ICE_IF(Decrement > Target, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "VM symbol address addition underflows");
      Target -= Decrement;
    }
    return Target;
  }

  std::uint64_t ExecutableEmissionContext::resolveRegisterRelativeOffset(RegisterId Register, std::uint64_t SymbolOffset, std::int64_t Addend, std::uint64_t InstructionOffset) const
  {
    const auto Target = resolveSymbolOffset(SymbolOffset, Addend);
    if (Register == RegisterId::ZERO)
    {
      return Target;
    }
    const bool Forward = Target >= InstructionOffset;
    const auto Distance = Forward ? Target - InstructionOffset : InstructionOffset - Target;
    const auto Limit = Forward ? static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) : (std::uint64_t{1} << 63);
    REPORT_ICE_IF(Distance > Limit, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "register-relative displacement exceeds i64 range");
    return Forward ? Distance : std::uint64_t{0} - Distance;
  }

  std::uint32_t ExecutableEmissionContext::getOrAddNativeSymbol(const DynamicImportSymbol &Import)
  {
    REPORT_ICE_IF(!core::NamePool::instance().contains(Import.Name), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "native import has an invalid host name");
    auto &Imports = Executable.getDynamicImportSymbols();
    std::size_t Index = 0;
    while (Index < Imports.size() && Imports[Index].Name != Import.Name)
    {
      ++Index;
    }
    REPORT_ICE_IF(Index > std::numeric_limits<std::uint32_t>::max(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "native import index exceeds u32 range");
    if (Index == Imports.size())
    {
      Imports.push_back(Import);
    }
    return static_cast<std::uint32_t>(Index);
  }

  void ExecutableEmissionContext::addStaticRelocation(StaticRelocationSymbol Relocation)
  {
    PendingStaticRelocationSymbol.push_back(Relocation);
  }

  void ExecutableEmissionContext::addImageBaseAddressRelocation(ImageBaseAddressRelocation Relocation)
  {
    Executable.getImageBaseAddressRelocations().push_back(Relocation);
  }

  void ExecutableEmissionContext::addDynamicImportSymbolRelocation(DynamicImportSymbolRelocation Relocation)
  {
    Executable.getDynamicImportSymbolRelocations().push_back(Relocation);
  }

  void ExecutableEmissionContext::addExportSymbol(ExportSymbol Symbol)
  {
    Executable.getExportSymbols().push_back(Symbol);
  }

  void ExecutableEmissionContext::resolveStaticRelocations()
  {
    for (const auto &Relocation : PendingStaticRelocationSymbol)
    {
      const auto Position = ResolvedSymbol.find(Relocation.Name);
      REPORT_ICE_IF(Position == ResolvedSymbol.end(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "static relocation", "VM symbol has no image offset after layout");

      if (Relocation.Register == RegisterId::ZERO)
      {
        addImageBaseAddressRelocation({Relocation.ImageOffset, Position->second, Relocation.Addend});
      }
      else
      {
        const auto Bits = resolveRegisterRelativeOffset(Relocation.Register, Position->second, Relocation.Addend, Relocation.InstructionOffset);
        patchInteger(Relocation.ImageOffset, Bits, 64);
      }
    }
    PendingStaticRelocationSymbol.clear();
  }
} // namespace ink::vm
