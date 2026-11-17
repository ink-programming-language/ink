#include "ink/backend/vm/executable_emitter/instruction_emitter.h"

#include "ink/backend/vm/executable_emitter/executable_emission_context.h"
#include "ink/backend/vm/executable_emitter/function_emission_context.h"
#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/instruction/machine_instruction.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"

#include <cstddef>

namespace ink::vm
{
    InstructionEmitter::InstructionEmitter(ExecutableEmissionContext& Context, FunctionEmissionContext& FunctionContext) noexcept
        : Context(Context),
          FunctionContext(FunctionContext)
    {
    }

    void InstructionEmitter::padInstruction(const MachineInstruction& Instruction)
    {
        Context.padAlignment(InstructionEncoding::WordBytes);
        const auto InstructionOffset = Context.byteSize();
        auto Header = InstructionEncoding::encodeOpcode(Instruction.getOpcode());
        for (std::size_t Index = 0; Index < Instruction.operandCount(); ++Index)
        {
            const auto& Operand = *Instruction.getOperand(Index);
            RegisterId Register = RegisterId::ZERO;
            if (Operand.getKind() == OperandKind::Register)
            {
                Register = static_cast<const RegisterOperand&>(Operand).getRegister();
            }
            else if (Operand.getKind() == OperandKind::Address)
            {
                const auto& Address = static_cast<const AddressOperand&>(Operand);
                Register = Address.getRegister().value_or(RegisterId::ZERO);
            }
            else if (Operand.getKind() == OperandKind::BasicBlock)
            {
                Register = RegisterId::PC;
            }
            Header = InstructionEncoding::encodeRegister(Header, Index, Register);
        }
        Context.padInteger(Header, 64);
        for (std::size_t Index = 0; Index < Instruction.operandCount(); ++Index)
        {
            const Operand& Operand = *Instruction.getOperand(Index);
            switch (Operand.getKind())
            {
            case OperandKind::Register:
                padOperand(static_cast<const RegisterOperand&>(Operand));
                break;
            case OperandKind::Address:
                padOperand(static_cast<const AddressOperand&>(Operand), InstructionOffset);
                break;
            case OperandKind::Immediate:
                if (Instruction.getOpcode() == Opcode::Jmpi)
                {
                    padPCRelativeTarget(static_cast<const ImmediateOperand&>(Operand), InstructionOffset);
                }
                else if (Instruction.getOpcode() == Opcode::Calli)
                {
                    padCallTarget(static_cast<const ImmediateOperand&>(Operand));
                }
                else
                {
                    padOperand(static_cast<const ImmediateOperand&>(Operand), Instruction.bitWidth());
                }
                break;
            case OperandKind::BasicBlock:
                padOperand(static_cast<const BasicBlockOperand&>(Operand), InstructionOffset);
                break;
            }
        }
    }

    void InstructionEmitter::padPCRelativeTarget(const ImmediateOperand& Operand, std::uint64_t InstructionOffset)
    {
        if (const auto Bits = Operand.bits())
        {
            Context.padInteger(*Bits, 64);
            return;
        }

        const auto Symbol = Operand.getSymbol();
        REPORT_ICE_IF(!Symbol || !core::NamePool::instance().contains(*Symbol), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "PC-relative target", "VM function target name must be registered and nonempty");
        REPORT_ICE_IF(*Operand.getByteOffset() != 0, Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "PC-relative target", "VM function target must have a zero symbol byte offset");
        if (const auto Position = Context.resolvedSymbolOffset(*Symbol))
        {
            const auto Bits = Context.resolveRegisterRelativeOffset(RegisterId::PC, *Position, 0, InstructionOffset);
            Context.padInteger(Bits, 64);
            return;
        }

        REPORT_ICE_IF(!Context.hasUnResolvedSymbol(*Symbol), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "PC-relative target", "VM function target has no resolved image offset or declaration");
        Context.addStaticRelocation({Context.byteSize(), *Symbol, InstructionOffset, 0, RegisterId::PC});
        Context.padZero(8);
    }

    void InstructionEmitter::padCallTarget(const ImmediateOperand& Operand)
    {
        REPORT_ICE_IF(Operand.getSymbol() && *Operand.getByteOffset() != 0, Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "call", "function target must have a zero symbol byte offset");
        padOperand(Operand, 64);
    }

    void InstructionEmitter::padRegister(RegisterId Register, std::int64_t ByteOffset)
    {
        REPORT_ICE_IF(!isAddressRegister(Register), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "register operand", "register must be BP, SP, PC, ZERO, or R0-R9");
        Context.padInteger(static_cast<std::uint64_t>(ByteOffset), 64);
    }

    void InstructionEmitter::padUnresolvedSymbol(RegisterId Register, core::Name Symbol, std::int64_t Addend, std::uint64_t InstructionOffset)
    {
        Context.addStaticRelocation({Context.byteSize(), Symbol, InstructionOffset, Addend, Register});
        Context.padZero(8);
    }

