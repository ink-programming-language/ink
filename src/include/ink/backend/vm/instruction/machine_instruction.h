#pragma once

#include "ink/backend/vm/instruction/operand.h"
#include "ink/backend/vm/instruction/instruction_form.h"
#include "ink/backend/vm/instruction/instruction_category.h"
#include "ink/backend/vm/instruction/instruction_domain.h"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>

namespace ink::vm
{
    class MachineBasicBlock;
    class MachineInstructionBuilder;

    enum class Opcode : std::uint16_t
    {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) Kind = Encoding,
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
    };

    template <Opcode OperationValue>
    class MachineInstructionImpl;

    class MachineInstruction
    {
        public:
            constexpr Opcode getOpcode() const noexcept
            {
                return Operation;
            }

            constexpr std::string_view getName() const noexcept
            {
                switch (Operation)
                {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) \
    case Opcode::Kind:                                                                                               \
        return Spelling;
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
                }
                return "InvalidOpcode";
            }

            constexpr std::uint32_t bitWidth() const noexcept
            {
                switch (Operation)
                {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) \
    case Opcode::Kind:                                                                                               \
        return Width;
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
                }
                return 0;
            }

            constexpr std::size_t operandCount() const noexcept
            {
                switch (Operation)
                {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) \
    case Opcode::Kind:                                                                                               \
        return FormTraits<InstructionForm::Form>::OperandCount;
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
                }
                return 0;
            }

            constexpr InstructionCategory category() const noexcept
            {
                switch (Operation)
                {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) \
    case Opcode::Kind:                                                                                               \
        return InstructionCategory::Category;
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
                }
                return InstructionCategory::Load;
            }

            constexpr InstructionDomain domain() const noexcept
            {
                switch (Operation)
                {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) \
    case Opcode::Kind:                                                                                               \
        return InstructionDomain::Domain;
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
                }
                return InstructionDomain::None;
            }

            constexpr bool hasDestination() const noexcept
            {
                const auto Kind = category();
                return Kind == InstructionCategory::Load || Kind == InstructionCategory::Store || Kind == InstructionCategory::Move || Kind == InstructionCategory::Arithmetic || Kind == InstructionCategory::Bitwise || Kind == InstructionCategory::Compare || Kind == InstructionCategory::Convert;
            }

            constexpr bool isFloatingPoint() const noexcept
            {
                return domain() == InstructionDomain::FloatingPoint;
            }

            constexpr bool isBranch() const noexcept
            {
                return category() == InstructionCategory::Branch || isConditionalBranch();
            }

            constexpr bool isConditionalBranch() const noexcept
            {
                return category() == InstructionCategory::ConditionalBranch;
            }

            constexpr bool isReturn() const noexcept
            {
                return category() == InstructionCategory::Return;
            }

            constexpr bool isCall() const noexcept
            {
                return category() == InstructionCategory::Call;
            }

            constexpr bool isTerminator() const noexcept
            {
                return isBranch() || isReturn();
            }

            // Borrows an operand in semantic order; an index outside operandCount() returns nullptr.
            const Operand* getOperand(std::size_t Index) const noexcept;

            constexpr auto* parent(this auto&& Self) noexcept
            {
                return Self.Parent ? &std::forward_like<decltype(Self)&>(*Self.Parent) : nullptr;
            }

            MachineInstruction(const MachineInstruction&) = delete;
            MachineInstruction& operator=(const MachineInstruction&) = delete;

        protected:
            // Instructions are destroyed through their concrete class, never through MachineInstruction.
            ~MachineInstruction() = default;

        private:
            friend class MachineInstructionBuilder;

            template <Opcode OperationValue>
            friend class MachineInstructionImpl;

            // Only the final opcode-specific implementation may establish an opcode and its matching layout.
            constexpr explicit MachineInstruction(Opcode Operation) noexcept
                : Operation(Operation)
            {
            }

            Opcode Operation;
            MachineBasicBlock* Parent = nullptr;
    };
} // namespace ink::vm
