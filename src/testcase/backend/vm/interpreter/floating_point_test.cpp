#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/interpreter/interpreter.h"

#include <gtest/gtest.h>

#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

namespace ink::vm::test
{
    namespace
    {
        constexpr std::array FloatingPointOperations = {
            Opcode::Fadd64rrr,
            Opcode::Fsub64rrr,
            Opcode::Fmul64rrr,
            Opcode::Fdiv64rrr,
            Opcode::Fsqrt64rr,
        };

        constexpr std::uint64_t DestinationSentinel = 0x7FF8000000000099ULL;

        void appendWord(ExecutableFile& Executable, std::uint64_t Value)
        {
            for (std::size_t Index = 0; Index < sizeof(Value); ++Index)
            {
                Executable.getImage().push_back(static_cast<std::uint8_t>(Value >> (Index * 8)));
            }
        }

        void appendRegisterConstant(ExecutableFile& Executable, RegisterId Register, std::uint64_t Bits)
        {
            auto Header = InstructionEncoding::encodeOpcode(Opcode::Add64rri);
            Header = InstructionEncoding::encodeRegister(Header, 0, Register);
            appendWord(Executable, Header);
            appendWord(Executable, 0);
            appendWord(Executable, 0);
            appendWord(Executable, Bits);
        }

        ExecutableFile makeFloatingPointExecutable(Opcode Operation, std::uint64_t Left, std::uint64_t Right, std::array<RegisterId, 3> Registers = {RegisterId::R9, RegisterId::R0, RegisterId::R1}, std::array<std::uint64_t, 3> Offsets = {})
        {
            ExecutableFile Executable;
            appendRegisterConstant(Executable, RegisterId::R0, Left);
            appendRegisterConstant(Executable, RegisterId::R1, Right);
            appendRegisterConstant(Executable, RegisterId::R9, DestinationSentinel);
            auto Header = InstructionEncoding::encodeOpcode(Operation);
            const auto OperandCount = InstructionEncoding::operandCount(Operation);
            for (std::size_t Index = 0; Index < OperandCount; ++Index)
            {
                Header = InstructionEncoding::encodeRegister(Header, Index, Registers[Index]);
            }
            appendWord(Executable, Header);
            for (std::size_t Index = 0; Index < OperandCount; ++Index)
            {
                appendWord(Executable, Offsets[Index]);
            }
            appendWord(Executable, InstructionEncoding::encodeOpcode(Opcode::Ret));
            return Executable;
        }

#if INK_ENABLE_ASSERTIONS
        void expectFloatingPointContractFailure(Opcode Operation, std::array<RegisterId, 3> Registers, std::array<std::uint64_t, 3> Offsets)
        {
            constexpr auto Left = std::bit_cast<std::uint64_t>(4.0);
            constexpr auto Right = std::bit_cast<std::uint64_t>(2.0);
            Interpreter VM;
            ASSERT_NE(VM.loadLibrary(makeFloatingPointExecutable(Operation, Left, Right, Registers, Offsets)), Interpreter::InvalidLibraryHandle);
            EXPECT_DEATH(static_cast<void>(VM.execute(VM.getCodeSegment().data())), "check failed");
        }
#endif
    } // namespace

