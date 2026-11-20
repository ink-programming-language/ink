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
            Opcode::Fneg64rr,
#define FLOAT_COMPARE(Kind, Encoding, Spelling, Expression) Opcode::Kind,
#include "ink/backend/vm/instruction/float_compare.def"
#undef FLOAT_COMPARE
            Opcode::Sitof64rr,
            Opcode::Uitof64rr,
            Opcode::F64tosi64rr,
            Opcode::F64toui64rr,
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
        constexpr std::array FloatingPointContractOperations = {
            Opcode::Fadd64rrr,
            Opcode::Fsqrt64rr,
            Opcode::Fneg64rr,
            Opcode::Fcmp64oeqrrr,
            Opcode::F64tosi64rr,
        };

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

    // ZERO supplies positive floating zero or integer zero, and all FP64 operations can read it without writable ZERO storage.
    TEST(VMFloatingPointTest, AcceptsZeroAsFloatingPointSource)
    {
        const auto ExpectedForZero = [](Opcode Operation) -> std::uint64_t
        {
            switch (Operation)
            {
            case Opcode::Fneg64rr:
                return std::uint64_t{1} << 63;
            case Opcode::Fcmp64oeqrrr:
            case Opcode::Fcmp64olerrr:
            case Opcode::Fcmp64ogerrr:
            case Opcode::Fcmp64ordrrr:
            case Opcode::Fcmp64ueqrrr:
            case Opcode::Fcmp64ulerrr:
            case Opcode::Fcmp64ugerrr:
                return 1U;
            default:
                return 0U;
            }
        };
        for (const auto Operation : FloatingPointOperations)
        {
            SCOPED_TRACE(static_cast<unsigned>(Operation));
            Interpreter VM;
            const bool Division = Operation == Opcode::Fdiv64rrr;
            auto Executable = makeFloatingPointExecutable(Operation, std::bit_cast<std::uint64_t>(4.0), std::bit_cast<std::uint64_t>(2.0), {RegisterId::R9, RegisterId::ZERO, Division ? RegisterId::R1 : RegisterId::ZERO});
            ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

            ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));

            EXPECT_EQ(READ_REGISTER(RegisterId::R9, VM.getFrame()), ExpectedForZero(Operation));
            EXPECT_EQ(READ_REGISTER(RegisterId::ZERO, VM.getFrame()), 0U);
        }
    }

    // fneg64rr flips only the IEEE 754 sign bit across finite values, signed zeros, infinities, and NaN payloads.
    TEST(VMFloatingPointTest, NegatesBinary64BitPatternsPreservingSignedZeroAndNaNPayload)
    {
        constexpr std::uint64_t NaNPayload = 0x7FF8000000001234ULL;
        constexpr std::uint64_t Inputs[] = {
            std::bit_cast<std::uint64_t>(0.0),
            std::bit_cast<std::uint64_t>(-0.0),
            std::bit_cast<std::uint64_t>(3.5),
            std::bit_cast<std::uint64_t>(-3.5),
            std::bit_cast<std::uint64_t>(std::numeric_limits<double>::infinity()),
            std::bit_cast<std::uint64_t>(-std::numeric_limits<double>::infinity()),
            NaNPayload,
            NaNPayload | (std::uint64_t{1} << 63),
        };
        for (const auto Input : Inputs)
        {
            SCOPED_TRACE(Input);
            Interpreter VM;
            ASSERT_NE(VM.loadLibrary(makeFloatingPointExecutable(Opcode::Fneg64rr, Input, 0, {RegisterId::R0, RegisterId::R0, RegisterId::ZERO})), Interpreter::InvalidLibraryHandle);

            ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));

            EXPECT_EQ(READ_REGISTER(RegisterId::R0, VM.getFrame()), Input ^ (std::uint64_t{1} << 63));
        }
    }

    // All fourteen FP64 comparison predicates distinguish ordered and unordered relations for finite values, signed zeros, infinities, and NaNs.
    TEST(VMFloatingPointTest, EvaluatesAllOrderedAndUnorderedFloatingPointComparisons)
    {
        constexpr auto Inf = std::numeric_limits<double>::infinity();
        constexpr auto NaN = std::numeric_limits<double>::quiet_NaN();
        constexpr std::array<double, 2> Pairs[] = {
            {1.5, 1.5},
            {1.5, 2.5},
            {2.5, 1.5},
            {0.0, -0.0},
            {-Inf, Inf},
            {NaN, 1.0},
            {1.0, NaN},
            {NaN, NaN},
        };
        for (const auto& Pair : Pairs)
        {
            const double Left = Pair[0];
            const double Right = Pair[1];
            const bool Unordered = std::isnan(Left) || std::isnan(Right);
#define FLOAT_COMPARE(Kind, Encoding, Spelling, Expression)                                                                                                                                                                                 \
    {                                                                                                                                                                                                                                       \
        SCOPED_TRACE(Spelling);                                                                                                                                                                                                             \
        Interpreter VM;                                                                                                                                                                                                                     \
        ASSERT_NE(VM.loadLibrary(makeFloatingPointExecutable(Opcode::Kind, std::bit_cast<std::uint64_t>(Left), std::bit_cast<std::uint64_t>(Right), {RegisterId::R0, RegisterId::R0, RegisterId::R1})), Interpreter::InvalidLibraryHandle); \
        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));                                                                                                                                                                                \
        EXPECT_EQ(READ_REGISTER(RegisterId::R0, VM.getFrame()), static_cast<std::uint64_t>(Expression));                                                                                                                                    \
    }
