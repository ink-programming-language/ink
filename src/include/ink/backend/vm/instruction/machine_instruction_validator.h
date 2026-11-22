#pragma once

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/machine_instruction.h"
#include "ink/backend/vm/instruction/machine_operand.h"
#include "ink/backend/vm/instruction/opcode_info.h"
#include "ink/backend/vm/instruction/register.h"
#include "ink/backend/vm/instruction/register_operand.h"

#include <cstddef>
#include <optional>
#include <span>
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

    namespace detail
    {
        template <typename OperandGetter>
        constexpr std::optional<std::string_view> validateFloatingPointOperandsImpl(const OpcodeInfo& Info, OperandGetter&& GetOperand) noexcept
        {
            if (Info.Domain != InstructionDomain::FloatingPoint)
            {
                return std::nullopt;
            }
            for (std::size_t Index = 0; Index < Info.OperandCount; ++Index)
            {
                const Operand* Value = GetOperand(Index);
                if (!Value || Value->getKind() != OperandKind::Register || !isValidFloatingPointOperand(Index, static_cast<const RegisterOperand&>(*Value)))
                {
                    return "floating-point operands require a GPR destination and GPR or ZERO sources, all with zero byte offset";
                }
            }
            return std::nullopt;
        }

        template <typename OperandGetter>
        constexpr std::optional<std::string_view> validateRegisterDestinationImpl(const OpcodeInfo& Info, OperandGetter&& GetOperand) noexcept
        {
            if (Info.HasDestination && Info.OperandCount != 0)
            {
                const Operand* Value = GetOperand(0);
                if (Value && Value->getKind() == OperandKind::Register && !isValidRegisterDestination(static_cast<const RegisterOperand&>(*Value)))
                {
                    return "register destination must be BP, SP, or R0-R9 with zero byte offset";
                }
            }
            return std::nullopt;
        }

        template <typename OperandGetter>
        constexpr std::optional<std::string_view> validateDirectControlTargetImpl(const OpcodeInfo& Info, OperandGetter&& GetOperand) noexcept
        {
            if ((Info.Category == InstructionCategory::Call || Info.Operation == Opcode::Jmpi) && Info.OperandCount != 0)
            {
                const Operand* Value = GetOperand(0);
                if (Value && Value->getKind() == OperandKind::Immediate && !isValidDirectFunctionTarget(static_cast<const ImmediateOperand&>(*Value)))
                {
                    return "direct function symbol target must have a zero byte offset";
                }
            }
            return std::nullopt;
        }

        template <typename OperandGetter>
        constexpr std::optional<std::string_view> validateSymbolicImmediateWidthImpl(const OpcodeInfo& Info, OperandGetter&& GetOperand) noexcept
        {
            if (Info.BitWidth == 64)
            {
                return std::nullopt;
            }
            for (std::size_t Index = 0; Index < Info.OperandCount; ++Index)
            {
                const Operand* Value = GetOperand(Index);
                if (Value && Value->getKind() == OperandKind::Immediate && static_cast<const ImmediateOperand&>(*Value).getSymbol())
                {
                    return "symbol address immediate requires a 64-bit instruction";
                }
            }
            return std::nullopt;
        }

        template <typename OperandGetter>
        constexpr std::optional<std::string_view> validateInstructionConstraintsImpl(const OpcodeInfo& Info, OperandGetter&& GetOperand) noexcept
        {
            if (const auto Error = validateFloatingPointOperandsImpl(Info, GetOperand))
            {
                return Error;
            }
            if (const auto Error = validateRegisterDestinationImpl(Info, GetOperand))
            {
                return Error;
            }
            if (const auto Error = validateDirectControlTargetImpl(Info, GetOperand))
            {
                return Error;
            }
            return validateSymbolicImmediateWidthImpl(Info, GetOperand);
        }
    } // namespace detail

    inline std::optional<std::string_view> validateFloatingPointOperands(const MachineInstruction& Instruction) noexcept
    {
        const OpcodeInfo* Info = findOpcodeInfo(Instruction.getOpcode());
        if (!Info)
        {
            return "instruction has an unknown opcode";
        }
        return detail::validateFloatingPointOperandsImpl(*Info, [&](std::size_t Index) noexcept
                                                         {
                                                             return Instruction.getOperand(Index);
                                                         });
    }

    inline std::optional<std::string_view> validateRegisterDestination(const MachineInstruction& Instruction) noexcept
    {
        const OpcodeInfo* Info = findOpcodeInfo(Instruction.getOpcode());
        if (!Info)
        {
            return "instruction has an unknown opcode";
        }
        return detail::validateRegisterDestinationImpl(*Info, [&](std::size_t Index) noexcept
                                                       {
                                                           return Instruction.getOperand(Index);
                                                       });
    }

    inline std::optional<std::string_view> validateDirectControlTarget(const MachineInstruction& Instruction) noexcept
    {
        const OpcodeInfo* Info = findOpcodeInfo(Instruction.getOpcode());
        if (!Info)
        {
            return "instruction has an unknown opcode";
        }
        return detail::validateDirectControlTargetImpl(*Info, [&](std::size_t Index) noexcept
                                                       {
                                                           return Instruction.getOperand(Index);
                                                       });
    }

    inline std::optional<std::string_view> validateSymbolicImmediateWidth(const MachineInstruction& Instruction) noexcept
    {
        const OpcodeInfo* Info = findOpcodeInfo(Instruction.getOpcode());
        if (!Info)
        {
            return "instruction has an unknown opcode";
        }
        return detail::validateSymbolicImmediateWidthImpl(*Info, [&](std::size_t Index) noexcept
                                                          {
                                                              return Instruction.getOperand(Index);
                                                          });
    }

    inline std::optional<std::string_view> validateInstructionConstraints(const OpcodeInfo& Info, std::span<const MachineOperand> Operands) noexcept
    {
        if (Operands.size() != Info.OperandCount)
        {
            return "operand count does not match instruction opcode";
        }
        for (std::size_t Index = 0; Index < Info.OperandCount; ++Index)
        {
            if (Operands[Index].getKind() != Info.OperandKinds[Index])
            {
                return "operand kinds do not match instruction opcode";
            }
        }
        return detail::validateInstructionConstraintsImpl(Info, [&](std::size_t Index) noexcept
                                                          {
                                                              return &Operands[Index].getOperand();
                                                          });
    }

    inline std::optional<std::string_view> validateInstructionConstraints(const MachineInstruction& Instruction) noexcept
    {
        const OpcodeInfo* Info = findOpcodeInfo(Instruction.getOpcode());
        if (!Info)
        {
            return "instruction has an unknown opcode";
        }
        return detail::validateInstructionConstraintsImpl(*Info, [&](std::size_t Index) noexcept
                                                          {
                                                              return Instruction.getOperand(Index);
                                                          });
    }
} // namespace ink::vm
