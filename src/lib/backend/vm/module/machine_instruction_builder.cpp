#include "ink/backend/vm/module/machine_instruction_builder.h"

#include "ink/core/support/core_define.h"

#include <cstddef>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ink::vm
{
    namespace
    {
        template <typename OperandType>
        const OperandType* extractOperand(const MachineOperand& Value) noexcept
        {
            if constexpr (std::is_same_v<OperandType, RegisterOperand>)
            {
                return Value.asRegister();
            }
            else if constexpr (std::is_same_v<OperandType, AddressOperand>)
            {
                return Value.asAddress();
            }
            else if constexpr (std::is_same_v<OperandType, ImmediateOperand>)
            {
                return Value.asImmediate();
            }
            else
            {
                static_assert(std::is_same_v<OperandType, BasicBlockOperand>);
                return Value.asBasicBlock();
            }
        }

        template <Opcode Operation, std::size_t... Indices>
        MachineInstruction* tryCreateWithOperands(MachineInstructionBuilder& Builder, std::span<const MachineOperand> Operands, std::index_sequence<Indices...>)
        {
            using Tuple = typename OpcodeTraits<Operation>::OperandTuple;
            if (Operands.size() != sizeof...(Indices))
            {
                return nullptr;
            }
            const std::tuple<const std::tuple_element_t<Indices, Tuple>*...> Extracted{extractOperand<std::tuple_element_t<Indices, Tuple>>(Operands[Indices])...};
            if (!((std::get<Indices>(Extracted) != nullptr) && ...))
            {
                return nullptr;
            }
            return Builder.tryCreate<Operation>(*std::get<Indices>(Extracted)...);
        }
    } // namespace

    MachineInstruction* MachineInstructionBuilder::tryCreate(Opcode Operation, std::span<const MachineOperand> Operands)
    {
        switch (Operation)
        {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) \
    case Opcode::Kind:                                                                                               \
        return tryCreateWithOperands<Opcode::Kind>(*this, Operands, std::make_index_sequence<OpcodeTraits<Opcode::Kind>::OperandCount>{});
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
        }
        return nullptr;
    }

    void MachineInstructionBuilder::invalidBranchTarget() noexcept
    {
        panic("VM branch target must belong to the insertion block's function");
    }

    void MachineInstructionBuilder::setInsertPoint(MachineBasicBlock& Block) noexcept
    {
        InsertBlock = &Block;
        InsertPosition = Block.Instructions.end();
    }

    bool MachineInstructionBuilder::setInsertPoint(MachineInstruction& Before) noexcept
    {
        MachineBasicBlock* Block = Before.parent();
        if (!Block)
        {
            return false;
        }
        for (auto Position = Block->Instructions.begin(); Position != Block->Instructions.end(); ++Position)
        {
            if (Position->get() == &Before)
            {
                InsertBlock = Block;
                InsertPosition = Position;
                return true;
            }
        }
        return false;
    }

    void MachineInstructionBuilder::clearInsertPoint() noexcept
    {
        InsertBlock = nullptr;
        InsertPosition = {};
    }

    MachineBasicBlock* MachineInstructionBuilder::getInsertBlock() const noexcept
    {
        return InsertBlock;
    }

    MachineInstruction* MachineInstructionBuilder::getInsertBefore() const noexcept
    {
        if (!InsertBlock || InsertPosition == InsertBlock->Instructions.end())
        {
            return nullptr;
        }
        return InsertPosition->get();
    }

    void MachineInstructionBuilder::checkInsertPoint() const noexcept
    {
        if (!InsertBlock)
        {
            panic("VM MachineInstructionBuilder requires an insertion point");
        }
    }
} // namespace ink::vm