#include "ink/backend/vm/instruction/float_compare.def"
#undef FLOAT_COMPARE
        }
    }

    // Integer-to-FP64 and saturating FP64-to-integer conversions handle signed, unsigned, zero, infinity, NaN, and out-of-range boundaries without host UB.
    TEST(VMFloatingPointTest, ConvertsBetween64BitIntegersAndBinary64WithSaturation)
    {
        constexpr auto SignedMin = std::numeric_limits<std::int64_t>::min();
        constexpr auto SignedMax = std::numeric_limits<std::int64_t>::max();
        constexpr auto UnsignedMax = std::numeric_limits<std::uint64_t>::max();
        constexpr auto Inf = std::numeric_limits<double>::infinity();
        constexpr auto NaN = std::numeric_limits<double>::quiet_NaN();

        const auto RunUnary = [](Opcode Operation, std::uint64_t Input) -> std::uint64_t
        {
            Interpreter VM;
            auto Executable = makeFloatingPointExecutable(Operation, Input, 0, {RegisterId::R0, RegisterId::R0, RegisterId::ZERO});
            EXPECT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
            EXPECT_TRUE(VM.execute(VM.getCodeSegment().data()));
            return READ_REGISTER(RegisterId::R0, VM.getFrame());
        };

        EXPECT_EQ(RunUnary(Opcode::Sitof64rr, 0), std::bit_cast<std::uint64_t>(0.0));
        EXPECT_EQ(RunUnary(Opcode::Sitof64rr, static_cast<std::uint64_t>(-42)), std::bit_cast<std::uint64_t>(-42.0));
        EXPECT_EQ(RunUnary(Opcode::Sitof64rr, static_cast<std::uint64_t>(SignedMin)), std::bit_cast<std::uint64_t>(static_cast<double>(SignedMin)));
        EXPECT_EQ(RunUnary(Opcode::Sitof64rr, static_cast<std::uint64_t>(SignedMax)), std::bit_cast<std::uint64_t>(static_cast<double>(SignedMax)));

        EXPECT_EQ(RunUnary(Opcode::Uitof64rr, 0), std::bit_cast<std::uint64_t>(0.0));
        EXPECT_EQ(RunUnary(Opcode::Uitof64rr, 42), std::bit_cast<std::uint64_t>(42.0));
        EXPECT_EQ(RunUnary(Opcode::Uitof64rr, UnsignedMax), std::bit_cast<std::uint64_t>(static_cast<double>(UnsignedMax)));

        EXPECT_EQ(RunUnary(Opcode::F64tosi64rr, std::bit_cast<std::uint64_t>(3.9)), 3U);
        EXPECT_EQ(RunUnary(Opcode::F64tosi64rr, std::bit_cast<std::uint64_t>(-3.9)), static_cast<std::uint64_t>(-3));
        EXPECT_EQ(RunUnary(Opcode::F64tosi64rr, std::bit_cast<std::uint64_t>(-0.0)), 0U);
        EXPECT_EQ(RunUnary(Opcode::F64tosi64rr, std::bit_cast<std::uint64_t>(NaN)), 0U);
        EXPECT_EQ(RunUnary(Opcode::F64tosi64rr, std::bit_cast<std::uint64_t>(0x1p63)), static_cast<std::uint64_t>(SignedMax));
        EXPECT_EQ(RunUnary(Opcode::F64tosi64rr, std::bit_cast<std::uint64_t>(Inf)), static_cast<std::uint64_t>(SignedMax));
        EXPECT_EQ(RunUnary(Opcode::F64tosi64rr, std::bit_cast<std::uint64_t>(-0x1p63)), static_cast<std::uint64_t>(SignedMin));
        EXPECT_EQ(RunUnary(Opcode::F64tosi64rr, std::bit_cast<std::uint64_t>(-Inf)), static_cast<std::uint64_t>(SignedMin));

        EXPECT_EQ(RunUnary(Opcode::F64toui64rr, std::bit_cast<std::uint64_t>(3.9)), 3U);
        EXPECT_EQ(RunUnary(Opcode::F64toui64rr, std::bit_cast<std::uint64_t>(-0.0)), 0U);
        EXPECT_EQ(RunUnary(Opcode::F64toui64rr, std::bit_cast<std::uint64_t>(-1.5)), 0U);
        EXPECT_EQ(RunUnary(Opcode::F64toui64rr, std::bit_cast<std::uint64_t>(-Inf)), 0U);
        EXPECT_EQ(RunUnary(Opcode::F64toui64rr, std::bit_cast<std::uint64_t>(NaN)), 0U);
        EXPECT_EQ(RunUnary(Opcode::F64toui64rr, std::bit_cast<std::uint64_t>(0x1p63)), std::uint64_t{1} << 63);
        EXPECT_EQ(RunUnary(Opcode::F64toui64rr, std::bit_cast<std::uint64_t>(0x1p64)), UnsignedMax);
        EXPECT_EQ(RunUnary(Opcode::F64toui64rr, std::bit_cast<std::uint64_t>(Inf)), UnsignedMax);
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
    // Assertion-enabled dispatch classifies every floating-point opcode uniformly and rejects positive and negative displacements at every explicit operand position.
    TEST(VMFloatingPointTest, ChecksNonzeroDisplacementsAtEveryOperandPosition)
    {
        for (const auto Operation : FloatingPointOperations)
        {
            EXPECT_TRUE(InstructionEncoding::isFloatingPoint(Operation));
        }
        for (const auto Operation : FloatingPointContractOperations)
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

    // Assertion-enabled dispatch rejects control registers, immutable destinations, and undefined floating register IDs across binary, unary, compare, and conversion forms.
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
        for (const auto Operation : {Opcode::Fadd64rrr, Opcode::Fsqrt64rr})
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
        for (const auto Operation : {Opcode::Fneg64rr, Opcode::Fcmp64oeqrrr, Opcode::Sitof64rr, Opcode::F64tosi64rr})
        {
            SCOPED_TRACE(static_cast<unsigned>(Operation));
            expectFloatingPointContractFailure(Operation, {RegisterId::ZERO, RegisterId::R0, RegisterId::R1}, {});
            expectFloatingPointContractFailure(Operation, {RegisterId::R9, RegisterId::SP, RegisterId::R1}, {});
        }
    }
#endif
} // namespace ink::vm::test
