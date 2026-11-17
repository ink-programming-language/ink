#include "../../../core/support/environment_test_support.h"

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace ink::vm::test
{
    namespace
    {
        constexpr std::size_t Add64Bytes = 32;
        constexpr std::size_t ReturnBytes = 8;
        constexpr std::size_t JumpBytes = 16;

        void appendWord(std::vector<std::uint8_t>& Bytes, std::uint64_t Bits)
        {
            for (std::size_t Index = 0; Index < 8; ++Index)
            {
                Bytes.push_back(static_cast<std::uint8_t>(Bits >> (Index * 8)));
            }
        }

        void appendHeader(std::vector<std::uint8_t>& Bytes, std::uint16_t Operation, RegisterId First = RegisterId::ZERO, RegisterId Second = RegisterId::ZERO, RegisterId Third = RegisterId::ZERO)
        {
            appendWord(Bytes, Operation | (static_cast<std::uint64_t>(First) << 16) | (static_cast<std::uint64_t>(Second) << 24) | (static_cast<std::uint64_t>(Third) << 32));
        }

        void appendAdd64(std::vector<std::uint8_t>& Bytes, std::uint16_t Operation, RegisterOperand Destination, RegisterOperand LHS, RegisterOperand RHS)
        {
            appendHeader(Bytes, Operation, Destination.getRegister(), LHS.getRegister(), RHS.getRegister());
            appendWord(Bytes, static_cast<std::uint64_t>(Destination.getByteOffset()));
            appendWord(Bytes, static_cast<std::uint64_t>(LHS.getByteOffset()));
            appendWord(Bytes, static_cast<std::uint64_t>(RHS.getByteOffset()));
        }

        void appendAdd64rrr(std::vector<std::uint8_t>& Bytes, RegisterOperand Destination, RegisterOperand LHS, RegisterOperand RHS)
        {
            appendAdd64(Bytes, 0x19, Destination, LHS, RHS);
        }

        void appendAdd64rrm(std::vector<std::uint8_t>& Bytes, RegisterOperand Destination, RegisterOperand LHS, AddressOperand RHS)
        {
            appendAdd64(Bytes, 0x1A, Destination, LHS, RegisterOperand(*RHS.getRegister(), RHS.getByteOffset()));
        }

        void appendAdd64mrr(std::vector<std::uint8_t>& Bytes, AddressOperand Destination, RegisterOperand LHS, RegisterOperand RHS)
        {
            appendAdd64(Bytes, 0x1C, RegisterOperand(*Destination.getRegister(), Destination.getByteOffset()), LHS, RHS);
        }

        void appendRegisterJump(std::vector<std::uint8_t>& Bytes, std::uint16_t Operation, RegisterOperand Target)
        {
            appendHeader(Bytes, Operation, Target.getRegister());
            appendWord(Bytes, static_cast<std::uint64_t>(Target.getByteOffset()));
        }

        void appendImmediateJump(std::vector<std::uint8_t>& Bytes, std::int64_t Offset)
        {
            appendHeader(Bytes, 0x3F);
            appendWord(Bytes, static_cast<std::uint64_t>(Offset));
        }

        void appendRet(std::vector<std::uint8_t>& Bytes)
        {
            appendHeader(Bytes, 0x3A);
        }

        std::uint64_t registerAddress(const Interpreter& VM, RegisterId Id) noexcept
        {
            return READ_REGISTER(Id, VM.getFrame());
        }

    } // namespace

    // Several additions advance PC by their encoded lengths and apply signed offsets to the address slots themselves.
    TEST(VMDispatchTest, ExecutesSequentialRegisterAdditionsWithSignedOffsets)
    {
        core::test::ScopedEnvironmentVariable Environment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(Environment.set("64"));
        Interpreter VM;
        ExecutableFile Executable;
        auto& Bytes = Executable.getImage();
        appendRet(Bytes);
        appendAdd64rrr(Bytes, RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::SP, 16), RegisterOperand(RegisterId::ZERO, 8));
        appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::SP, -8), RegisterOperand(RegisterId::ZERO));
        appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::BP, 4), RegisterOperand(RegisterId::ZERO, -4));
        appendRet(Bytes);
        const auto ExpectedImage = Bytes;
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto* Entry = VM.getCodeSegment().data() + ReturnBytes;

        ASSERT_TRUE(VM.execute(Entry));

        const auto StackTop = registerAddress(VM, RegisterId::SP);
        ASSERT_NE(StackTop, 0U);
        EXPECT_EQ(registerAddress(VM, RegisterId::BP), StackTop - 8);
        EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + 3 * Add64Bytes));
        EXPECT_TRUE(std::ranges::equal(VM.getCodeSegment(), ExpectedImage));
    }

    // A destination used by both sources is read twice before commit, and PC sources use the current instruction's start address.
    TEST(VMDispatchTest, ReadsAliasedSourcesAndCurrentInstructionPC)
    {
        core::test::ScopedEnvironmentVariable Environment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(Environment.set("64"));
        Interpreter VM;
        ExecutableFile Executable;
        auto& Bytes = Executable.getImage();
        appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO), RegisterOperand(RegisterId::ZERO, 8));
        appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::BP));
        appendRet(Bytes);
        const auto PCProgramOffset = Bytes.size();
        appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::PC, -static_cast<std::int64_t>(PCProgramOffset)), RegisterOperand(RegisterId::ZERO, 7));
        appendRet(Bytes);
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto* Entry = VM.getCodeSegment().data();

        ASSERT_TRUE(VM.execute(Entry));

        EXPECT_EQ(registerAddress(VM, RegisterId::BP), 16U);
        EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + 2 * Add64Bytes));

        ASSERT_TRUE(VM.execute(Entry + PCProgramOffset));

        EXPECT_EQ(registerAddress(VM, RegisterId::BP), reinterpret_cast<std::uintptr_t>(Entry + 7));
        EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + PCProgramOffset + Add64Bytes));
    }

    // A memory write to the live PC storage is superseded by normal advancement before either a PC source or its memory alias is read.
    TEST(VMDispatchTest, AdvancesPastAliasedPCWriteAndSynchronizesBothReadForms)
    {
        Interpreter VM;
        const auto PCStorage = static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(GET_REGISTER(RegisterId::PC, VM.getFrame())));
        ExecutableFile Executable;
        auto& Bytes = Executable.getImage();
        appendAdd64rrr(Bytes, RegisterOperand(RegisterId::R0), RegisterOperand(RegisterId::ZERO, PCStorage), RegisterOperand(RegisterId::ZERO));
        appendHeader(Bytes, 0x0C, RegisterId::R0);
        appendWord(Bytes, 0);
        appendWord(Bytes, 0);
        const auto MemoryReadOffset = Bytes.size();
        appendAdd64rrm(Bytes, RegisterOperand(RegisterId::R1), RegisterOperand(RegisterId::ZERO), AddressOperand(RegisterId::R0));
        const auto RegisterReadOffset = Bytes.size();
        appendAdd64rrr(Bytes, RegisterOperand(RegisterId::R2), RegisterOperand(RegisterId::PC), RegisterOperand(RegisterId::ZERO));
        const auto ReturnOffset = Bytes.size();
        appendRet(Bytes);
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto* Entry = VM.getCodeSegment().data();

        ASSERT_TRUE(VM.execute(Entry));

        EXPECT_EQ(registerAddress(VM, RegisterId::R1), reinterpret_cast<std::uintptr_t>(Entry + MemoryReadOffset));
        EXPECT_EQ(registerAddress(VM, RegisterId::R2), reinterpret_cast<std::uintptr_t>(Entry + RegisterReadOffset));
        EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + ReturnOffset));
    }

    // An unsupported opcode reached after a completed instruction reports failure with PC at the failing instruction rather than its predecessor.
    TEST(VMDispatchTest, PreservesAdvancedPCWhenFollowingInstructionIsUnsupported)
    {
        Interpreter VM;
        ExecutableFile Executable;
        auto& Bytes = Executable.getImage();
        appendAdd64rrr(Bytes, RegisterOperand(RegisterId::R0), RegisterOperand(RegisterId::ZERO, 19), RegisterOperand(RegisterId::ZERO));
        const auto UnsupportedOffset = Bytes.size();
        appendHeader(Bytes, 0x01);
        appendWord(Bytes, 0);
        appendWord(Bytes, 0);
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto* Entry = VM.getCodeSegment().data();
        core::CollectingDiagnosticConsumer Collector;
        VM.getDiagnostics().addConsumer(Collector);

        EXPECT_FALSE(VM.execute(Entry));

        VM.getDiagnostics().removeConsumer(Collector);
        ASSERT_EQ(Collector.diagnostics().size(), 1U);
        EXPECT_EQ(Collector.diagnostics().front().Kind, core::DiagnosticKind::VMExecutionFailed);
        EXPECT_EQ(registerAddress(VM, RegisterId::R0), 19U);
        EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + UnsupportedOffset));
    }

    // ZERO-relative integer bit patterns exercise 64-bit wrapping arithmetic without changing register storage bindings.
    TEST(VMDispatchTest, WrapsRegisterArithmeticAt64BitBoundaries)
    {
        struct AdditionCase
        {
                std::int64_t LHS;
                std::int64_t RHS;
                std::uint64_t Result;
        };
        const AdditionCase Cases[] = {
            {0, 0, 0},
            {1, -1, 0},
            {-1, 1, 0},
            {0, -1, std::numeric_limits<std::uint64_t>::max()},
            {std::numeric_limits<std::int64_t>::max(), 1, std::uint64_t{1} << 63},
            {std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::min(), 0},
            {-1, 0, std::numeric_limits<std::uint64_t>::max()},
            {std::numeric_limits<std::int64_t>::min(), -1, static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())},
        };
        core::test::ScopedEnvironmentVariable Environment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(Environment.set("64"));
        for (const auto& Case : Cases)
        {
            SCOPED_TRACE(Case.LHS);
            SCOPED_TRACE(Case.RHS);
            Interpreter VM;
            ExecutableFile Executable;
            auto& Bytes = Executable.getImage();
            appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO, Case.LHS), RegisterOperand(RegisterId::ZERO, Case.RHS));
            appendRet(Bytes);
            ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
            const auto* Entry = VM.getCodeSegment().data();
            const auto Bindings = VM.getFrame().Register;

            ASSERT_TRUE(VM.execute(Entry));

            EXPECT_EQ(VM.getFrame().Register, Bindings);
            EXPECT_EQ(registerAddress(VM, RegisterId::BP), Case.Result);
            EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + Add64Bytes));
            EXPECT_EQ(READ_REGISTER(RegisterId::ZERO, VM.getFrame()), 0U);
        }
    }

    // SP additions store raw address values at and outside allocation boundaries without dereferencing those addresses.
    TEST(VMDispatchTest, StoresRawStackRegisterAddressArithmetic)
    {
        constexpr std::int64_t Offsets[] = {
            64,
            65,
            -1,
        };
        core::test::ScopedEnvironmentVariable Environment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(Environment.set("64"));
        for (const auto Offset : Offsets)
        {
            SCOPED_TRACE(Offset);
            Interpreter VM;
            ExecutableFile Executable;
            auto& Bytes = Executable.getImage();
            appendAdd64rrr(Bytes, RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::ZERO, Offset));
            appendRet(Bytes);
            ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
            const auto* Entry = VM.getCodeSegment().data();

            ASSERT_TRUE(VM.execute(Entry));

            EXPECT_EQ(registerAddress(VM, RegisterId::SP), registerAddress(VM, RegisterId::BP) + static_cast<std::uint64_t>(Offset));
            EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + Add64Bytes));
        }
    }

    // Memory forms access unaligned stack bytes while register destinations update stored values and preserve every register binding.
    TEST(VMDispatchTest, DistinguishesStackMemoryOperandsFromRegisterStorage)
    {
        core::test::ScopedEnvironmentVariable Environment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(Environment.set("64"));
        Interpreter VM;
        ExecutableFile Executable;
        auto& Bytes = Executable.getImage();
        appendRet(Bytes);
        appendAdd64mrr(Bytes, AddressOperand(RegisterId::SP, 3), RegisterOperand(RegisterId::ZERO, 17), RegisterOperand(RegisterId::ZERO, 25));
        appendAdd64rrm(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO, 8), AddressOperand(RegisterId::BP, 3));
        appendAdd64mrr(Bytes, AddressOperand(RegisterId::SP, 17), RegisterOperand(RegisterId::BP, -1), RegisterOperand(RegisterId::ZERO, 2));
        appendRet(Bytes);
        const auto ExpectedImage = Bytes;
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto* Entry = VM.getCodeSegment().data();
        ASSERT_TRUE(VM.execute(Entry));
        const auto StackAddress = registerAddress(VM, RegisterId::SP);
        ASSERT_NE(StackAddress, 0U);
        auto* const Stack = reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(StackAddress));
        std::array<std::uint8_t, 64> Expected;
        Expected.fill(0xA5);
        std::memcpy(Stack, Expected.data(), Expected.size());
        const std::uint64_t FirstResult = 42;
        const std::uint64_t SecondResult = 51;
        std::memcpy(Expected.data() + 3, &FirstResult, sizeof(FirstResult));
        std::memcpy(Expected.data() + 17, &SecondResult, sizeof(SecondResult));
        const auto RegisterBindings = VM.getFrame().Register;

        ASSERT_TRUE(VM.execute(Entry + ReturnBytes));

        EXPECT_EQ(registerAddress(VM, RegisterId::SP), StackAddress);
        EXPECT_EQ(registerAddress(VM, RegisterId::BP), 50U);
        EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + ReturnBytes + 3 * Add64Bytes));
        EXPECT_EQ(VM.getFrame().Register, RegisterBindings);
        EXPECT_TRUE(std::ranges::equal(std::span<const std::uint8_t>(Stack, Expected.size()), Expected));
        EXPECT_TRUE(std::ranges::equal(VM.getCodeSegment(), ExpectedImage));
    }

    // ZERO-relative memory operands load and store absolute unaligned addresses, leaving source bytes and surrounding padding intact.
    TEST(VMDispatchTest, ReadsAndWritesAbsoluteMemoryThroughZeroRegisterStorage)
    {
        std::array<std::uint8_t, 32> Memory;
        Memory.fill(0x5A);
        constexpr std::uint64_t Input = 0x1020304050607080;
        std::memcpy(Memory.data() + 3, &Input, sizeof(Input));
        auto Expected = Memory;
        constexpr std::uint64_t Result = Input + 14;
        std::memcpy(Expected.data() + 17, &Result, sizeof(Result));
        const auto InputAddress = static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(Memory.data() + 3));
        const auto OutputAddress = static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(Memory.data() + 17));
        core::test::ScopedEnvironmentVariable Environment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(Environment.set("64"));
        Interpreter VM;
        ExecutableFile Executable;
        auto& Bytes = Executable.getImage();
        appendAdd64rrm(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO, 7), AddressOperand(RegisterId::ZERO, InputAddress));
        appendAdd64mrr(Bytes, AddressOperand(RegisterId::ZERO, OutputAddress), RegisterOperand(RegisterId::BP, -2), RegisterOperand(RegisterId::ZERO, 9));
        appendAdd64rrm(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO), AddressOperand(RegisterId::ZERO, OutputAddress));
        appendRet(Bytes);
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto* Entry = VM.getCodeSegment().data();

        ASSERT_TRUE(VM.execute(Entry));

        EXPECT_EQ(Memory, Expected);
        EXPECT_EQ(registerAddress(VM, RegisterId::BP), Result);
        EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + 3 * Add64Bytes));
        EXPECT_EQ(READ_REGISTER(RegisterId::ZERO, VM.getFrame()), 0U);
    }

    // Block-style jumps take a finite forward/backward path and preserve the arithmetic result when control reaches RET.
    TEST(VMDispatchTest, FollowsForwardAndBackwardBlockJumps)
    {
        Interpreter VM;
        ExecutableFile Executable;
        auto& Bytes = Executable.getImage();
        appendRegisterJump(Bytes, 0x2B, RegisterOperand(RegisterId::PC, 2 * JumpBytes + Add64Bytes));
        appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO, 40), RegisterOperand(RegisterId::ZERO, 2));
        appendRegisterJump(Bytes, 0x2B, RegisterOperand(RegisterId::PC, 2 * JumpBytes));
        appendRegisterJump(Bytes, 0x2B, RegisterOperand(RegisterId::PC, -static_cast<std::int64_t>(JumpBytes + Add64Bytes)));
        appendRet(Bytes);
        const auto ExpectedImage = Bytes;
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto* Entry = VM.getCodeSegment().data();

        ASSERT_TRUE(VM.execute(Entry));

        EXPECT_EQ(registerAddress(VM, RegisterId::BP), 42U);
        EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + 3 * JumpBytes + Add64Bytes));
        EXPECT_TRUE(std::ranges::equal(VM.getCodeSegment(), ExpectedImage));
    }

    // Immediate jumps use their own instruction PC, while register jumps read the stored target and add the encoded offset without indirection.
    TEST(VMDispatchTest, ExecutesImmediateAndRegisterTargetJumps)
    {
        Interpreter VM;
        ExecutableFile Executable;
        auto& Bytes = Executable.getImage();
        appendImmediateJump(Bytes, JumpBytes + ReturnBytes);
        appendWord(Bytes, std::numeric_limits<std::uint64_t>::max());
        appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::PC, Add64Bytes + JumpBytes), RegisterOperand(RegisterId::ZERO));
        appendRegisterJump(Bytes, 0x40, RegisterOperand(RegisterId::BP, ReturnBytes));
        appendWord(Bytes, std::numeric_limits<std::uint64_t>::max());
        appendRet(Bytes);
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto* Entry = VM.getCodeSegment().data();

        ASSERT_TRUE(VM.execute(Entry));

        EXPECT_EQ(registerAddress(VM, RegisterId::BP), reinterpret_cast<std::uintptr_t>(Entry + 2 * JumpBytes + Add64Bytes + ReturnBytes));
        EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + 2 * JumpBytes + Add64Bytes + 2 * ReturnBytes));
    }

    // Indirect jumps load an unaligned target pointer through either ZERO-based absolute memory or a stored register base.
    TEST(VMDispatchTest, LoadsIndirectJumpTargetsFromRegisterAndZeroBasedMemory)
    {
        constexpr RegisterId Bases[] = {
            RegisterId::ZERO,
            RegisterId::BP,
        };
        for (const auto Base : Bases)
        {
            SCOPED_TRACE(static_cast<std::uint8_t>(Base));
            std::array<std::uint8_t, 24> TargetMemory;
            TargetMemory.fill(0xA5);
            const auto MemoryAddress = static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(TargetMemory.data()));
            Interpreter VM;
            ExecutableFile Executable;
            auto& Bytes = Executable.getImage();
            if (Base == RegisterId::BP)
            {
                appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO, MemoryAddress), RegisterOperand(RegisterId::ZERO));
            }
            appendRegisterJump(Bytes, 0x41, RegisterOperand(Base, Base == RegisterId::ZERO ? MemoryAddress + 3 : 3));
            appendWord(Bytes, std::numeric_limits<std::uint64_t>::max());
            const auto ReturnOffset = Bytes.size();
            appendRet(Bytes);
            ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
            const auto* Entry = VM.getCodeSegment().data();
            const auto Target = reinterpret_cast<std::uintptr_t>(Entry + ReturnOffset);
            std::memcpy(TargetMemory.data() + 3, &Target, sizeof(Target));
            const auto ExpectedMemory = TargetMemory;

            ASSERT_TRUE(VM.execute(Entry));

            EXPECT_EQ(registerAddress(VM, RegisterId::PC), Target);
            EXPECT_EQ(TargetMemory, ExpectedMemory);
            EXPECT_EQ(READ_REGISTER(RegisterId::ZERO, VM.getFrame()), 0U);
        }
    }
} // namespace ink::vm::test
