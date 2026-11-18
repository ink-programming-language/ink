#pragma once

#include "ink/backend/vm/executable/executable_file_type.h"
#include "ink/backend/vm/instruction/register.h"
#include "ink/core/name/name.h"

#include <cstdint>

namespace ink::vm
{
    class ExecutableEmissionContext;
    class FunctionEmissionContext;
    class MachineInstruction;
    class AddressOperand;
    class RegisterOperand;
    class ImmediateOperand;
    class BasicBlockOperand;

    class InstructionEmitter
    {
        public:
            // Both borrowed contexts must outlive this emitter.
            InstructionEmitter(ExecutableEmissionContext& Context, FunctionEmissionContext& FunctionContext) noexcept;

            // Appends an aligned header word, then one payload word per operand in semantic order.
            // Direct target kinds and zero symbol addends must be validated before emission; calls encode full address fields.
            void padInstruction(const MachineInstruction& Instruction);

            // Appends the register's signed byte offset as one word; its identifier belongs to the instruction header.
            void padOperand(const RegisterOperand& Operand);

            // Appends a zero-extended, width-truncated numeric word or a symbol address placeholder with a loading relocation.
            void padOperand(const ImmediateOperand& Operand, std::uint32_t Width);

            // Appends a register displacement or a symbol address placeholder with a loading relocation.
            void padOperand(const AddressOperand& Operand, std::uint64_t InstructionOffset);

            // Appends a PC-relative basic-block target; unresolved blocks reserve a zero displacement until function finalization.
            void padOperand(const BasicBlockOperand& Operand, std::uint64_t InstructionOffset);

        private:
            void padPCRelativeTarget(const ImmediateOperand& Operand, std::uint64_t InstructionOffset);
            void padCallTarget(const ImmediateOperand& Operand);
            void padImmediateOperand(const ImmediateOperand& Operand, std::uint32_t Width, RelocationKind Kind);
            void padRegister(RegisterId Register, std::int64_t ByteOffset);
            void padUnresolvedSymbol(RegisterId Register, core::Name Symbol, std::int64_t Addend, std::uint64_t InstructionOffset);
            void padRegisterRelativeSymbol(RegisterId Register, std::uint64_t SymbolOffset, std::int64_t Addend, std::uint64_t InstructionOffset);
            void padImportSymbol(const DynamicImportSymbol& Import, std::int64_t Addend);

            ExecutableEmissionContext& Context;
            FunctionEmissionContext& FunctionContext;
    };
} // namespace ink::vm