    void InstructionEmitter::padRegisterRelativeSymbol(RegisterId Register, std::uint64_t SymbolOffset, std::int64_t Addend, std::uint64_t InstructionOffset)
    {
        if (Register == RegisterId::ZERO)
        {
            Context.addImageBaseAddressRelocation({Context.byteSize(), SymbolOffset, Addend});
            Context.padZero(8);
            return;
        }
        const auto Bits = Context.resolveRegisterRelativeOffset(Register, SymbolOffset, Addend, InstructionOffset);
        Context.padInteger(Bits, 64);
    }

    void InstructionEmitter::padImportSymbol(const DynamicImportSymbol& Import, std::int64_t Addend)
    {
        const auto Index = Context.getOrAddImportSymbol(Import);
        Context.addDynamicImportSymbolRelocation({Context.byteSize(), Index, Addend});
        Context.padZero(8);
    }

    void InstructionEmitter::padOperand(const RegisterOperand& Operand)
    {
        Context.padAlignment(InstructionEncoding::WordBytes);
        padRegister(Operand.getRegister(), Operand.getByteOffset());
    }

    void InstructionEmitter::padOperand(const ImmediateOperand& Operand, std::uint32_t Width)
    {
        Context.padAlignment(InstructionEncoding::WordBytes);
        REPORT_ICE_IF(Width != 8 && Width != 16 && Width != 32 && Width != 64, Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "integer encoding requires a supported width");
        if (const auto Bits = Operand.bits())
        {
            const auto Mask = ~std::uint64_t{0} >> (64 - Width);
            Context.padInteger(*Bits & Mask, 64);
            return;
        }

        REPORT_ICE_IF(Width != 64, Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol address requires a 64-bit immediate field");
        const auto Symbol = Operand.getSymbol();
        REPORT_ICE_IF(!Symbol || !core::NamePool::instance().contains(*Symbol), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol name must be registered and nonempty");
        const auto Addend = *Operand.getByteOffset();
        const auto Position = Context.resolvedSymbolOffset(*Symbol);
        const bool Import = Context.isImportSymbol(*Symbol);
        REPORT_ICE_IF(Import && (Position || Context.hasUnResolvedSymbol(*Symbol)), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has both image and import definitions");
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

        REPORT_ICE_IF(!Import, Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has no resolved VM offset or unresolved symbol declaration");
        const auto Index = Context.getOrAddImportSymbol(DynamicImportSymbol{*Symbol});
        Context.addDynamicImportSymbolRelocation({Context.byteSize(), Index, Addend});
        Context.padZero(8);
    }

    void InstructionEmitter::padOperand(const AddressOperand& Operand, std::uint64_t InstructionOffset)
    {
        Context.padAlignment(InstructionEncoding::WordBytes);
        if (const auto Register = Operand.getRegister())
        {
            padRegister(*Register, Operand.getByteOffset());
            return;
        }

        const auto Symbol = Operand.getSymbol();
        REPORT_ICE_IF(!Symbol || !core::NamePool::instance().contains(*Symbol), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol name must be registered and nonempty");
        const auto Position = Context.resolvedSymbolOffset(*Symbol);
        const bool Import = Context.isImportSymbol(*Symbol);
        REPORT_ICE_IF(Import && (Position || Context.hasUnResolvedSymbol(*Symbol)), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has both image and import definitions");
        if (Position)
        {
            padRegisterRelativeSymbol(RegisterId::ZERO, *Position, Operand.getByteOffset(), InstructionOffset);
            return;
        }

        if (Context.hasUnResolvedSymbol(*Symbol))
        {
            padUnresolvedSymbol(RegisterId::ZERO, *Symbol, Operand.getByteOffset(), InstructionOffset);
            return;
        }

        REPORT_ICE_IF(!Import, Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "symbol has no resolved VM offset or unresolved symbol declaration");
        padImportSymbol(DynamicImportSymbol{*Symbol}, Operand.getByteOffset());
    }

    void InstructionEmitter::padOperand(const BasicBlockOperand& Operand, std::uint64_t InstructionOffset)
    {
        Context.padAlignment(InstructionEncoding::WordBytes);
        const auto* Block = Operand.getBlock();
        REPORT_ICE_IF(!Block, Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "basic block target must be nonnull");
        if (const auto Position = FunctionContext.resolvedBasicBlockOffset(*Block))
        {
            padRegisterRelativeSymbol(RegisterId::PC, *Position, 0, InstructionOffset);
            return;
        }

        REPORT_ICE_IF(!FunctionContext.hasUnResolvedBasicBlock(*Block), Context.getDiagnostics(), core::DiagnosticKind::VMExecutableValidationFailed, {}, "pad", "basic block target is not declared in the current function");
        FunctionContext.addBasicBlockStaticRelocation({Context.byteSize(), Block, InstructionOffset});
        Context.padZero(8);
    }
} // namespace ink::vm
