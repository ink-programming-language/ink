#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/interpreter/interpreter.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <utility>

namespace ink::vm::test
{
    namespace
    {
        std::uint64_t addressOf(const void* Address)
        {
            return static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Address));
        }

        void appendWord(ExecutableFile& Executable, std::uint64_t Value)
        {
            for (std::size_t Index = 0; Index < sizeof(Value); ++Index)
            {
                Executable.getImage().push_back(static_cast<std::uint8_t>(Value >> (Index * 8)));
            }
        }

        std::size_t appendInstruction(ExecutableFile& Executable, Opcode Operation, std::initializer_list<std::uint64_t> Operands = {}, RegisterId First = RegisterId::ZERO, RegisterId Second = RegisterId::ZERO, RegisterId Third = RegisterId::ZERO)
        {
            const auto Offset = Executable.byteSize();
            auto Header = InstructionEncoding::encodeOpcode(Operation);
            Header = InstructionEncoding::encodeRegister(Header, 0, First);
            Header = InstructionEncoding::encodeRegister(Header, 1, Second);
            Header = InstructionEncoding::encodeRegister(Header, 2, Third);
            appendWord(Executable, Header);
            for (const auto Operand : Operands)
            {
                appendWord(Executable, Operand);
            }
            return Offset;
        }

        std::uint64_t negativeOffset(std::size_t Offset)
        {
            return std::uint64_t{0} - static_cast<std::uint64_t>(Offset);
        }

        std::uint64_t readWord(const void* Address)
        {
            std::uint64_t Value;
            std::memcpy(&Value, Address, sizeof(Value));
            return Value;
        }
    } // namespace

    // Every general-purpose register uses the existing immediate-add encoding and is reset before the next execution.
    TEST(VMArithmeticTest, WritesAllGeneralPurposeRegistersAndResetsBetweenExecutions)
    {
        Interpreter VM;
        ExecutableFile Executable;
        for (std::size_t Index = 0; Index < 10; ++Index)
        {
            const auto Register = static_cast<RegisterId>(static_cast<std::uint8_t>(RegisterId::R0) + Index);
            appendInstruction(Executable, Opcode::Add64rri, {0, 0, Index + 1}, Register, RegisterId::ZERO);
        }
        appendInstruction(Executable, Opcode::Ret);
        const auto ResetEntry = appendInstruction(Executable, Opcode::Ret);
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto Bindings = VM.getFrame().Register;

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));

        for (std::size_t Index = 0; Index < 10; ++Index)
        {
            const auto Register = static_cast<RegisterId>(static_cast<std::uint8_t>(RegisterId::R0) + Index);
            EXPECT_EQ(READ_REGISTER(Register, VM.getFrame()), Index + 1);
        }
        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + ResetEntry));
        for (std::size_t Index = 0; Index < 10; ++Index)
        {
            const auto Register = static_cast<RegisterId>(static_cast<std::uint8_t>(RegisterId::R0) + Index);
            EXPECT_EQ(READ_REGISTER(Register, VM.getFrame()), 0U);
        }
        EXPECT_EQ(VM.getFrame().Register, Bindings);
    }

    // General-purpose values wrap, support aliased destinations and source offsets, and serve as unaligned memory bases.
    TEST(VMArithmeticTest, UsesGeneralPurposeRegistersForValuesAndMemoryBases)
    {
        std::array<std::uint8_t, 40> Memory{};
        constexpr std::uint64_t Input = 11;
        std::memcpy(Memory.data() + 1, &Input, sizeof(Input));
        Interpreter VM;
        ExecutableFile Executable;
        appendInstruction(Executable, Opcode::Add64rri, {0, 0, std::numeric_limits<std::uint64_t>::max()}, RegisterId::R0, RegisterId::ZERO);
        appendInstruction(Executable, Opcode::Add64rri, {0, 0, 7}, RegisterId::R1, RegisterId::ZERO);
        appendInstruction(Executable, Opcode::Add64rrr, {0, 0, 0}, RegisterId::R0, RegisterId::R0, RegisterId::R1);
        appendInstruction(Executable, Opcode::Add64rri, {0, 0, addressOf(Memory.data())}, RegisterId::R9, RegisterId::ZERO);
        appendInstruction(Executable, Opcode::Add64rrm, {0, 0, 1}, RegisterId::R2, RegisterId::R0, RegisterId::R9);
        appendInstruction(Executable, Opcode::Add64rrr, {0, 1, negativeOffset(2)}, RegisterId::R3, RegisterId::R2, RegisterId::R0);
        appendInstruction(Executable, Opcode::Store64i, {9, 123}, RegisterId::R9);
        appendInstruction(Executable, Opcode::Load64, {17, 9}, RegisterId::R9, RegisterId::R9);
        appendInstruction(Executable, Opcode::Add64mrr, {25, 0, 0}, RegisterId::R9, RegisterId::R2, RegisterId::R0);
        appendInstruction(Executable, Opcode::Ret);
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));

        EXPECT_EQ(READ_REGISTER(RegisterId::R0, VM.getFrame()), 6U);
        EXPECT_EQ(READ_REGISTER(RegisterId::R1, VM.getFrame()), 7U);
        EXPECT_EQ(READ_REGISTER(RegisterId::R2, VM.getFrame()), 17U);
        EXPECT_EQ(READ_REGISTER(RegisterId::R3, VM.getFrame()), 22U);
        EXPECT_EQ(READ_REGISTER(RegisterId::R9, VM.getFrame()), addressOf(Memory.data()));
        EXPECT_EQ(readWord(Memory.data() + 1), Input);
        EXPECT_EQ(readWord(Memory.data() + 9), 123U);
        EXPECT_EQ(readWord(Memory.data() + 17), 123U);
        EXPECT_EQ(readWord(Memory.data() + 25), 23U);
        EXPECT_EQ(READ_REGISTER(RegisterId::ZERO, VM.getFrame()), 0U);
    }

    // Immediate memory addition wraps at 64 bits and reads an unaligned aliased source before writing its destination.
    TEST(VMArithmeticTest, AddsImmediateToUnalignedAliasedMemoryWithWrapping)
    {
        struct AdditionCase
        {
                std::uint64_t Input;
                std::uint64_t Immediate;
                std::uint64_t Expected;
        };
        constexpr AdditionCase Cases[] = {
            {std::numeric_limits<std::uint64_t>::max(), 1, 0},
            {0, std::numeric_limits<std::uint64_t>::max(), std::numeric_limits<std::uint64_t>::max()},
            {std::uint64_t{1} << 63, std::uint64_t{1} << 63, 0},
            {42, std::numeric_limits<std::uint64_t>::max(), 41},
        };
        constexpr std::size_t DestinationOffsets[] = {
            1,
            3,
            13,
        };
        for (const auto& Case : Cases)
        {
            for (const auto DestinationOffset : DestinationOffsets)
            {
                SCOPED_TRACE(Case.Input);
                SCOPED_TRACE(Case.Immediate);
                SCOPED_TRACE(DestinationOffset);
                std::array<std::uint8_t, 32> Memory;
                Memory.fill(0xA5);
                std::memcpy(Memory.data() + 1, &Case.Input, sizeof(Case.Input));
                auto ExpectedMemory = Memory;
                std::memcpy(ExpectedMemory.data() + DestinationOffset, &Case.Expected, sizeof(Case.Expected));
                Interpreter VM;
                ExecutableFile Executable;
                appendInstruction(Executable, Opcode::Add64mmi, {addressOf(Memory.data() + DestinationOffset), addressOf(Memory.data() + 1), Case.Immediate});
                const auto ReturnOffset = appendInstruction(Executable, Opcode::Ret);
                ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
                const auto* Entry = VM.getCodeSegment().data();
                const auto Bindings = VM.getFrame().Register;

                ASSERT_TRUE(VM.execute(Entry));

                EXPECT_EQ(Memory, ExpectedMemory);
                EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(Entry + ReturnOffset));
                EXPECT_EQ(VM.getFrame().Register, Bindings);
                EXPECT_EQ(READ_REGISTER(RegisterId::ZERO, VM.getFrame()), 0U);
            }
        }
    }

    // Memory-memory addition reads both unaligned inputs before a store that may overlap either or both input ranges.
    TEST(VMArithmeticTest, AddsOverlappingMemorySourcesBeforeCommittingResult)
    {
        constexpr std::size_t DestinationOffsets[] = {
            1,
            5,
            9,
            17,
        };
        constexpr std::size_t RightOffsets[] = {
            1,
            9,
        };
        for (const auto RightOffset : RightOffsets)
        {
            for (const auto DestinationOffset : DestinationOffsets)
            {
                SCOPED_TRACE(RightOffset);
                SCOPED_TRACE(DestinationOffset);
                std::array<std::uint8_t, 40> Memory;
                Memory.fill(0x5A);
                constexpr auto Left = std::numeric_limits<std::uint64_t>::max();
                const std::uint64_t Right = RightOffset == 1 ? Left : 2;
                const std::uint64_t ExpectedResult = RightOffset == 1 ? std::numeric_limits<std::uint64_t>::max() - 1 : 1;
                std::memcpy(Memory.data() + 1, &Left, sizeof(Left));
                std::memcpy(Memory.data() + RightOffset, &Right, sizeof(Right));
                auto ExpectedMemory = Memory;
                std::memcpy(ExpectedMemory.data() + DestinationOffset, &ExpectedResult, sizeof(ExpectedResult));
                Interpreter VM;
                ExecutableFile Executable;
                appendInstruction(Executable, Opcode::Add64mmm, {addressOf(Memory.data() + DestinationOffset), addressOf(Memory.data() + 1), addressOf(Memory.data() + RightOffset)});
                const auto ReturnOffset = appendInstruction(Executable, Opcode::Ret);
                ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
                const auto* Entry = VM.getCodeSegment().data();

                ASSERT_TRUE(VM.execute(Entry));

                EXPECT_EQ(Memory, ExpectedMemory);
                EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(Entry + ReturnOffset));
            }
        }
    }

    // Mixed instruction widths and PC-relative memory operands use each current instruction's start address after PC advances.
    TEST(VMArithmeticTest, AdvancesPCBetweenMixedWidthsAndRelativeMemoryAdds)
    {
        Interpreter VM;
        ExecutableFile Executable;
        Executable.getImage().resize(32);
        constexpr std::uint64_t Input = 7;
        constexpr auto Maximum = std::numeric_limits<std::uint64_t>::max();
        std::memcpy(Executable.getImage().data(), &Input, sizeof(Input));
        std::memcpy(Executable.getImage().data() + 8, &Maximum, sizeof(Maximum));
        const auto EntryOffset = Executable.byteSize();
        appendInstruction(Executable, Opcode::Add64mmi, {negativeOffset(EntryOffset), negativeOffset(EntryOffset), 5}, RegisterId::PC, RegisterId::PC);
        const auto StoreOffset = Executable.byteSize();
        appendInstruction(Executable, Opcode::Store32i, {negativeOffset(StoreOffset - 24), 0x55667788}, RegisterId::PC);
        const auto AddOffset = Executable.byteSize();
        appendInstruction(Executable, Opcode::Add64mmm, {negativeOffset(AddOffset - 16), negativeOffset(AddOffset), negativeOffset(AddOffset - 8)}, RegisterId::PC, RegisterId::PC, RegisterId::PC);
        const auto ReturnOffset = appendInstruction(Executable, Opcode::Ret);
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto* Image = VM.getCodeSegment().data();

        ASSERT_TRUE(VM.execute(Image + EntryOffset));

        EXPECT_EQ(readWord(Image), 12U);
        EXPECT_EQ(readWord(Image + 8), Maximum);
        EXPECT_EQ(readWord(Image + 16), 11U);
        EXPECT_EQ(readWord(Image + 24), 0x55667788U);
        EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(Image + ReturnOffset));
    }
} // namespace ink::vm::test
