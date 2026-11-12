#include "ink/backend/vm/executable_emitter/executable_emitter.h"

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/machine_instruction.h"
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
      : Diagnostics(Diagnostics),
        FunctionEmitter(*this)
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
    padFunction(Module);
    resolveStaticRelocations();
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
    for (const auto &[Name, Import] : Module.nativeFunctions())
    {
      AddNativeSymbol(Name, DynamicImportSymbol{Import.Name});
    }
    for (const auto &[Name, Import] : Module.nativeData())
    {
      AddNativeSymbol(Name, DynamicImportSymbol{Import.Name});
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
    REPORT_ICE_IF(!isAddressRegister(Register), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "register operand", "register must be BP, SP, PC, or ZERO");
    padInteger(static_cast<std::uint8_t>(Register), 8);
    padInteger(static_cast<std::uint64_t>(ByteOffset), 64);
  }

  void ExecutableEmitter::padUnresolvedSymbol(RegisterId Register, core::Name Symbol, std::int64_t Addend, std::uint64_t InstructionOffset)
  {
    padInteger(static_cast<std::uint8_t>(Register), 8);
    PendingStaticRelocationSymbol.push_back({Executable.byteSize(), Symbol, InstructionOffset, Addend, Register});
    padZero(8);
  }

  std::uint64_t ExecutableEmitter::resolveSymbolOffset(std::uint64_t SymbolOffset, std::int64_t Addend) const
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

  std::uint64_t ExecutableEmitter::resolveRegisterRelativeOffset(RegisterId Register, std::uint64_t SymbolOffset, std::int64_t Addend, std::uint64_t InstructionOffset) const
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

  void ExecutableEmitter::resolveStaticRelocations()
  {
    for (const auto &Relocation : PendingStaticRelocationSymbol)
    {
      const auto Position = ResolvedSymbol.find(Relocation.Name);
      REPORT_ICE_IF(Position == ResolvedSymbol.end(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "static relocation", "VM symbol has no image offset after layout");

      if (Relocation.Register == RegisterId::ZERO)
      {
        Executable.getImageBaseAddressRelocations().push_back({Relocation.ImageOffset, Position->second, Relocation.Addend});
      }
      else
      {
        const auto Bits = resolveRegisterRelativeOffset(Relocation.Register, Position->second, Relocation.Addend, Relocation.InstructionOffset);
        std::memcpy(Executable.getImage().data() + Relocation.ImageOffset, &Bits, sizeof(Bits));
      }
    }
    PendingStaticRelocationSymbol.clear();
  }

  void ExecutableEmitter::padRegisterRelativeSymbol(RegisterId Register, std::uint64_t SymbolOffset, std::int64_t Addend, std::uint64_t InstructionOffset)
  {
    if (Register == RegisterId::ZERO)
    {
      padInteger(static_cast<std::uint8_t>(Register), 8);
      Executable.getImageBaseAddressRelocations().push_back({Executable.byteSize(), SymbolOffset, Addend});
      padZero(8);
      return;
    }
    const auto Bits = resolveRegisterRelativeOffset(Register, SymbolOffset, Addend, InstructionOffset);
    padInteger(static_cast<std::uint8_t>(Register), 8);
    padInteger(Bits, 64);
  }

  std::uint32_t ExecutableEmitter::getOrAddNativeSymbol(const DynamicImportSymbol &Import)
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

  void ExecutableEmitter::padNativeSymbol(const DynamicImportSymbol &Import, std::int64_t Addend)
  {
    const auto Index = getOrAddNativeSymbol(Import);
    padInteger(static_cast<std::uint8_t>(RegisterId::ZERO), 8);
    Executable.getDynamicImportSymbolRelocations().push_back({Executable.byteSize(), Index, Addend});
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

  void ExecutableEmitter::padFunction(const MachineModule &Module)
  {
    for (const MachineFunction &Function : Module.functions())
    {
      FunctionEmitter.emit(Function);
    }
  }

  void ExecutableEmitter::padInstruction(const MachineInstruction &Instruction)
  {
    const auto InstructionOffset = Executable.byteSize();
    padInteger(static_cast<std::uint16_t>(Instruction.getOpcode()), 16);
    for (std::size_t Index = 0; Index < Instruction.operandCount(); ++Index)
    {
      const Operand &Operand = *Instruction.getOperand(Index);
      switch (Operand.getKind())
      {
      case OperandKind::Register:
        padOperand(static_cast<const RegisterOperand &>(Operand));
        break;
      case OperandKind::Address:
        padOperand(static_cast<const AddressOperand &>(Operand), InstructionOffset);
        break;
      case OperandKind::Immediate:
        padOperand(static_cast<const ImmediateOperand &>(Operand), Instruction.bitWidth());
        break;
      case OperandKind::BasicBlock:
        padOperand(static_cast<const BasicBlockOperand &>(Operand), InstructionOffset);
        break;
      }
    }
  }

  void ExecutableEmitter::padOperand(const RegisterOperand &Operand)
  {
    padRegister(Operand.getRegister(), Operand.getByteOffset());
  }

  void ExecutableEmitter::padOperand(const ImmediateOperand &Operand, std::uint32_t Width)
  {
    REPORT_ICE_IF(Width != 8 && Width != 16 && Width != 32 && Width != 64, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "integer encoding requires a supported width");
    if (const auto Bits = Operand.bits())
    {
      padInteger(*Bits, Width);
      return;
    }

    REPORT_ICE_IF(Width != 64, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol address requires a 64-bit immediate field");
    const auto Symbol = Operand.getSymbol();
    REPORT_ICE_IF(!Symbol || !core::NamePool::instance().contains(*Symbol), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol name must be registered and nonempty");
    const auto Addend = *Operand.getByteOffset();
    const auto Position = ResolvedSymbol.find(*Symbol);
    const auto Native = NativeSymbols.find(*Symbol);
    REPORT_ICE_IF(Native != NativeSymbols.end() && (Position != ResolvedSymbol.end() || UnresolvedSymbol.contains(*Symbol)), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has both VM and native definitions");
    if (Position != ResolvedSymbol.end())
    {
      Executable.getImageBaseAddressRelocations().push_back({Executable.byteSize(), Position->second, Addend});
      padZero(8);
      return;
    }

    if (UnresolvedSymbol.contains(*Symbol))
    {
      PendingStaticRelocationSymbol.push_back({Executable.byteSize(), *Symbol, 0, Addend, RegisterId::ZERO});
      padZero(8);
      return;
    }

    REPORT_ICE_IF(Native == NativeSymbols.end(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has no resolved VM offset or explicit native declaration");
    const auto Index = getOrAddNativeSymbol(Native->second);
    Executable.getDynamicImportSymbolRelocations().push_back({Executable.byteSize(), Index, Addend});
    padZero(8);
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
      padRegisterRelativeSymbol(RegisterId::PC, Position->second, Operand.getByteOffset(), InstructionOffset);
      return;
    }

    if (UnresolvedSymbol.contains(*Symbol))
    {
      padUnresolvedSymbol(RegisterId::PC, *Symbol, Operand.getByteOffset(), InstructionOffset);
      return;
    }

    REPORT_ICE_IF(Native == NativeSymbols.end(), Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has no resolved VM offset or explicit native declaration");
    padNativeSymbol(Native->second, Operand.getByteOffset());
  }

  void ExecutableEmitter::padOperand(const BasicBlockOperand &, std::uint64_t)
  {
  }
} // namespace ink::vm
