#include "ink/backend/vm/executable_emitter/instruction_emitter.h"

#include "ink/backend/vm/executable_emitter/executable_emission_context.h"
#include "ink/backend/vm/executable_emitter/function_emission_context.h"
#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/machine_instruction.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"

#include <cstddef>

namespace ink::vm
{
  InstructionEmitter::InstructionEmitter(ExecutableEmissionContext &Context, FunctionEmissionContext &FunctionContext) noexcept
      : Context(Context),
        FunctionContext(FunctionContext)
  {
  }

  void InstructionEmitter::padInstruction(const MachineInstruction &Instruction)
  {
    const auto InstructionOffset = Context.byteSize();
    Context.padInteger(static_cast<std::uint16_t>(Instruction.getOpcode()), 16);
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

  void InstructionEmitter::padRegister(RegisterId Register, std::int64_t ByteOffset)
  {
    REPORT_ICE_IF(!isAddressRegister(Register), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "register operand", "register must be BP, SP, PC, or ZERO");
    Context.padInteger(static_cast<std::uint8_t>(Register), 8);
    Context.padInteger(static_cast<std::uint64_t>(ByteOffset), 64);
  }

  void InstructionEmitter::padUnresolvedSymbol(RegisterId Register, core::Name Symbol, std::int64_t Addend, std::uint64_t InstructionOffset)
  {
    Context.padInteger(static_cast<std::uint8_t>(Register), 8);
    Context.addStaticRelocation({Context.byteSize(), Symbol, InstructionOffset, Addend, Register});
    Context.padZero(8);
  }

  void InstructionEmitter::padRegisterRelativeSymbol(RegisterId Register, std::uint64_t SymbolOffset, std::int64_t Addend, std::uint64_t InstructionOffset)
  {
    if (Register == RegisterId::ZERO)
    {
      Context.padInteger(static_cast<std::uint8_t>(Register), 8);
      Context.addImageBaseAddressRelocation({Context.byteSize(), SymbolOffset, Addend});
      Context.padZero(8);
      return;
    }
    const auto Bits = Context.resolveRegisterRelativeOffset(Register, SymbolOffset, Addend, InstructionOffset);
    Context.padInteger(static_cast<std::uint8_t>(Register), 8);
    Context.padInteger(Bits, 64);
  }

  void InstructionEmitter::padNativeSymbol(const DynamicImportSymbol &Import, std::int64_t Addend)
  {
    const auto Index = Context.getOrAddNativeSymbol(Import);
    Context.padInteger(static_cast<std::uint8_t>(RegisterId::ZERO), 8);
    Context.addDynamicImportSymbolRelocation({Context.byteSize(), Index, Addend});
    Context.padZero(8);
  }

  void InstructionEmitter::padOperand(const RegisterOperand &Operand)
  {
    padRegister(Operand.getRegister(), Operand.getByteOffset());
  }

  void InstructionEmitter::padOperand(const ImmediateOperand &Operand, std::uint32_t Width)
  {
    REPORT_ICE_IF(Width != 8 && Width != 16 && Width != 32 && Width != 64, Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "integer encoding requires a supported width");
    if (const auto Bits = Operand.bits())
    {
      Context.padInteger(*Bits, Width);
      return;
    }

    REPORT_ICE_IF(Width != 64, Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol address requires a 64-bit immediate field");
    const auto Symbol = Operand.getSymbol();
    REPORT_ICE_IF(!Symbol || !core::NamePool::instance().contains(*Symbol), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol name must be registered and nonempty");
    const auto Addend = *Operand.getByteOffset();
    const auto Position = Context.resolvedSymbolOffset(*Symbol);
    const auto *Native = Context.findNativeSymbol(*Symbol);
    REPORT_ICE_IF(Native && (Position || Context.hasUnResolvedSymbol(*Symbol)), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has both VM and native definitions");
    if (Position)
    {
      Context.addImageBaseAddressRelocation({Context.byteSize(), *Position, Addend});
      Context.padZero(8);
      return;
    }

    if (Context.hasUnResolvedSymbol(*Symbol))
    {
      Context.addStaticRelocation({Context.byteSize(), *Symbol, 0, Addend, RegisterId::ZERO});
      Context.padZero(8);
      return;
    }

    REPORT_ICE_IF(!Native, Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has no resolved VM offset or explicit native declaration");
    const auto Index = Context.getOrAddNativeSymbol(*Native);
    Context.addDynamicImportSymbolRelocation({Context.byteSize(), Index, Addend});
    Context.padZero(8);
  }

  void InstructionEmitter::padOperand(const AddressOperand &Operand, std::uint64_t InstructionOffset)
  {
    if (const auto Register = Operand.getRegister())
    {
      padRegister(*Register, Operand.getByteOffset());
      return;
    }

    const auto Symbol = Operand.getSymbol();
    REPORT_ICE_IF(!Symbol || !core::NamePool::instance().contains(*Symbol), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol name must be registered and nonempty");
    const auto Position = Context.resolvedSymbolOffset(*Symbol);
    const auto *Native = Context.findNativeSymbol(*Symbol);
    REPORT_ICE_IF(Native && (Position || Context.hasUnResolvedSymbol(*Symbol)), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has both VM and native definitions");
    if (Position)
    {
      padRegisterRelativeSymbol(RegisterId::PC, *Position, Operand.getByteOffset(), InstructionOffset);
      return;
    }

    if (Context.hasUnResolvedSymbol(*Symbol))
    {
      padUnresolvedSymbol(RegisterId::PC, *Symbol, Operand.getByteOffset(), InstructionOffset);
      return;
    }

    REPORT_ICE_IF(!Native, Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has no resolved VM offset or explicit native declaration");
    padNativeSymbol(*Native, Operand.getByteOffset());
  }

  void InstructionEmitter::padOperand(const BasicBlockOperand &Operand, std::uint64_t InstructionOffset)
  {
    const auto *Block = Operand.getBlock();
    REPORT_ICE_IF(!Block, Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "basic block target must be nonnull");
    if (const auto Position = FunctionContext.resolvedBasicBlockOffset(*Block))
    {
      padRegisterRelativeSymbol(RegisterId::PC, *Position, 0, InstructionOffset);
      return;
    }

    REPORT_ICE_IF(!FunctionContext.hasUnResolvedBasicBlock(*Block), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "basic block target is not declared in the current function");
    Context.padInteger(static_cast<std::uint8_t>(RegisterId::PC), 8);
    FunctionContext.addBasicBlockStaticRelocation({Context.byteSize(), Block, InstructionOffset});
    Context.padZero(8);
  }
} // namespace ink::vm
