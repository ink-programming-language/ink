#pragma once

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/machine_instruction.h"
#include "ink/backend/vm/instruction/register.h"
#include "ink/backend/vm/instruction/register_operand.h"

#include <cstddef>
#include <optional>
#include <string_view>

namespace ink::vm
{
    constexpr bool isValidFloatingPointOperand(std::size_t OperandIndex, const RegisterOperand& Operand) noexcept
    {
        const bool ValidRegister = OperandIndex == 0 ? isGeneralPurposeRegister(Operand.getRegister()) : isFloatingPointSourceRegister(Operand.getRegister());
        return ValidRegister && Operand.getByteOffset() == 0;
    }

    constexpr bool isValidRegisterDestination(const RegisterOperand& Destination) noexcept
    {
        return isWritableAddressRegister(Destination.getRegister()) && Destination.getByteOffset() == 0;
    }

    constexpr bool isValidDirectFunctionTarget(const ImmediateOperand& Target) noexcept
    {
        return !Target.getSymbol() || *Target.getByteOffset() == 0;
    }

    inline std::optional<std::string_view> validateFloatingPointOperands(const MachineInstruction& Instruction) noexcept
    {
        if (!Instruction.isFloatingPoint())
        {
            return std::nullopt;
        }
        for (std::size_t Index = 0; Index < Instruction.operandCount(); ++Index)
        {
            const auto& Operand = static_cast<const RegisterOperand&>(*Instruction.getOperand(Index));
            if (!isValidFloatingPointOperand(Index, Operand))
            {
                return "floating-point operands require a GPR destination and GPR or ZERO sources, all with zero byte offset";
            }
        }
        return std::nullopt;
    }

    inline std::optional<std::string_view> validateRegisterDestination(const MachineInstruction& Instruction) noexcept
    {
        if (Instruction.hasDestination() && Instruction.getOperand(0)->getKind() == OperandKind::Register)
        {
            const auto& Destination = static_cast<const RegisterOperand&>(*Instruction.getOperand(0));
            if (!isValidRegisterDestination(Destination))
            {
                return "register destination must be BP, SP, or R0-R9 with zero byte offset";
            }
        }
        return std::nullopt;
    }

    inline std::optional<std::string_view> validateDirectControlTarget(const MachineInstruction& Instruction) noexcept
    {
        if ((Instruction.isCall() || Instruction.getOpcode() == Opcode::Jmpi) && Instruction.getOperand(0)->getKind() == OperandKind::Immediate)
        {
            const auto& Target = static_cast<const ImmediateOperand&>(*Instruction.getOperand(0));
            if (!isValidDirectFunctionTarget(Target))
            {
                return "direct function symbol target must have a zero byte offset";
            }
        }
        return std::nullopt;
    }

    inline std::optional<std::string_view> validateInstructionConstraints(const MachineInstruction& Instruction) noexcept
    {
        if (const auto Error = validateFloatingPointOperands(Instruction))
        {
            return Error;
        }
        if (const auto Error = validateRegisterDestination(Instruction))
        {
            return Error;
        }
        return validateDirectControlTarget(Instruction);
    }
} // namespace ink::vm
