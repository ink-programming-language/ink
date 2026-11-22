#pragma once

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/operand.h"
#include "ink/backend/vm/instruction/register_operand.h"

#include <variant>

namespace ink::vm
{
    // Tagged value wrapper around the concrete VM operand types for runtime validation and dynamic construction.
    class MachineOperand final
    {
        public:
            constexpr MachineOperand() noexcept
                : Value(ImmediateOperand(0))
            {
            }

            constexpr MachineOperand(RegisterOperand OperandValue) noexcept
                : Value(OperandValue)
            {
            }

            constexpr MachineOperand(AddressOperand OperandValue) noexcept
                : Value(OperandValue)
            {
            }

            constexpr MachineOperand(ImmediateOperand OperandValue) noexcept
                : Value(OperandValue)
            {
            }

            constexpr MachineOperand(BasicBlockOperand OperandValue) noexcept
                : Value(OperandValue)
            {
            }

            static constexpr MachineOperand fromOperand(const Operand& OperandValue) noexcept
            {
                switch (OperandValue.getKind())
                {
                case OperandKind::Register:
                    return MachineOperand(static_cast<const RegisterOperand&>(OperandValue));
                case OperandKind::Address:
                    return MachineOperand(static_cast<const AddressOperand&>(OperandValue));
                case OperandKind::Immediate:
                    return MachineOperand(static_cast<const ImmediateOperand&>(OperandValue));
                case OperandKind::BasicBlock:
                    return MachineOperand(static_cast<const BasicBlockOperand&>(OperandValue));
                }
                return MachineOperand();
            }

            constexpr OperandKind getKind() const noexcept
            {
                return getOperand().getKind();
            }

            constexpr const Operand& getOperand() const noexcept
            {
                if (const auto* Register = std::get_if<RegisterOperand>(&Value))
                {
                    return *Register;
                }
                if (const auto* Address = std::get_if<AddressOperand>(&Value))
                {
                    return *Address;
                }
                if (const auto* Immediate = std::get_if<ImmediateOperand>(&Value))
                {
                    return *Immediate;
                }
                return *std::get_if<BasicBlockOperand>(&Value);
            }

            constexpr const RegisterOperand* asRegister() const noexcept
            {
                return std::get_if<RegisterOperand>(&Value);
            }

            constexpr const AddressOperand* asAddress() const noexcept
            {
                return std::get_if<AddressOperand>(&Value);
            }

            constexpr const ImmediateOperand* asImmediate() const noexcept
            {
                return std::get_if<ImmediateOperand>(&Value);
            }

            constexpr const BasicBlockOperand* asBasicBlock() const noexcept
            {
                return std::get_if<BasicBlockOperand>(&Value);
            }

        private:
            std::variant<RegisterOperand, AddressOperand, ImmediateOperand, BasicBlockOperand> Value;
    };
} // namespace ink::vm
