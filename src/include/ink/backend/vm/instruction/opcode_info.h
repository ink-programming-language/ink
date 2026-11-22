#pragma once

#include "ink/backend/vm/instruction/instruction_category.h"
#include "ink/backend/vm/instruction/instruction_domain.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/instruction/instruction_form.h"
#include "ink/backend/vm/instruction/machine_instruction.h"
#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/instruction/operand.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ink::vm
{
    struct OpcodeInfo
    {
            Opcode Operation = Opcode::Ret;
            std::string_view Spelling;
            InstructionForm Form = InstructionForm::None;
            InstructionCategory Category = InstructionCategory::Return;
            InstructionDomain Domain = InstructionDomain::None;
            std::uint32_t BitWidth = 0;
            bool HasDestination = false;
            bool HasBlockTarget = false;
            std::uint8_t OperandCount = 0;
            std::array<OperandKind, InstructionEncoding::MaxOperands> OperandKinds{};

            constexpr std::span<const OperandKind> operandKinds() const noexcept
            {
                return {OperandKinds.data(), OperandCount};
            }

            constexpr bool matchesOperandKinds(std::span<const OperandKind> Kinds) const noexcept
            {
                if (Kinds.size() != OperandCount)
                {
                    return false;
                }
                for (std::size_t Index = 0; Index < OperandCount; ++Index)
                {
                    if (OperandKinds[Index] != Kinds[Index])
                    {
                        return false;
                    }
                }
                return true;
            }
    };

    namespace detail
    {
        enum class OpcodeDenseIndex : std::uint16_t
        {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) Kind,
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
            Count,
        };

        inline constexpr std::size_t OpcodeCount = static_cast<std::size_t>(OpcodeDenseIndex::Count);

        template <typename OperandType>
        consteval OperandKind operandKindForType() noexcept
        {
            if constexpr (std::is_same_v<OperandType, RegisterOperand>)
            {
                return OperandKind::Register;
            }
            else if constexpr (std::is_same_v<OperandType, AddressOperand>)
            {
                return OperandKind::Address;
            }
            else if constexpr (std::is_same_v<OperandType, ImmediateOperand>)
            {
                return OperandKind::Immediate;
            }
            else
            {
                static_assert(std::is_same_v<OperandType, BasicBlockOperand>);
                return OperandKind::BasicBlock;
            }
        }

        template <Opcode Operation, std::size_t... Indices>
        consteval std::array<OperandKind, InstructionEncoding::MaxOperands> buildOpcodeOperandKinds(std::index_sequence<Indices...>) noexcept
        {
            using Tuple = typename OpcodeTraits<Operation>::OperandTuple;
            std::array<OperandKind, InstructionEncoding::MaxOperands> Result{};
            ((Result[Indices] = operandKindForType<std::tuple_element_t<Indices, Tuple>>()), ...);
            return Result;
        }

        template <Opcode Operation>
        consteval OpcodeInfo makeOpcodeInfo(std::string_view Spelling, InstructionForm Form, std::uint32_t Width, InstructionCategory Category, InstructionDomain Domain) noexcept
        {
            using Traits = OpcodeTraits<Operation>;
            static_assert(Traits::OperandCount <= InstructionEncoding::MaxOperands, "VM opcode operand count exceeds InstructionEncoding::MaxOperands");
            static_assert(Traits::BlockCount <= 1, "VM opcode supports at most one basic block target");
            return OpcodeInfo{
                Operation,
                Spelling,
                Form,
                Category,
                Domain,
                Width,
                Traits::HasDestination,
                Traits::BlockCount == 1,
                static_cast<std::uint8_t>(Traits::OperandCount),
                buildOpcodeOperandKinds<Operation>(std::make_index_sequence<Traits::OperandCount>{}),
            };
        }

        struct SortedOpcodeTable
        {
                std::array<OpcodeInfo, OpcodeCount> Entries{};
                std::array<std::uint16_t, OpcodeCount> DenseToSorted{};
        };

        consteval bool opcodeInfoPrecedes(const OpcodeInfo& LHS, const OpcodeInfo& RHS) noexcept
        {
            if (LHS.Spelling != RHS.Spelling)
            {
                return LHS.Spelling < RHS.Spelling;
            }
            return static_cast<std::uint16_t>(LHS.Operation) < static_cast<std::uint16_t>(RHS.Operation);
        }

        consteval SortedOpcodeTable buildSortedOpcodeTable() noexcept
        {
            struct IndexedEntry
            {
                    OpcodeInfo Info{};
                    std::uint16_t DenseIndex = 0;
            };

            std::array<IndexedEntry, OpcodeCount> Raw = {{
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) {makeOpcodeInfo<Opcode::Kind>(Spelling, InstructionForm::Form, Width, InstructionCategory::Category, InstructionDomain::Domain), static_cast<std::uint16_t>(OpcodeDenseIndex::Kind)},
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
            }};

            for (std::size_t Index = 1; Index < Raw.size(); ++Index)
            {
                IndexedEntry Current = Raw[Index];
                std::size_t Position = Index;
                while (Position > 0 && opcodeInfoPrecedes(Current.Info, Raw[Position - 1].Info))
                {
                    Raw[Position] = Raw[Position - 1];
                    --Position;
                }
                Raw[Position] = Current;
            }

            SortedOpcodeTable Table{};
            for (std::size_t SortedIndex = 0; SortedIndex < Raw.size(); ++SortedIndex)
            {
                Table.Entries[SortedIndex] = Raw[SortedIndex].Info;
                Table.DenseToSorted[Raw[SortedIndex].DenseIndex] = static_cast<std::uint16_t>(SortedIndex);
            }
            return Table;
        }

        inline constexpr SortedOpcodeTable OpcodeTable = buildSortedOpcodeTable();

        consteval bool verifyOpcodeInfoTable() noexcept
        {
            const auto& Entries = OpcodeTable.Entries;
            for (std::size_t Index = 0; Index < Entries.size(); ++Index)
            {
                const OpcodeInfo& Entry = Entries[Index];
                if (Entry.OperandCount > InstructionEncoding::MaxOperands)
                {
                    return false;
                }
                std::size_t BlockOperandCount = 0;
                for (std::size_t OperandIndex = 0; OperandIndex < Entry.OperandCount; ++OperandIndex)
                {
                    if (Entry.OperandKinds[OperandIndex] == OperandKind::BasicBlock)
                    {
                        ++BlockOperandCount;
                    }
                    if (Entry.Domain == InstructionDomain::FloatingPoint && Entry.OperandKinds[OperandIndex] != OperandKind::Register)
                    {
                        return false;
                    }
                }
                if (Entry.HasBlockTarget)
                {
                    if (BlockOperandCount != 1 || Entry.OperandCount == 0 || Entry.OperandKinds[Entry.OperandCount - 1] != OperandKind::BasicBlock)
                    {
                        return false;
                    }
                }
                else if (BlockOperandCount != 0)
                {
                    return false;
                }
            }

            std::size_t GroupBegin = 0;
            while (GroupBegin < Entries.size())
            {
                std::size_t GroupEnd = GroupBegin + 1;
                while (GroupEnd < Entries.size() && Entries[GroupEnd].Spelling == Entries[GroupBegin].Spelling)
                {
                    ++GroupEnd;
                }
                for (std::size_t First = GroupBegin; First < GroupEnd; ++First)
                {
                    const OpcodeInfo& LHS = Entries[First];
                    for (std::size_t Second = First + 1; Second < GroupEnd; ++Second)
                    {
                        const OpcodeInfo& RHS = Entries[Second];
                        if (LHS.HasBlockTarget != RHS.HasBlockTarget)
                        {
                            return false;
                        }
                        if (LHS.HasBlockTarget && LHS.OperandCount != RHS.OperandCount)
                        {
                            return false;
                        }
                        if (LHS.matchesOperandKinds(RHS.operandKinds()))
                        {
                            return false;
                        }
                    }
                }
                GroupBegin = GroupEnd;
            }
            return true;
        }

        static_assert(verifyOpcodeInfoTable(), "VM opcode table must satisfy operand, floating-point, block-target, and overload invariants");
    } // namespace detail

    constexpr std::span<const OpcodeInfo> sortedOpcodeInfos() noexcept
    {
        return detail::OpcodeTable.Entries;
    }

    constexpr const OpcodeInfo* findOpcodeInfo(Opcode Operation) noexcept
    {
        switch (Operation)
        {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) \
    case Opcode::Kind:                                                                                               \
        return &detail::OpcodeTable.Entries[detail::OpcodeTable.DenseToSorted[static_cast<std::size_t>(detail::OpcodeDenseIndex::Kind)]];
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
        }
        return nullptr;
    }

    constexpr std::span<const OpcodeInfo> findOpcodeOverloads(std::string_view Spelling) noexcept
    {
        const auto Entries = sortedOpcodeInfos();
        std::size_t Low = 0;
        std::size_t High = Entries.size();
        while (Low < High)
        {
            const std::size_t Mid = Low + (High - Low) / 2;
            if (Entries[Mid].Spelling < Spelling)
            {
                Low = Mid + 1;
            }
            else
            {
                High = Mid;
            }
        }
        std::size_t End = Low;
        while (End < Entries.size() && Entries[End].Spelling == Spelling)
        {
            ++End;
        }
        return Entries.subspan(Low, End - Low);
    }

    constexpr const OpcodeInfo* findOpcodeInfo(std::string_view Spelling, std::span<const OperandKind> Kinds) noexcept
    {
        for (const OpcodeInfo& Entry : findOpcodeOverloads(Spelling))
        {
            if (Entry.matchesOperandKinds(Kinds))
            {
                return &Entry;
            }
        }
        return nullptr;
    }
} // namespace ink::vm
