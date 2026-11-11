#include "ink/backend/vm/executable_emitter/executable_emitter.h"

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"

#include <bit>
#include <limits>
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
    PendingStaticRelocationSymbol.clear();
    scanUnResolvedSymbol(Module);
    padGlobals(Module);
    return std::move(Executable);
  }

  void ExecutableEmitter::scanUnResolvedSymbol(const MachineModule &Module)
  {
    UnresolvedSymbol.clear();
    NativeSymbols.clear();
    const auto AddUnResolvedSymbol = [&](core::Name Name)
    {
      const auto [Position, Inserted] = UnresolvedSymbol.try_emplace(Name, 0);
      REPORT_ICE_IF(!Inserted && Position->first != core::NAME_None, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "scanUnResolvedSymbol", "VM symbol name has multiple definitions");
    };
    const auto AddNativeSymbol = [&](core::Name Name, DynamicImportSymbol Import)
    {
      const auto [Position, Inserted] = NativeSymbols.try_emplace(Name, Import);
      REPORT_ICE_IF(!Inserted && Position->second.Name != Import.Name, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "scanUnResolvedSymbol", "native alias has conflicting import identities");
    };

    for (const MachineFunction &Function : Module.functions())
    {
      AddUnResolvedSymbol(Function.name());
    }
    for (const MachineGlobalVariable &Global : Module.globalVariables())
    {
      AddUnResolvedSymbol(Global.name());
    }
    for (const auto &Declaration : Module.nativeFunctions())
    {
      AddNativeSymbol(Declaration.Name, DynamicImportSymbol{Declaration.Import.Name});
    }
    for (const auto &Declaration : Module.nativeData())
    {
      AddNativeSymbol(Declaration.Name, DynamicImportSymbol{Declaration.Import.Name});
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

  void ExecutableEmitter::padRegister(RegisterId Register, std::int64_t ByteOffset)
  {
    REPORT_ICE_IF(!isAddressRegister(Register), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "register operand", "register must be BP, SP, or PC");
    padInteger(static_cast<std::uint8_t>(Register), 8);
    padInteger(static_cast<std::uint64_t>(ByteOffset), 64);
  }

  void ExecutableEmitter::padUnresolvedSymbol(core::Name Symbol, std::int64_t Addend, std::uint64_t InstructionOffset)
  {
    padInteger(static_cast<std::uint8_t>(RegisterId::PC), 8);
    PendingStaticRelocationSymbol.push_back({Executable.byteSize(), Symbol, InstructionOffset, Addend});
    padZero(8);
  }

  void ExecutableEmitter::padResolvedSymbol(std::uint64_t SymbolOffset, std::int64_t Addend, std::uint64_t InstructionOffset)
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
    const bool Forward = Target >= InstructionOffset;
    const auto Distance = Forward ? Target - InstructionOffset : InstructionOffset - Target;
    const auto Limit = Forward ? static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) : (std::uint64_t{1} << 63);
    REPORT_ICE_IF(Distance > Limit, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "PC-relative displacement exceeds i64 range");
    padInteger(static_cast<std::uint8_t>(RegisterId::PC), 8);
    padInteger(Forward ? Distance : std::uint64_t{0} - Distance, 64);
  }

  void ExecutableEmitter::padNativeSymbol(const DynamicImportSymbol &Import, std::int64_t Addend)
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
    padInteger(0xFF, 8);
    Executable.getDynamicImportSymbolRelocations().push_back({Executable.byteSize(), static_cast<std::uint32_t>(Index), Addend});
    padZero(8);
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

  void ExecutableEmitter::padOperand(const RegisterOperand &Operand)
  {
    padRegister(Operand.getRegister(), Operand.getByteOffset());
  }

  void ExecutableEmitter::padOperand(const AddressOperand &Operand, std::uint64_t InstructionOffset)
  {
    if (const auto Register = Operand.getRegister())
    {
      padRegister(*Register, Operand.getByteOffset());
      return;
    }

    const auto Symbol = Operand.getSymbol();
    REPORT_ICE_IF(!Symbol || !core::NamePool::instance().contains(*Symbol), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol name must be registered and nonempty");
    const auto Position = ResolvedSymbol.find(*Symbol);
    const auto Native = NativeSymbols.find(*Symbol);
    REPORT_ICE_IF(Native != NativeSymbols.end() && (Position != ResolvedSymbol.end() || UnresolvedSymbol.contains(*Symbol)), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has both VM and native definitions");
    if (Position != ResolvedSymbol.end())
    {
      padResolvedSymbol(Position->second, Operand.getByteOffset(), InstructionOffset);
      return;
    }

    if (UnresolvedSymbol.contains(*Symbol))
    {
      padUnresolvedSymbol(*Symbol, Operand.getByteOffset(), InstructionOffset);
      return;
    }

    REPORT_ICE_IF(Native == NativeSymbols.end(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has no resolved VM offset or explicit native declaration");
    padNativeSymbol(Native->second, Operand.getByteOffset());
  }
} // namespace ink::vm
