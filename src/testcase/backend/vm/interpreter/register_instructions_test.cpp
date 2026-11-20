#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace ink::vm::test
{
    namespace
    {
        std::uint64_t addressOf(const void* Address) noexcept
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

        bool expectedCompare64(std::size_t Relation, std::uint64_t Left, std::uint64_t Right) noexcept
        {
            const auto SignedLeft = static_cast<std::int64_t>(Left);
            const auto SignedRight = static_cast<std::int64_t>(Right);
            switch (Relation)
            {
            case 0:
                return Left == Right;
            case 1:
                return Left != Right;
            case 2:
                return SignedLeft < SignedRight;
            case 3:
                return SignedLeft <= SignedRight;
            case 4:
                return SignedLeft > SignedRight;
            case 5:
                return SignedLeft >= SignedRight;
            case 6:
                return Left < Right;
            case 7:
                return Left <= Right;
            case 8:
                return Left > Right;
            case 9:
                return Left >= Right;
            }
            return false;
        }

        void expectExecutionFailure(const core::CollectingDiagnosticConsumer& Collector, std::string_view Substring)
        {
            ASSERT_EQ(Collector.diagnostics().size(), 1U);
            EXPECT_EQ(Collector.diagnostics().front().Kind, core::DiagnosticKind::VMExecutionFailed);
            EXPECT_NE(core::DiagnosticFormatter{}.format(Collector.diagnostics().front()).Message.find(Substring), std::string::npos);
        }
    } // namespace

    // mov64rr and mov64ri transfer full 64-bit values, apply signed source displacements, support destination aliasing, and relocate symbol addresses.
    TEST(VMRegisterInstructionsTest, MovesImmediatesDisplacedRegistersAndRelocatedSymbols)
    {
        auto& Names = core::NamePool::instance();
        const auto EntryName = Names.intern("reg-mov-entry");
        const auto GlobalName = Names.intern("reg-mov-global");
        const std::array<std::uint8_t, 8> InitialBytes{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(16, 8, InitialBytes, GlobalName).has_value());
        auto& Block = Module.createFunction(EntryName).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R0), ImmediateOperand(0xFEDCBA9876543210ULL));
        Builder.create<Opcode::Mov64rr>(RegisterOperand(RegisterId::R1), RegisterOperand(RegisterId::R0, 16));
        Builder.create<Opcode::Mov64rr>(RegisterOperand(RegisterId::R1), RegisterOperand(RegisterId::R1, -20));
        Builder.create<Opcode::Mov64rr>(RegisterOperand(RegisterId::R2), RegisterOperand(RegisterId::ZERO, -5));
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R3), ImmediateOperand(GlobalName, 4));
        Builder.create<Opcode::Load32urm>(RegisterOperand(RegisterId::R4), AddressOperand(RegisterId::R3));
        Builder.create<Opcode::Ret>();

        Interpreter VM;
        ExecutableEmitter Emitter(VM.getDiagnostics());
        const std::array Exports{EntryName, GlobalName};
        auto Executable = Emitter.emit(Module, EntryName, Exports);
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.resolveSymbolAddress(EntryName)));

        EXPECT_EQ(READ_REGISTER(RegisterId::R0, VM.getFrame()), 0xFEDCBA9876543210ULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R1, VM.getFrame()), 0xFEDCBA987654320CULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R2, VM.getFrame()), static_cast<std::uint64_t>(-5));
        EXPECT_EQ(READ_REGISTER(RegisterId::R3, VM.getFrame()), addressOf(VM.resolveSymbolAddress(GlobalName)) + 4);
        EXPECT_EQ(READ_REGISTER(RegisterId::R4, VM.getFrame()), 0x88776655U);
    }

    // Unsigned, signed, and 64-bit register loads zero-extend or sign-extend unaligned bytes and support destination-base aliasing.
    TEST(VMRegisterInstructionsTest, LoadsZeroExtendedSignExtendedAndFullWidthValuesFromUnalignedMemory)
    {
        alignas(8) std::array<std::uint8_t, 32> Memory;
        Memory.fill(0xA5);
        constexpr std::array<std::uint8_t, 8> NegativeBytes{0x80, 0x90, 0xA0, 0x80, 0x11, 0x22, 0x33, 0xF0};
        constexpr std::array<std::uint8_t, 8> PositiveBytes{0x7F, 0x7E, 0x7D, 0x7C, 0x01, 0x02, 0x03, 0x04};
        std::ranges::copy(NegativeBytes, Memory.begin() + 1);
        std::ranges::copy(PositiveBytes, Memory.begin() + 11);

        const auto EntryName = core::NamePool::instance().intern("reg-load-entry");
        MachineModule Module;
        auto& Block = Module.createFunction(EntryName).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R9), ImmediateOperand(addressOf(Memory.data())));
        Builder.create<Opcode::Load8urm>(RegisterOperand(RegisterId::R0), AddressOperand(RegisterId::R9, 1));
        Builder.create<Opcode::Load16urm>(RegisterOperand(RegisterId::R1), AddressOperand(RegisterId::R9, 1));
        Builder.create<Opcode::Load32urm>(RegisterOperand(RegisterId::R2), AddressOperand(RegisterId::R9, 1));
        Builder.create<Opcode::Load8srm>(RegisterOperand(RegisterId::R3), AddressOperand(RegisterId::R9, 1));
        Builder.create<Opcode::Load16srm>(RegisterOperand(RegisterId::R4), AddressOperand(RegisterId::R9, 1));
        Builder.create<Opcode::Load32srm>(RegisterOperand(RegisterId::R5), AddressOperand(RegisterId::R9, 1));
        Builder.create<Opcode::Load8srm>(RegisterOperand(RegisterId::R6), AddressOperand(RegisterId::R9, 11));
        Builder.create<Opcode::Load16srm>(RegisterOperand(RegisterId::R7), AddressOperand(RegisterId::R9, 11));
        Builder.create<Opcode::Load32srm>(RegisterOperand(RegisterId::R8), AddressOperand(RegisterId::R9, 11));
        Builder.create<Opcode::Load64rm>(RegisterOperand(RegisterId::R9), AddressOperand(RegisterId::R9, 1));
        Builder.create<Opcode::Ret>();

        Interpreter VM;
        ExecutableEmitter Emitter(VM.getDiagnostics());
        auto Executable = Emitter.emit(Module, EntryName, {});
        const auto EntryOffset = Executable.getEntryOffset();
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + EntryOffset));

        EXPECT_EQ(READ_REGISTER(RegisterId::R0, VM.getFrame()), 0x0000000000000080ULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R1, VM.getFrame()), 0x0000000000009080ULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R2, VM.getFrame()), 0x0000000080A09080ULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R3, VM.getFrame()), 0xFFFFFFFFFFFFFF80ULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R4, VM.getFrame()), 0xFFFFFFFFFFFF9080ULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R5, VM.getFrame()), 0xFFFFFFFF80A09080ULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R6, VM.getFrame()), 0x000000000000007FULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R7, VM.getFrame()), 0x0000000000007E7FULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R8, VM.getFrame()), 0x000000007C7D7E7FULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R9, VM.getFrame()), 0xF033221180A09080ULL);
    }

    // Register stores truncate to their encoded widths, apply source register displacements, and leave adjacent unaligned memory intact.
    TEST(VMRegisterInstructionsTest, StoresExactRegisterWidthsWithDisplacementsAtUnalignedAddresses)
    {
        alignas(8) std::array<std::uint8_t, 48> Memory;
        Memory.fill(0xA5);
        auto ExpectedMemory = Memory;
        constexpr std::uint64_t SourceValue = 0x1122334455667780ULL;
        const std::uint8_t ByteValue = static_cast<std::uint8_t>(SourceValue + 3);
        const std::uint16_t WordValue = static_cast<std::uint16_t>(SourceValue + 5);
        const std::uint32_t DwordValue = static_cast<std::uint32_t>(SourceValue - 1);
        const std::uint64_t QwordValue = SourceValue + 0x10;
        std::memcpy(ExpectedMemory.data() + 1, &ByteValue, sizeof(ByteValue));
        std::memcpy(ExpectedMemory.data() + 5, &WordValue, sizeof(WordValue));
        std::memcpy(ExpectedMemory.data() + 11, &DwordValue, sizeof(DwordValue));
        std::memcpy(ExpectedMemory.data() + 21, &QwordValue, sizeof(QwordValue));

        const auto EntryName = core::NamePool::instance().intern("reg-store-entry");
        MachineModule Module;
        auto& Block = Module.createFunction(EntryName).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R9), ImmediateOperand(addressOf(Memory.data())));
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R0), ImmediateOperand(SourceValue));
        Builder.create<Opcode::Store8mr>(AddressOperand(RegisterId::R9, 1), RegisterOperand(RegisterId::R0, 3));
        Builder.create<Opcode::Store16mr>(AddressOperand(RegisterId::R9, 5), RegisterOperand(RegisterId::R0, 5));
        Builder.create<Opcode::Store32mr>(AddressOperand(RegisterId::R9, 11), RegisterOperand(RegisterId::R0, -1));
        Builder.create<Opcode::Store64mr>(AddressOperand(RegisterId::R9, 21), RegisterOperand(RegisterId::R0, 0x10));
        Builder.create<Opcode::Ret>();

        Interpreter VM;
        ExecutableEmitter Emitter(VM.getDiagnostics());
        auto Executable = Emitter.emit(Module, EntryName, {});
        const auto EntryOffset = Executable.getEntryOffset();
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + EntryOffset));

        EXPECT_EQ(Memory, ExpectedMemory);
    }

    // 64-bit register subtraction, multiplication, bitwise logic, and masked shifts compute wrapping bit patterns with aliased destinations.
    TEST(VMRegisterInstructionsTest, ExecutesWrappingSubtractionMultiplicationBitwiseAndMaskedShifts)
    {
        const auto EntryName = core::NamePool::instance().intern("reg-alu-entry");
        MachineModule Module;
        auto& Block = Module.createFunction(EntryName).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R0), ImmediateOperand(5));
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R1), ImmediateOperand(12));
        Builder.create<Opcode::Sub64rrr>(RegisterOperand(RegisterId::R2), RegisterOperand(RegisterId::R0), RegisterOperand(RegisterId::R1));
        Builder.create<Opcode::Sub64rri>(RegisterOperand(RegisterId::R3), RegisterOperand(RegisterId::R2), ImmediateOperand(3));
        Builder.create<Opcode::Mul64rrr>(RegisterOperand(RegisterId::R4), RegisterOperand(RegisterId::R0, 1), RegisterOperand(RegisterId::R1, -2));
        Builder.create<Opcode::Mul64rri>(RegisterOperand(RegisterId::R5), RegisterOperand(RegisterId::ZERO, -3), ImmediateOperand(7));
        Builder.create<Opcode::And64rrr>(RegisterOperand(RegisterId::R6), RegisterOperand(RegisterId::ZERO, 0xF0F0), RegisterOperand(RegisterId::ZERO, 0x0FF0));
        Builder.create<Opcode::And64rri>(RegisterOperand(RegisterId::R6), RegisterOperand(RegisterId::R6), ImmediateOperand(0x00FF));
        Builder.create<Opcode::Or64rrr>(RegisterOperand(RegisterId::R7), RegisterOperand(RegisterId::R6), RegisterOperand(RegisterId::ZERO, 0x0F00));
        Builder.create<Opcode::Or64rri>(RegisterOperand(RegisterId::R7), RegisterOperand(RegisterId::R7), ImmediateOperand(0x0005));
        Builder.create<Opcode::Xor64rrr>(RegisterOperand(RegisterId::R8), RegisterOperand(RegisterId::R7), RegisterOperand(RegisterId::ZERO, 0x0FFF));
        Builder.create<Opcode::Xor64rri>(RegisterOperand(RegisterId::R8), RegisterOperand(RegisterId::R8), ImmediateOperand(0xF000));
        Builder.create<Opcode::Ret>();

        Interpreter VM;
        ExecutableEmitter Emitter(VM.getDiagnostics());
        auto Executable = Emitter.emit(Module, EntryName, {});
        const auto EntryOffset = Executable.getEntryOffset();
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + EntryOffset));

        EXPECT_EQ(READ_REGISTER(RegisterId::R2, VM.getFrame()), static_cast<std::uint64_t>(-7));
        EXPECT_EQ(READ_REGISTER(RegisterId::R3, VM.getFrame()), static_cast<std::uint64_t>(-10));
        EXPECT_EQ(READ_REGISTER(RegisterId::R4, VM.getFrame()), 60U);
        EXPECT_EQ(READ_REGISTER(RegisterId::R5, VM.getFrame()), static_cast<std::uint64_t>(-21));
        EXPECT_EQ(READ_REGISTER(RegisterId::R6, VM.getFrame()), 0x00F0U);
        EXPECT_EQ(READ_REGISTER(RegisterId::R7, VM.getFrame()), 0x0FF5U);
        EXPECT_EQ(READ_REGISTER(RegisterId::R8, VM.getFrame()), 0xF00AU);

        const auto ShiftEntry = core::NamePool::instance().intern("reg-shift-entry");
        MachineModule ShiftModule;
        auto& ShiftBlock = ShiftModule.createFunction(ShiftEntry).createBasicBlock();
        Builder.setInsertPoint(ShiftBlock);
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R0), ImmediateOperand(0x8000000000000001ULL));
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R1), ImmediateOperand(65));
        Builder.create<Opcode::Shl64rrr>(RegisterOperand(RegisterId::R2), RegisterOperand(RegisterId::R0), RegisterOperand(RegisterId::R1));
        Builder.create<Opcode::Shl64rri>(RegisterOperand(RegisterId::R3), RegisterOperand(RegisterId::R0), ImmediateOperand(64));
        Builder.create<Opcode::Lshr64rrr>(RegisterOperand(RegisterId::R4), RegisterOperand(RegisterId::R0), RegisterOperand(RegisterId::R1));
        Builder.create<Opcode::Lshr64rri>(RegisterOperand(RegisterId::R5), RegisterOperand(RegisterId::R0), ImmediateOperand(63));
        Builder.create<Opcode::Ashr64rrr>(RegisterOperand(RegisterId::R6), RegisterOperand(RegisterId::R0), RegisterOperand(RegisterId::R1));
        Builder.create<Opcode::Ashr64rri>(RegisterOperand(RegisterId::R7), RegisterOperand(RegisterId::R0), ImmediateOperand(63));
        Builder.create<Opcode::Ashr64rri>(RegisterOperand(RegisterId::R8), RegisterOperand(RegisterId::ZERO, 8), ImmediateOperand(65));
        Builder.create<Opcode::Ret>();

        auto ShiftExecutable = Emitter.emit(ShiftModule, ShiftEntry, {});
        const auto ShiftOffset = ShiftExecutable.getEntryOffset();
        const auto BaseBefore = VM.getCodeSegment().size();
        ASSERT_NE(VM.loadLibrary(std::move(ShiftExecutable)), Interpreter::InvalidLibraryHandle);
        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + BaseBefore + ShiftOffset));

        EXPECT_EQ(READ_REGISTER(RegisterId::R2, VM.getFrame()), 2U);
        EXPECT_EQ(READ_REGISTER(RegisterId::R3, VM.getFrame()), 0x8000000000000001ULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R4, VM.getFrame()), 0x4000000000000000ULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R5, VM.getFrame()), 1U);
        EXPECT_EQ(READ_REGISTER(RegisterId::R6, VM.getFrame()), 0xC000000000000000ULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R7, VM.getFrame()), 0xFFFFFFFFFFFFFFFFULL);
        EXPECT_EQ(READ_REGISTER(RegisterId::R8, VM.getFrame()), 4U);
    }

    // Signed and unsigned 64-bit division and remainder handle negative truncation toward zero and full unsigned 64-bit dividends.
    TEST(VMRegisterInstructionsTest, ExecutesSignedAndUnsignedDivisionAndRemainderAtBoundaryValues)
    {
        const auto EntryName = core::NamePool::instance().intern("reg-div-entry");
        MachineModule Module;
        auto& Block = Module.createFunction(EntryName).createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R0), ImmediateOperand(static_cast<std::uint64_t>(-17)));
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R1), ImmediateOperand(5));
        Builder.create<Opcode::Sdiv64rrr>(RegisterOperand(RegisterId::R2), RegisterOperand(RegisterId::R0), RegisterOperand(RegisterId::R1));
        Builder.create<Opcode::Srem64rrr>(RegisterOperand(RegisterId::R3), RegisterOperand(RegisterId::R0), RegisterOperand(RegisterId::R1));
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R4), ImmediateOperand(std::numeric_limits<std::uint64_t>::max()));
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R5), ImmediateOperand(3));
        Builder.create<Opcode::Udiv64rrr>(RegisterOperand(RegisterId::R6), RegisterOperand(RegisterId::R4), RegisterOperand(RegisterId::R5));
        Builder.create<Opcode::Urem64rrr>(RegisterOperand(RegisterId::R7), RegisterOperand(RegisterId::R4), RegisterOperand(RegisterId::R5));
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R8), ImmediateOperand(static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::min())));
        Builder.create<Opcode::Sdiv64rrr>(RegisterOperand(RegisterId::R9), RegisterOperand(RegisterId::R8), RegisterOperand(RegisterId::ZERO, 2));
        Builder.create<Opcode::Ret>();

        Interpreter VM;
        ExecutableEmitter Emitter(VM.getDiagnostics());
        auto Executable = Emitter.emit(Module, EntryName, {});
        const auto EntryOffset = Executable.getEntryOffset();
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + EntryOffset));

        EXPECT_EQ(READ_REGISTER(RegisterId::R2, VM.getFrame()), static_cast<std::uint64_t>(-3));
        EXPECT_EQ(READ_REGISTER(RegisterId::R3, VM.getFrame()), static_cast<std::uint64_t>(-2));
        EXPECT_EQ(READ_REGISTER(RegisterId::R6, VM.getFrame()), std::numeric_limits<std::uint64_t>::max() / 3);
        EXPECT_EQ(READ_REGISTER(RegisterId::R7, VM.getFrame()), std::numeric_limits<std::uint64_t>::max() % 3);
        EXPECT_EQ(READ_REGISTER(RegisterId::R9, VM.getFrame()), static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::min() / 2));
    }

    // Division by zero and signed INT64_MIN / -1 or % -1 report VMExecutionFailed without overwriting the destination register or advancing PC.
    TEST(VMRegisterInstructionsTest, ReportsDivisionByZeroAndSignedDivisionOverflowWithoutMutatingDestinationOrPC)
    {
        struct FaultCase
        {
                Opcode Operation;
                std::uint64_t Left;
                std::uint64_t Right;
                std::string_view ExpectedMessage;
        };
        constexpr auto SignedMin = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::min());
        constexpr auto MinusOne = static_cast<std::uint64_t>(-1);
        constexpr FaultCase Cases[] = {
            {Opcode::Sdiv64rrr, 42, 0, "division by zero"},
            {Opcode::Udiv64rrr, 42, 0, "division by zero"},
            {Opcode::Srem64rrr, 42, 0, "division by zero"},
            {Opcode::Urem64rrr, 42, 0, "division by zero"},
            {Opcode::Sdiv64rrr, SignedMin, MinusOne, "signed integer division overflow"},
            {Opcode::Srem64rrr, SignedMin, MinusOne, "signed integer division overflow"},
        };
        for (const auto& Case : Cases)
        {
            SCOPED_TRACE(static_cast<std::uint16_t>(Case.Operation));
            Interpreter VM;
            core::CollectingDiagnosticConsumer Collector;
            VM.getDiagnostics().addConsumer(Collector);
            ExecutableFile Executable;
            appendInstruction(Executable, Opcode::Mov64ri, {0, Case.Left}, RegisterId::R0);
            appendInstruction(Executable, Opcode::Mov64ri, {0, Case.Right}, RegisterId::R1);
            appendInstruction(Executable, Opcode::Mov64ri, {0, 0xCAFEBABEULL}, RegisterId::R9);
            const auto FaultOffset = appendInstruction(Executable, Case.Operation, {0, 0, 0}, RegisterId::R9, RegisterId::R0, RegisterId::R1);
            appendInstruction(Executable, Opcode::Ret);
            ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
            const auto* Entry = VM.getCodeSegment().data();

            EXPECT_FALSE(VM.execute(Entry));

            expectExecutionFailure(Collector, Case.ExpectedMessage);
            EXPECT_EQ(READ_REGISTER(RegisterId::R9, VM.getFrame()), 0xCAFEBABEULL);
            EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(Entry + FaultOffset));
            VM.getDiagnostics().removeConsumer(Collector);
        }
    }

    // All twenty 64-bit register/immediate comparison opcodes write 1 or 0 matching an independent signed/unsigned reference across boundary values.
    TEST(VMRegisterInstructionsTest, EvaluatesEveryIntegerCompareRelationInRegisterAndImmediateForms)
    {
        constexpr std::uint64_t SignBit = std::uint64_t{1} << 63;
        constexpr std::uint64_t Max = std::numeric_limits<std::uint64_t>::max();
        constexpr std::array<std::uint64_t, 2> Inputs[] = {
            {0, 0},
            {0, 1},
            {1, 0},
            {Max, 0},
            {0, Max},
            {SignBit, SignBit - 1},
            {SignBit - 1, SignBit},
            {Max, Max},
        };
        for (std::size_t Relation = 0; Relation < 10; ++Relation)
        {
            for (std::size_t Form = 0; Form < 2; ++Form)
            {
                const auto Operation = static_cast<Opcode>(0x00B8U + Relation * 2 + Form);
                SCOPED_TRACE(static_cast<std::uint16_t>(Operation));
                bool SawTrue = false;
                bool SawFalse = false;
                for (const auto& Input : Inputs)
                {
                    const bool Expected = expectedCompare64(Relation, Input[0], Input[1]);
                    SawTrue |= Expected;
                    SawFalse |= !Expected;
                    Interpreter VM;
                    ExecutableFile Executable;
                    appendInstruction(Executable, Opcode::Mov64ri, {0, Input[0]}, RegisterId::R0);
                    if (Form == 0)
                    {
                        appendInstruction(Executable, Opcode::Mov64ri, {0, Input[1]}, RegisterId::R1);
                        appendInstruction(Executable, Operation, {0, 0, 0}, RegisterId::R0, RegisterId::R0, RegisterId::R1);
                    }
                    else
                    {
                        appendInstruction(Executable, Operation, {0, 0, Input[1]}, RegisterId::R0, RegisterId::R0);
                    }
                    appendInstruction(Executable, Opcode::Ret);
                    ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

                    ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));

                    EXPECT_EQ(READ_REGISTER(RegisterId::R0, VM.getFrame()), Expected ? 1U : 0U);
                }
                EXPECT_TRUE(SawTrue);
                EXPECT_TRUE(SawFalse);
            }
        }
    }

    // All twenty 64-bit register comparison branch opcodes branch or fall through according to signed and unsigned boundary ordering.
    TEST(VMRegisterInstructionsTest, BranchesForEveryRegisterCompareBranchRelationAtBoundaryValues)
    {
        constexpr std::uint64_t SignBit = std::uint64_t{1} << 63;
        constexpr std::uint64_t Max = std::numeric_limits<std::uint64_t>::max();
        constexpr std::array<std::uint64_t, 2> Inputs[] = {
            {0, 0},
            {0, 1},
            {1, 0},
            {Max, 0},
            {0, Max},
            {SignBit, SignBit - 1},
            {SignBit - 1, SignBit},
            {Max, Max},
        };
        constexpr std::size_t BranchBytes = 32;
        constexpr std::size_t ReturnBytes = 8;
        for (std::size_t Relation = 0; Relation < 10; ++Relation)
        {
            for (std::size_t Form = 0; Form < 2; ++Form)
            {
                const auto Operation = static_cast<Opcode>(0x00CCU + Relation * 2 + Form);
                SCOPED_TRACE(static_cast<std::uint16_t>(Operation));
                bool SawTaken = false;
                bool SawFallthrough = false;
                for (const auto& Input : Inputs)
                {
                    const bool Taken = expectedCompare64(Relation, Input[0], Input[1]);
                    SawTaken |= Taken;
                    SawFallthrough |= !Taken;
                    Interpreter VM;
                    ExecutableFile Executable;
                    appendInstruction(Executable, Opcode::Mov64ri, {0, Input[0]}, RegisterId::R0);
                    if (Form == 0)
                    {
                        appendInstruction(Executable, Opcode::Mov64ri, {0, Input[1]}, RegisterId::R1);
                    }
                    const auto BranchOffset = Form == 0 ? appendInstruction(Executable, Operation, {0, 0, BranchBytes + ReturnBytes}, RegisterId::R0, RegisterId::R1, RegisterId::PC) : appendInstruction(Executable, Operation, {0, Input[1], BranchBytes + ReturnBytes}, RegisterId::R0, RegisterId::ZERO, RegisterId::PC);
                    const auto FallthroughOffset = appendInstruction(Executable, Opcode::Ret);
                    const auto TakenOffset = appendInstruction(Executable, Opcode::Ret);
                    ASSERT_EQ(FallthroughOffset, BranchOffset + BranchBytes);
                    ASSERT_EQ(TakenOffset, FallthroughOffset + ReturnBytes);
                    ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
                    const auto* Entry = VM.getCodeSegment().data();

                    ASSERT_TRUE(VM.execute(Entry));

                    EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(Entry + (Taken ? TakenOffset : FallthroughOffset)));
                }
                EXPECT_TRUE(SawTaken);
                EXPECT_TRUE(SawFallthrough);
            }
        }
    }

    // Emitted register comparison branches drive a pure-register accumulation loop using forward and backward block displacements.
    TEST(VMRegisterInstructionsTest, EmitsAndExecutesPureRegisterLoop)
    {
        const auto EntryName = core::NamePool::instance().intern("reg-loop-entry");
        MachineModule Module;
        auto& Function = Module.createFunction(EntryName);
        auto& Entry = Function.createBasicBlock();
        auto& Loop = Function.createBasicBlock();
        auto& Exit = Function.createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Entry);
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R0), ImmediateOperand(0));
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R1), ImmediateOperand(1));
        Builder.create<Opcode::Mov64ri>(RegisterOperand(RegisterId::R2), ImmediateOperand(11));
        Builder.create<Opcode::Jge64rrb>(RegisterOperand(RegisterId::R1), RegisterOperand(RegisterId::R2), BasicBlockOperand(&Exit));
        Builder.setInsertPoint(Loop);
        Builder.create<Opcode::Add64rrr>(RegisterOperand(RegisterId::R0), RegisterOperand(RegisterId::R0), RegisterOperand(RegisterId::R1));
        Builder.create<Opcode::Add64rri>(RegisterOperand(RegisterId::R1), RegisterOperand(RegisterId::R1), ImmediateOperand(1));
        Builder.create<Opcode::Jlt64rib>(RegisterOperand(RegisterId::R1), ImmediateOperand(11), BasicBlockOperand(&Loop));
        Builder.setInsertPoint(Exit);
        Builder.create<Opcode::Ret>();

        Interpreter VM;
        ExecutableEmitter Emitter(VM.getDiagnostics());
        auto Executable = Emitter.emit(Module, EntryName, {});
        const auto EntryOffset = Executable.getEntryOffset();
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + EntryOffset));

        EXPECT_EQ(READ_REGISTER(RegisterId::R0, VM.getFrame()), 55U);
        EXPECT_EQ(READ_REGISTER(RegisterId::R1, VM.getFrame()), 11U);
    }
} // namespace ink::vm::test
