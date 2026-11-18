#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <algorithm>
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

        template <typename BuildFunction>
        void executeBuiltInstructions(BuildFunction Build)
        {
            const auto EntryName = core::NamePool::instance().intern("arithmetic-test-entry");
            MachineModule Module;
            auto& Block = Module.createFunction(EntryName).createBasicBlock();
            MachineInstructionBuilder Builder;
            Builder.setInsertPoint(Block);
            Build(Builder);
            Builder.create<Opcode::Ret>();

            Interpreter VM;
            ExecutableEmitter Emitter(VM.getDiagnostics());
            auto Executable = Emitter.emit(Module, EntryName, {});
            const auto EntryOffset = Executable.getEntryOffset();
            ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
            ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + EntryOffset));
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

    // All narrow loads and the remaining stores copy only their encoded width at unaligned register-relative addresses.
    TEST(VMArithmeticTest, CopiesExactWidthWithMemoryAndImmediateOperands)
    {
        alignas(8) std::array<std::uint8_t, 100> Memory;
        Memory.fill(0xA5);
        constexpr std::array<std::uint8_t, 8> Input{0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
        std::ranges::copy(Input, Memory.begin() + 1);
        auto ExpectedMemory = Memory;
        constexpr std::array<std::size_t, 7> CopyWidths{1, 2, 4, 1, 2, 4, 8};
        for (std::size_t Index = 0; Index < CopyWidths.size(); ++Index)
        {
            std::copy_n(Input.begin(), CopyWidths[Index], ExpectedMemory.begin() + 11 + Index * 10);
        }
        ExpectedMemory[81] = 0xAB;
        ExpectedMemory[91] = 0xEF;
        ExpectedMemory[92] = 0xBE;

        const auto Build = [&](MachineInstructionBuilder& Builder)
        {
            const auto At = [](std::int64_t Offset)
            {
                return AddressOperand(RegisterId::R9, Offset - 3);
            };
            const auto Source = At(1);
            Builder.create<Opcode::Add64rri>(RegisterOperand(RegisterId::R9), RegisterOperand(RegisterId::ZERO), ImmediateOperand(addressOf(Memory.data() + 3)));
            Builder.create<Opcode::Load8>(At(11), Source);
            Builder.create<Opcode::Load16>(At(21), Source);
            Builder.create<Opcode::Load32>(At(31), Source);
            Builder.create<Opcode::Store8m>(At(41), Source);
            Builder.create<Opcode::Store16m>(At(51), Source);
            Builder.create<Opcode::Store32m>(At(61), Source);
            Builder.create<Opcode::Store64m>(At(71), Source);
            Builder.create<Opcode::Store8i>(At(81), ImmediateOperand(0x1AB));
            Builder.create<Opcode::Store16i>(At(91), ImmediateOperand(0x1BEEF));
        };
        ASSERT_NO_FATAL_FAILURE(executeBuiltInstructions(Build));

        EXPECT_EQ(Memory, ExpectedMemory);
    }

    // Each narrow addition form wraps at its own width and leaves the surrounding destination bytes unchanged.
    TEST(VMArithmeticTest, WrapsEveryNarrowMemoryAdditionForm)
    {
        alignas(8) std::array<std::uint8_t, 70> Memory;
        Memory.fill(0xA5);
        std::fill_n(Memory.begin() + 1, 4, 0xFF);
        std::fill_n(Memory.begin() + 7, 4, 0);
        Memory[7] = 2;
        auto ExpectedMemory = Memory;
        constexpr std::array<std::size_t, 9> ResultWidths{1, 1, 1, 2, 2, 2, 4, 4, 4};
        for (std::size_t Index = 0; Index < ResultWidths.size(); ++Index)
        {
            const auto Offset = 15 + Index * 6;
            std::fill_n(ExpectedMemory.begin() + Offset, ResultWidths[Index], 0);
            ExpectedMemory[Offset] = static_cast<std::uint8_t>(1 + Index % 3);
        }

        const auto Build = [&](MachineInstructionBuilder& Builder)
        {
            const auto At = [](std::int64_t Offset)
            {
                return AddressOperand(RegisterId::R9, Offset);
            };
            const auto Left = At(1);
            const auto Right = At(7);
            const ImmediateOperand Max(std::numeric_limits<std::uint64_t>::max());
            const ImmediateOperand Three(3);
            const ImmediateOperand Four(4);
            Builder.create<Opcode::Add64rri>(RegisterOperand(RegisterId::R9), RegisterOperand(RegisterId::ZERO), ImmediateOperand(addressOf(Memory.data())));
            Builder.create<Opcode::Add8mmm>(At(15), Left, Right);
            Builder.create<Opcode::Add8mmi>(At(21), Left, Three);
            Builder.create<Opcode::Add8mii>(At(27), Max, Four);
            Builder.create<Opcode::Add16mmm>(At(33), Left, Right);
            Builder.create<Opcode::Add16mmi>(At(39), Left, Three);
            Builder.create<Opcode::Add16mii>(At(45), Max, Four);
            Builder.create<Opcode::Add32mmm>(At(51), Left, Right);
            Builder.create<Opcode::Add32mmi>(At(57), Left, Three);
            Builder.create<Opcode::Add32mii>(At(63), Max, Four);
        };
        ASSERT_NO_FATAL_FAILURE(executeBuiltInstructions(Build));

        EXPECT_EQ(Memory, ExpectedMemory);
    }

    // The remaining 64-bit memory additions apply signed register offsets and read an aliased source before writing.
    TEST(VMArithmeticTest, Adds64BitImmediatesAndDisplacedRegisterSources)
    {
        alignas(8) std::array<std::uint8_t, 32> Memory;
        Memory.fill(0xA5);
        constexpr std::uint64_t Input = 7;
        std::memcpy(Memory.data() + 11, &Input, sizeof(Input));
        auto ExpectedMemory = Memory;
        constexpr std::uint64_t ImmediateSum = 41;
        constexpr std::uint64_t AliasedSum = 2;
        constexpr std::uint64_t RegisterSum = 5;
        std::memcpy(ExpectedMemory.data() + 1, &ImmediateSum, sizeof(ImmediateSum));
        std::memcpy(ExpectedMemory.data() + 11, &AliasedSum, sizeof(AliasedSum));
        std::memcpy(ExpectedMemory.data() + 21, &RegisterSum, sizeof(RegisterSum));

        const auto Build = [&](MachineInstructionBuilder& Builder)
        {
            const auto At = [](std::int64_t Offset)
            {
                return AddressOperand(RegisterId::R9, Offset);
            };
            const ImmediateOperand Max(std::numeric_limits<std::uint64_t>::max());
            Builder.create<Opcode::Add64rri>(RegisterOperand(RegisterId::R9), RegisterOperand(RegisterId::ZERO), ImmediateOperand(addressOf(Memory.data())));
            Builder.create<Opcode::Add64rri>(RegisterOperand(RegisterId::R0), RegisterOperand(RegisterId::ZERO), ImmediateOperand(2));
            Builder.create<Opcode::Add64mii>(At(1), Max, ImmediateOperand(42));
            Builder.create<Opcode::Add64mrm>(At(11), RegisterOperand(RegisterId::R0, -7), At(11));
            Builder.create<Opcode::Add64mri>(At(21), RegisterOperand(RegisterId::R0, 4), Max);
        };
        ASSERT_NO_FATAL_FAILURE(executeBuiltInstructions(Build));

        EXPECT_EQ(Memory, ExpectedMemory);
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
