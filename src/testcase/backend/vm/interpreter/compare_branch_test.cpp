#include "../../../core/support/environment_test_support.h"

#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/register_operand.h"
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
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace ink::vm::test
{
    namespace
    {
        void appendInteger(std::vector<std::uint8_t>& Bytes, std::uint64_t Value, std::size_t ByteCount)
        {
            for (std::size_t Index = 0; Index < ByteCount; ++Index)
            {
                Bytes.push_back(static_cast<std::uint8_t>(Value >> (Index * 8)));
            }
        }

        void appendHeader(std::vector<std::uint8_t>& Bytes, std::uint16_t Operation, RegisterId Target = RegisterId::ZERO)
        {
            const auto Header = Operation | (static_cast<std::uint64_t>(RegisterId::ZERO) << 16) | (static_cast<std::uint64_t>(RegisterId::ZERO) << 24) | (static_cast<std::uint64_t>(Target) << 32);
            appendInteger(Bytes, Header, 8);
        }

        std::uint64_t storedRegister(const Frame& Value, RegisterId Id) noexcept
        {
            return READ_REGISTER(Id, Value);
        }

        bool expectedComparison(std::size_t Relation, std::size_t Width, std::uint64_t LeftBits, std::uint64_t RightBits) noexcept
        {
            const auto Mask = Width == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << Width) - 1;
            const auto Left = LeftBits & Mask;
            const auto Right = RightBits & Mask;
            const auto Sign = std::uint64_t{1} << (Width - 1);
            const bool Equal = Left == Right;
            const bool SignedLess = ((Left ^ Right) & Sign) != 0 ? (Left & Sign) != 0 : Left < Right;
            switch (Relation)
            {
            case 0:
                return Equal;
            case 1:
                return !Equal;
            case 2:
                return SignedLess;
            case 3:
                return SignedLess || Equal;
            case 4:
                return !SignedLess && !Equal;
            case 5:
                return !SignedLess;
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

        void verifyComparison(std::uint16_t Operation, std::size_t Width, bool MemoryRHS, std::uint64_t LeftBits, std::uint64_t RightBits, bool Taken)
        {
            std::array<std::uint8_t, 24> LeftMemory;
            std::array<std::uint8_t, 24> RightMemory;
            LeftMemory.fill(0xA5);
            RightMemory.fill(0x5A);
            std::memcpy(LeftMemory.data() + 3, &LeftBits, sizeof(LeftBits));
            std::memcpy(RightMemory.data() + 5, &RightBits, sizeof(RightBits));
            const auto ExpectedLeft = LeftMemory;
            const auto ExpectedRight = RightMemory;
            constexpr std::size_t BranchBytes = 32;
            constexpr std::size_t ReturnBytes = 8;
            ExecutableFile Executable;
            auto& Bytes = Executable.getImage();
            appendHeader(Bytes, 0x3A);
            appendHeader(Bytes, Operation, RegisterId::PC);
            appendInteger(Bytes, reinterpret_cast<std::uintptr_t>(LeftMemory.data() + 3), 8);
            if (MemoryRHS)
            {
                appendInteger(Bytes, reinterpret_cast<std::uintptr_t>(RightMemory.data() + 5), 8);
            }
            else
            {
                const auto Mask = Width == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << Width) - 1;
                appendInteger(Bytes, RightBits & Mask, 8);
            }
            appendInteger(Bytes, BranchBytes + ReturnBytes, 8);
            appendHeader(Bytes, 0x3A);
            appendHeader(Bytes, 0x3A);
            const auto ExpectedCode = Bytes;
            Interpreter VM;
            ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
            const auto* Entry = VM.getCodeSegment().data();
            ASSERT_TRUE(VM.execute(Entry));
            const auto Bindings = VM.getFrame().Register;
            const auto StackAddress = storedRegister(VM.getFrame(), RegisterId::SP);
            ASSERT_NE(StackAddress, 0U);
            auto* const Stack = reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(StackAddress));
            std::array<std::uint8_t, 64> ExpectedStack;
            ExpectedStack.fill(0xA7);
            std::memcpy(Stack, ExpectedStack.data(), ExpectedStack.size());

            ASSERT_TRUE(VM.execute(Entry + ReturnBytes));

            EXPECT_EQ(storedRegister(VM.getFrame(), RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + ReturnBytes + BranchBytes + (Taken ? ReturnBytes : 0)));
            EXPECT_EQ(storedRegister(VM.getFrame(), RegisterId::BP), StackAddress);
            EXPECT_EQ(storedRegister(VM.getFrame(), RegisterId::SP), StackAddress);
            EXPECT_EQ(VM.getFrame().Register, Bindings);
            EXPECT_EQ(LeftMemory, ExpectedLeft);
            EXPECT_EQ(RightMemory, ExpectedRight);
            EXPECT_TRUE(std::ranges::equal(std::span<const std::uint8_t>(Stack, ExpectedStack.size()), ExpectedStack));
            EXPECT_TRUE(std::ranges::equal(VM.getCodeSegment(), ExpectedCode));
        }
    } // namespace

    // All eighty encodings take and skip branches using an independent bit-order reference, including signed boundaries, equality, truncation, and unaligned memory.
    TEST(VMCompareBranchTest, ExecutesEveryRelationWidthAndFormAtBoundaryValues)
    {
        core::test::ScopedEnvironmentVariable Environment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(Environment.set("64"));
        constexpr std::size_t Widths[] = {
            8,
            16,
            32,
            64,
        };
        for (std::size_t Relation = 0; Relation < 10; ++Relation)
        {
            for (std::size_t WidthIndex = 0; WidthIndex < std::size(Widths); ++WidthIndex)
            {
                const auto Width = Widths[WidthIndex];
                const auto Mask = Width == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << Width) - 1;
                const auto Sign = std::uint64_t{1} << (Width - 1);
                const std::array<std::uint64_t, 2> Inputs[] = {
                    {0, 0},
                    {0, 1},
                    {1, 0},
                    {Mask, 0},
                    {0, Mask},
                    {Sign, Sign - 1},
                    {Sign - 1, Sign},
                    {Mask, Mask},
                    {~Mask | 1, 1},
                    {1, ~Mask | 1},
                };
                for (std::size_t Form = 0; Form < 2; ++Form)
                {
                    const auto Operation = static_cast<std::uint16_t>(0x42 + Relation * 8 + WidthIndex * 2 + Form);
                    SCOPED_TRACE(Operation);
                    bool SawTaken = false;
                    bool SawFallthrough = false;
                    for (const auto& Input : Inputs)
                    {
                        SCOPED_TRACE(Input[0]);
                        SCOPED_TRACE(Input[1]);
                        const bool Taken = expectedComparison(Relation, Width, Input[0], Input[1]);
                        SawTaken |= Taken;
                        SawFallthrough |= !Taken;
                        verifyComparison(Operation, Width, Form == 0, Input[0], Input[1], Taken);
                    }
                    EXPECT_TRUE(SawTaken);
                    EXPECT_TRUE(SawFallthrough);
                }
            }
        }
    }

    // Emitted memory/memory and memory/immediate branches jump forward into a finite loop, branch backward until its counter reaches three, then fall through to RET.
    TEST(VMCompareBranchTest, EmitsAndExecutesForwardAndBackwardComparisonBranches)
    {
        auto& Names = core::NamePool::instance();
        const auto FunctionName = Names.intern("compare-branch-dispatch-loop");
        const auto CounterName = Names.intern("compare-branch-dispatch-counter");
        const auto LimitName = Names.intern("compare-branch-dispatch-limit");
        const std::array<std::uint8_t, 8> LimitBytes{3};
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(8, 8, {}, CounterName).has_value());
        ASSERT_TRUE(Module.createGlobalVariable(8, 8, LimitBytes, LimitName).has_value());
        auto& Function = Module.createFunction(FunctionName);
        auto& Entry = Function.createBasicBlock();
        auto& Wrong = Function.createBasicBlock();
        auto& Loop = Function.createBasicBlock();
        auto& Exit = Function.createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Entry);
        Builder.create<Opcode::Add64mrr>(AddressOperand(CounterName), RegisterOperand(RegisterId::ZERO), RegisterOperand(RegisterId::ZERO));
        Builder.create<Opcode::Jltu64mmb>(AddressOperand(CounterName), AddressOperand(LimitName), BasicBlockOperand(&Loop));
        Builder.setInsertPoint(Wrong);
        Builder.create<Opcode::Add64rrr>(RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO, -1), RegisterOperand(RegisterId::ZERO));
        Builder.create<Opcode::Ret>();
        Builder.setInsertPoint(Loop);
        Builder.create<Opcode::Add64rrm>(RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO), AddressOperand(CounterName));
        Builder.create<Opcode::Add64mrr>(AddressOperand(CounterName), RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO, 1));
        Builder.create<Opcode::Jltu64mib>(AddressOperand(CounterName), ImmediateOperand(3), BasicBlockOperand(&Loop));
        Builder.setInsertPoint(Exit);
        Builder.create<Opcode::Add64rrm>(RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO), AddressOperand(CounterName));
        Builder.create<Opcode::Ret>();
        Interpreter VM;
        ExecutableEmitter Emitter(VM.getDiagnostics());
        auto Executable = Emitter.emit(Module, FunctionName, {});
        const auto EntryOffset = Executable.getEntryOffset();
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto Bindings = VM.getFrame().Register;

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + EntryOffset));

        std::uint64_t Counter;
        std::uint64_t Limit;
        std::memcpy(&Counter, VM.getCodeSegment().data(), sizeof(Counter));
        std::memcpy(&Limit, VM.getCodeSegment().data() + 8, sizeof(Limit));
        EXPECT_EQ(Counter, 3U);
        EXPECT_EQ(Limit, 3U);
        EXPECT_EQ(storedRegister(VM.getFrame(), RegisterId::BP), 3U);
        EXPECT_EQ(VM.getFrame().Register, Bindings);
    }
} // namespace ink::vm::test