    // FP64 operations interpret register bits as doubles and read all inputs before writing any aliased destination.
    TEST(VMFloatingPointTest, ExecutesAllOperationsWithAliasedDestinations)
    {
        struct OperationCase
        {
                Opcode Operation;
                double Left;
                double Right;
                double Expected;
        };
        constexpr OperationCase Cases[] = {
            {Opcode::Fadd64rrr, 1.5, 2.25, 3.75},
            {Opcode::Fsub64rrr, 1.5, 2.25, -0.75},
            {Opcode::Fmul64rrr, -1.5, 2.25, -3.375},
            {Opcode::Fdiv64rrr, 7.5, 2.5, 3.0},
            {Opcode::Fsqrt64rr, 2.25, 0.0, 1.5},
        };
        for (const auto& Case : Cases)
        {
            for (const auto Destination : {RegisterId::R0, RegisterId::R1, RegisterId::R9})
            {
                SCOPED_TRACE(static_cast<unsigned>(Case.Operation));
                SCOPED_TRACE(static_cast<unsigned>(Destination));
                Interpreter VM;
                auto Executable = makeFloatingPointExecutable(Case.Operation, std::bit_cast<std::uint64_t>(Case.Left), std::bit_cast<std::uint64_t>(Case.Right), {Destination, RegisterId::R0, RegisterId::R1});
                ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

                ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));

                EXPECT_EQ(READ_REGISTER(Destination, VM.getFrame()), std::bit_cast<std::uint64_t>(Case.Expected));
            }
        }
    }

    // ZERO supplies positive floating zero, and all five operations can read it without writable ZERO storage.
    TEST(VMFloatingPointTest, AcceptsZeroAsFloatingPointSource)
    {
        for (const auto Operation : FloatingPointOperations)
        {
            SCOPED_TRACE(static_cast<unsigned>(Operation));
            Interpreter VM;
            const bool Division = Operation == Opcode::Fdiv64rrr;
            auto Executable = makeFloatingPointExecutable(Operation, std::bit_cast<std::uint64_t>(4.0), std::bit_cast<std::uint64_t>(2.0), {RegisterId::R9, RegisterId::ZERO, Division ? RegisterId::R1 : RegisterId::ZERO});
            ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

            ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));

            EXPECT_EQ(READ_REGISTER(RegisterId::R9, VM.getFrame()), 0U);
            EXPECT_EQ(READ_REGISTER(RegisterId::ZERO, VM.getFrame()), 0U);
        }
    }

    // FP64 exceptional values remain numeric results: division by zero, invalid roots, signed zeros, and NaN propagation do not fail execution.
    TEST(VMFloatingPointTest, PreservesInfinityNaNAndSignedZeroSemantics)
    {
        struct SpecialCase
        {
                Opcode Operation;
                double Left;
                double Right;
                double Expected;
        };
        constexpr auto Infinity = std::numeric_limits<double>::infinity();
        constexpr auto NaN = std::numeric_limits<double>::quiet_NaN();
        constexpr SpecialCase Cases[] = {
            {Opcode::Fadd64rrr, Infinity, -Infinity, NaN},
            {Opcode::Fsub64rrr, Infinity, Infinity, NaN},
            {Opcode::Fmul64rrr, 0.0, Infinity, NaN},
            {Opcode::Fdiv64rrr, 1.0, 0.0, Infinity},
            {Opcode::Fdiv64rrr, 1.0, -0.0, -Infinity},
            {Opcode::Fdiv64rrr, 0.0, 0.0, NaN},
            {Opcode::Fsqrt64rr, -1.0, 0.0, NaN},
            {Opcode::Fsqrt64rr, Infinity, 0.0, Infinity},
            {Opcode::Fsqrt64rr, -0.0, 0.0, -0.0},
            {Opcode::Fadd64rrr, -0.0, -0.0, -0.0},
            {Opcode::Fsub64rrr, -0.0, 0.0, -0.0},
            {Opcode::Fmul64rrr, -0.0, 2.0, -0.0},
            {Opcode::Fdiv64rrr, -0.0, 2.0, -0.0},
            {Opcode::Fadd64rrr, NaN, 1.0, NaN},
            {Opcode::Fsub64rrr, NaN, 1.0, NaN},
            {Opcode::Fmul64rrr, NaN, 1.0, NaN},
            {Opcode::Fdiv64rrr, NaN, 1.0, NaN},
            {Opcode::Fsqrt64rr, NaN, 0.0, NaN},
        };
        for (const auto& Case : Cases)
        {
            SCOPED_TRACE(static_cast<unsigned>(Case.Operation));
            Interpreter VM;
            ASSERT_NE(VM.loadLibrary(makeFloatingPointExecutable(Case.Operation, std::bit_cast<std::uint64_t>(Case.Left), std::bit_cast<std::uint64_t>(Case.Right))), Interpreter::InvalidLibraryHandle);

            ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));

            const auto Bits = READ_REGISTER(RegisterId::R9, VM.getFrame());
            if (std::isnan(Case.Expected))
            {
                EXPECT_TRUE(std::isnan(std::bit_cast<double>(Bits)));
            }
            else
            {
                EXPECT_EQ(Bits, std::bit_cast<std::uint64_t>(Case.Expected));
            }
        }
    }

#if INK_ENABLE_ASSERTIONS
    // Assertion-enabled dispatch rejects positive and negative displacements at every explicit floating register position.
    TEST(VMFloatingPointTest, ChecksNonzeroDisplacementsAtEveryOperandPosition)
    {
        for (const auto Operation : FloatingPointOperations)
        {
            const auto OperandCount = InstructionEncoding::operandCount(Operation);
            for (std::size_t Index = 0; Index < OperandCount; ++Index)
            {
                for (const auto Offset : {std::uint64_t{1}, std::numeric_limits<std::uint64_t>::max()})
                {
                    SCOPED_TRACE(static_cast<unsigned>(Operation));
                    SCOPED_TRACE(Index);
                    SCOPED_TRACE(Offset);
                    std::array<std::uint64_t, 3> Offsets{};
                    Offsets[Index] = Offset;
                    expectFloatingPointContractFailure(Operation, {RegisterId::R9, RegisterId::R0, RegisterId::R1}, Offsets);
                }
            }
        }
    }

    // Assertion-enabled dispatch rejects control registers, immutable destinations, and undefined floating register IDs.
    TEST(VMFloatingPointTest, ChecksInvalidFloatingPointRegisters)
    {
        constexpr RegisterId InvalidDestinations[] = {
            RegisterId::BP,
            RegisterId::SP,
            RegisterId::PC,
            RegisterId::ZERO,
            static_cast<RegisterId>(0x0E),
            static_cast<RegisterId>(0xFF),
        };
        constexpr RegisterId InvalidSources[] = {
            RegisterId::BP,
            RegisterId::SP,
            RegisterId::PC,
            static_cast<RegisterId>(0x0E),
            static_cast<RegisterId>(0xFF),
        };
        for (const auto Operation : FloatingPointOperations)
        {
            SCOPED_TRACE(static_cast<unsigned>(Operation));
            for (const auto Destination : InvalidDestinations)
            {
                SCOPED_TRACE(static_cast<unsigned>(Destination));
                expectFloatingPointContractFailure(Operation, {Destination, RegisterId::R0, RegisterId::R1}, {});
            }
            for (std::size_t Index = 1; Index < InstructionEncoding::operandCount(Operation); ++Index)
            {
                for (const auto Source : InvalidSources)
                {
                    SCOPED_TRACE(Index);
                    SCOPED_TRACE(static_cast<unsigned>(Source));
                    std::array Registers = {RegisterId::R9, RegisterId::R0, RegisterId::R1};
                    Registers[Index] = Source;
                    expectFloatingPointContractFailure(Operation, Registers, {});
                }
            }
        }
    }
#endif
} // namespace ink::vm::test
