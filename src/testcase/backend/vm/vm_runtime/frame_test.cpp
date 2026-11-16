#include "ink/backend/vm/vm_runtime/frame.h"
#include "ink/backend/vm/instruction/register.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>
#include <utility>

namespace ink::vm::test
{
    namespace
    {
        struct TwelveByteValue
        {
                std::uint32_t First;
                std::uint32_t Second;
                std::uint32_t Third;

                constexpr bool operator==(const TwelveByteValue&) const noexcept = default;
        };

        struct TwentyFourByteValue
        {
                std::uint64_t First;
                std::uint64_t Second;
                std::uint64_t Third;

                constexpr bool operator==(const TwentyFourByteValue&) const noexcept = default;
        };

        struct NonDefaultConstructibleValue
        {
                explicit constexpr NonDefaultConstructibleValue(std::uint64_t Value) noexcept
                    : Value(Value)
                {
                }

                std::uint64_t Value;
        };

        template <typename FrameType>
        concept HasPublicSP = requires(FrameType& Value) {
            Value.SP;
        };

        template <typename FrameType>
        concept HasPublicBP = requires(FrameType& Value) {
            Value.BP;
        };

        template <typename FrameType>
        concept HasPublicPC = requires(FrameType& Value) {
            Value.PC;
        };

        template <typename FrameType>
        concept HasPublicZERO = requires(FrameType& Value) {
            Value.ZERO;
        };

        template <typename FrameType>
        concept CanWriteRegister = requires(FrameType& Value) {
            WRITE_REGISTER(RegisterId::SP, Value, std::uint64_t{0});
        };

        static_assert(!HasPublicSP<Frame> && !HasPublicBP<Frame> && !HasPublicPC<Frame> && !HasPublicZERO<Frame>);
        static_assert(sizeof(TwelveByteValue) == 12 && alignof(TwelveByteValue) <= 8);
        static_assert(sizeof(TwentyFourByteValue) == 24 && alignof(TwentyFourByteValue) <= 8);
        static_assert(sizeof(NonDefaultConstructibleValue) == 8 && alignof(NonDefaultConstructibleValue) <= 8);
        static_assert(std::is_trivially_copyable_v<NonDefaultConstructibleValue> && !std::is_default_constructible_v<NonDefaultConstructibleValue>);
        static_assert(sizeof(const std::uint64_t*) == 8);
        static_assert(std::is_same_v<decltype(GET_REGISTER(RegisterId::SP, std::declval<const Frame&>())), const std::uint8_t*>);
        static_assert(std::is_same_v<decltype(SET_REGISTER(RegisterId::SP, std::declval<Frame&>())), std::uint8_t*>);
        static_assert(!std::is_assignable_v<decltype((GET_REGISTER(RegisterId::ZERO, std::declval<Frame&>()))), const std::uint8_t*>);
        static_assert(std::is_same_v<decltype(READ_REGISTER(RegisterId::SP, std::declval<const Frame&>())), std::uint64_t>);
        static_assert(!std::is_lvalue_reference_v<decltype((READ_REGISTER(RegisterId::SP, std::declval<const Frame&>())))>);
        static_assert(!std::is_assignable_v<decltype((READ_REGISTER(RegisterId::SP, std::declval<const Frame&>()))), std::uint64_t>);
        static_assert(std::is_same_v<decltype(WRITE_REGISTER(RegisterId::SP, std::declval<Frame&>(), std::uint64_t{0})), void>);
        static_assert(CanWriteRegister<Frame> && !CanWriteRegister<const Frame>);

        std::uint8_t* readAddressRegister(const Frame& Value, RegisterId Id) noexcept
        {
            return reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(READ_REGISTER(Id, Value)));
        }

        void writeAddressRegister(Frame& Value, RegisterId Id, std::uint8_t* Address) noexcept
        {
            WRITE_REGISTER(Id, Value, reinterpret_cast<std::uintptr_t>(Address));
        }
    } // namespace

    // Typed register access preserves every bit and shares the same integer objects with the legacy byte-storage macros.
    TEST(VMFrameTest, ReadsAndWritesIntegerRegisterStorageWithoutChangingBindings)
    {
        Frame Value;
        const auto Bindings = Value.Register;
        constexpr std::uint64_t Bits = 0xFEDCBA9876543210;
        WRITE_REGISTER(RegisterId::BP, Value, Bits);
        WRITE_REGISTER(RegisterId::SP, Value, std::numeric_limits<std::uint64_t>::max());
        WRITE_REGISTER(RegisterId::PC, Value, 1);

        const Frame& View = Value;
        EXPECT_EQ(READ_REGISTER(RegisterId::BP, View), Bits);
        EXPECT_EQ(READ_REGISTER(RegisterId::SP, View), std::numeric_limits<std::uint64_t>::max());
        EXPECT_EQ(READ_REGISTER(RegisterId::PC, View), 1U);
        EXPECT_EQ(READ_REGISTER(RegisterId::ZERO, View), 0U);
        EXPECT_EQ(*reinterpret_cast<const std::uint64_t*>(GET_REGISTER(RegisterId::BP, View)), Bits);
        *reinterpret_cast<std::uint64_t*>(SET_REGISTER(RegisterId::SP, Value)) = Bits;
        EXPECT_EQ(READ_REGISTER(RegisterId::SP, View), Bits);
        EXPECT_EQ(Value.Register, Bindings);
    }

    // Direct PC access shares the live register object with typed and byte-storage aliases, including after independent frame copies.
    TEST(VMFrameTest, KeepsDirectPCAccessCoherentWithRegisterAliasesAndCopies)
    {
        Frame Value;
        const Frame& View = Value;
        const auto Bindings = Value.Register;
        constexpr std::uint64_t Bits = 0xFEDCBA9876543210;
        Value.setPC(std::numeric_limits<std::uint64_t>::max());
        EXPECT_EQ(READ_REGISTER(RegisterId::PC, View), std::numeric_limits<std::uint64_t>::max());
        EXPECT_EQ(*reinterpret_cast<const std::uint64_t*>(GET_REGISTER(RegisterId::PC, View)), std::numeric_limits<std::uint64_t>::max());

        WRITE_REGISTER(RegisterId::PC, Value, Bits);
        EXPECT_EQ(View.getPC(), Bits);
        constexpr std::uint64_t AliasBits = 0x0123456789ABCDEF;
        std::memcpy(SET_REGISTER(RegisterId::PC, Value), &AliasBits, sizeof(AliasBits));
        EXPECT_EQ(View.getPC(), AliasBits);
        EXPECT_EQ(Value.Register, Bindings);

        Frame Copied = Value;
        Frame Assigned;
        Assigned = Value;
        EXPECT_NE(GET_REGISTER(RegisterId::PC, Copied), GET_REGISTER(RegisterId::PC, View));
        EXPECT_NE(GET_REGISTER(RegisterId::PC, Assigned), GET_REGISTER(RegisterId::PC, View));
        Copied.setPC(17);
        WRITE_REGISTER(RegisterId::PC, Assigned, 23);
        Value.setPC(42);
        EXPECT_EQ(READ_REGISTER(RegisterId::PC, Copied), 17U);
        EXPECT_EQ(Assigned.getPC(), 23U);
        EXPECT_EQ(View.getPC(), 42U);
        EXPECT_EQ(Value.Register, Bindings);
    }

    // Value macros accept a const read view and evaluate every register, frame, and written-value expression once.
    TEST(VMFrameTest, EvaluatesRegisterValueMacroArgumentsOnce)
    {
        Frame Value;
        WRITE_REGISTER(RegisterId::SP, Value, 17);
        std::size_t IdEvaluations = 0;
        std::size_t ReadFrameEvaluations = 0;
        std::size_t WriteFrameEvaluations = 0;
        std::size_t ValueEvaluations = 0;
        const auto SelectId = [&]()
        {
            ++IdEvaluations;
            return RegisterId::SP;
        };
        const auto SelectReadFrame = [&]() -> const Frame&
        {
            ++ReadFrameEvaluations;
            return Value;
        };
        const auto SelectWriteFrame = [&]() -> Frame&
        {
            ++WriteFrameEvaluations;
            return Value;
        };
        const auto SelectValue = [&]()
        {
            ++ValueEvaluations;
            return std::uint64_t{42};
        };

        EXPECT_EQ(READ_REGISTER(SelectId(), SelectReadFrame()), 17U);
        EXPECT_EQ(IdEvaluations, 1U);
        EXPECT_EQ(ReadFrameEvaluations, 1U);
        WRITE_REGISTER(SelectId(), SelectWriteFrame(), SelectValue());
        EXPECT_EQ(IdEvaluations, 2U);
        EXPECT_EQ(WriteFrameEvaluations, 1U);
        EXPECT_EQ(ValueEvaluations, 1U);
        EXPECT_EQ(READ_REGISTER(RegisterId::SP, static_cast<const Frame&>(Value)), 42U);
    }

#if INK_ENABLE_ASSERTIONS
    // Invalid register numbers and writes to the immutable ZERO register fail the typed access contract.
    TEST(VMFrameTest, RejectsInvalidRegisterAccessAndZeroWrites)
    {
        Frame Value;
        const auto Invalid = static_cast<RegisterId>(0xFF);
        EXPECT_DEATH(static_cast<void>(READ_REGISTER(Invalid, Value)), "check failed");
        EXPECT_DEATH(WRITE_REGISTER(Invalid, Value, 1), "check failed");
        EXPECT_DEATH(WRITE_REGISTER(RegisterId::ZERO, Value, 1), "check failed");
        EXPECT_EQ(READ_REGISTER(RegisterId::ZERO, Value), 0U);
    }
#endif

    // Every slot points to register storage; copies own independent address storage while sharing the immutable ZERO object.
    TEST(VMFrameTest, InitializesRegisterStorageAndRebindsIndependentCopies)
    {
        constexpr RegisterId AddressRegisters[] = {
            RegisterId::BP,
            RegisterId::SP,
            RegisterId::PC,
        };
        Frame Initial;
        EXPECT_EQ(Initial.Register.size(), 4U);
        EXPECT_EQ(Initial.Register[3], GET_REGISTER(RegisterId::ZERO, Initial));
        for (const auto* Storage : Initial.Register)
        {
            ASSERT_NE(Storage, nullptr);
        }
        for (const auto Id : AddressRegisters)
        {
            ASSERT_NE(GET_REGISTER(Id, Initial), nullptr);
            EXPECT_EQ(readAddressRegister(Initial, Id), nullptr);
        }
        ASSERT_NE(GET_REGISTER(RegisterId::ZERO, Initial), nullptr);
        EXPECT_EQ(*reinterpret_cast<const std::uint64_t*>(GET_REGISTER(RegisterId::ZERO, Initial)), 0U);

        std::array<std::uint8_t, 16> Stack{};
        std::array<std::uint8_t, 8> Code{};
        Frame Original{Stack.data() + 12};
        const auto OriginalBindings = Original.Register;
        writeAddressRegister(Original, RegisterId::BP, Stack.data() + 4);
        writeAddressRegister(Original, RegisterId::PC, Code.data() + 1);
        EXPECT_EQ(readAddressRegister(Original, RegisterId::BP), Stack.data() + 4);
        EXPECT_EQ(readAddressRegister(Original, RegisterId::SP), Stack.data() + 12);
        EXPECT_EQ(readAddressRegister(Original, RegisterId::PC), Code.data() + 1);
        EXPECT_EQ(Original.Register, OriginalBindings);
        for (const auto Id : AddressRegisters)
        {
            EXPECT_EQ(SET_REGISTER(Id, Original), const_cast<std::uint8_t*>(GET_REGISTER(Id, Original)));
        }

        Frame Copied = Original;
        Frame Assigned;
        Assigned = Original;
        Frame RvalueCopied = std::move(Original);
        Frame RvalueAssigned;
        RvalueAssigned = std::move(Original);
        const std::array<Frame*, 4> Copies = {
            &Copied,
            &Assigned,
            &RvalueCopied,
            &RvalueAssigned,
        };
        for (auto* Value : Copies)
        {
            const auto CopyBindings = Value->Register;
            EXPECT_EQ(Value->Register.size(), 4U);
            EXPECT_EQ(Value->Register[3], GET_REGISTER(RegisterId::ZERO, Initial));
            for (const auto Id : AddressRegisters)
            {
                EXPECT_NE(GET_REGISTER(Id, *Value), GET_REGISTER(Id, Original));
                EXPECT_EQ(readAddressRegister(*Value, Id), readAddressRegister(Original, Id));
            }
            EXPECT_EQ(GET_REGISTER(RegisterId::ZERO, *Value), GET_REGISTER(RegisterId::ZERO, Initial));
            EXPECT_EQ(*reinterpret_cast<const std::uint64_t*>(GET_REGISTER(RegisterId::ZERO, *Value)), 0U);
            writeAddressRegister(*Value, RegisterId::BP, Stack.data() + 2);
            writeAddressRegister(*Value, RegisterId::SP, Stack.data() + 3);
            writeAddressRegister(*Value, RegisterId::PC, Code.data() + 2);
            EXPECT_EQ(Value->Register, CopyBindings);
            EXPECT_EQ(readAddressRegister(Original, RegisterId::BP), Stack.data() + 4);
            EXPECT_EQ(readAddressRegister(Original, RegisterId::SP), Stack.data() + 12);
            EXPECT_EQ(readAddressRegister(Original, RegisterId::PC), Code.data() + 1);
        }
        EXPECT_EQ(readAddressRegister(Copied, RegisterId::BP), Stack.data() + 2);
        EXPECT_EQ(readAddressRegister(Copied, RegisterId::SP), Stack.data() + 3);
        EXPECT_EQ(readAddressRegister(Copied, RegisterId::PC), Code.data() + 2);
        EXPECT_EQ(Stack, (std::array<std::uint8_t, 16>{}));
        EXPECT_EQ(Code, (std::array<std::uint8_t, 8>{}));

        std::size_t IdEvaluations = 0;
        std::size_t FrameEvaluations = 0;
        auto SelectId = [&]()
        {
            ++IdEvaluations;
            return RegisterId::SP;
        };
        auto SelectFrame = [&]() -> Frame&
        {
            ++FrameEvaluations;
            return Copied;
        };

        const auto ExpectedStorageAddress = GET_REGISTER(RegisterId::SP, Copied);
        const auto ReadAddress = GET_REGISTER(SelectId(), SelectFrame());
        EXPECT_EQ(ReadAddress, ExpectedStorageAddress);
        EXPECT_EQ(IdEvaluations, 1U);
        EXPECT_EQ(FrameEvaluations, 1U);
        const auto WriteAddress = SET_REGISTER(SelectId(), SelectFrame());
        EXPECT_EQ(WriteAddress, ExpectedStorageAddress);
        EXPECT_EQ(IdEvaluations, 2U);
        EXPECT_EQ(FrameEvaluations, 2U);
        const Frame& View = Copied;
        EXPECT_EQ(GET_REGISTER(RegisterId::SP, View), GET_REGISTER(RegisterId::SP, Copied));
        EXPECT_EQ(GET_REGISTER(RegisterId::ZERO, View), GET_REGISTER(RegisterId::ZERO, Initial));
    }

    // Writing the stored SP changes native argument and return locations without changing slot bindings, and copies retain independent register storage.
    TEST(VMFrameTest, UsesStoredRegisterValuesForNativeCallsAndCopies)
    {
        alignas(8) std::array<std::uint8_t, 48> Stack;
        Stack.fill(0xA7);
        const std::uint64_t Argument = 0x0123456789ABCDEF;
        std::memcpy(Stack.data() + 32, &Argument, sizeof(Argument));
        auto Expected = Stack;
        Frame CallFrame{Stack.data() + 16};
        const auto Bindings = CallFrame.Register;
        writeAddressRegister(CallFrame, RegisterId::SP, Stack.data() + 40);

        const auto Actual = CallFrame.getArgument<0, std::uint64_t>();
        EXPECT_EQ(Actual, Argument);
        Frame Copied = CallFrame;
        Frame Assigned;
        Assigned = CallFrame;
        for (auto* Value : {&Copied, &Assigned})
        {
            const auto CopiedArgument = Value->getArgument<0, std::uint64_t>();
            EXPECT_EQ(CopiedArgument, Argument);
            EXPECT_NE(GET_REGISTER(RegisterId::SP, *Value), GET_REGISTER(RegisterId::SP, CallFrame));
            EXPECT_EQ(readAddressRegister(*Value, RegisterId::SP), Stack.data() + 40);
            writeAddressRegister(*Value, RegisterId::SP, Stack.data() + 24);
            EXPECT_EQ(readAddressRegister(CallFrame, RegisterId::SP), Stack.data() + 40);
        }

        const std::uint64_t Result = 0xFEDCBA9876543210;
        std::memcpy(Expected.data() + 24, &Result, sizeof(Result));
        CallFrame.setReturnValue<std::uint64_t, std::uint64_t>(Result);
        EXPECT_EQ(Stack, Expected);
        EXPECT_EQ(readAddressRegister(CallFrame, RegisterId::SP), Stack.data() + 40);
        EXPECT_EQ(CallFrame.Register, Bindings);
    }

    // Mixed arguments and a one-byte result preserve the runtime registers, argument order, and all slot padding.
    TEST(VMFrameTest, ReadsMixedArgumentsAndWritesOnlySmallReturnValue)
    {
        alignas(8) std::array<std::uint8_t, 56> Stack;
        Stack.fill(0xA5);
        const std::int16_t Small = -1234;
        const TwelveByteValue Aggregate{0x01234567, 0x89ABCDEF, 0x76543210};
        const std::uint64_t Pointee = 0xFEDCBA9876543210;
        const std::uint64_t* const Pointer = &Pointee;
        std::array<std::uint8_t, 8> Code{};
        // The call area is return [8,16), small [16,24), aggregate [24,40), and pointer [40,48).
        std::memcpy(Stack.data() + 16, &Small, sizeof(Small));
        std::memcpy(Stack.data() + 24, &Aggregate, sizeof(Aggregate));
        std::memcpy(Stack.data() + 40, &Pointer, sizeof(Pointer));
        const auto InitialStack = Stack;
        Frame MutableCallFrame{Stack.data() + 48};
        writeAddressRegister(MutableCallFrame, RegisterId::BP, Stack.data() + 16);
        writeAddressRegister(MutableCallFrame, RegisterId::PC, Code.data());
        const Frame CallFrame = MutableCallFrame;
        auto* const InitialSP = readAddressRegister(CallFrame, RegisterId::SP);

        const auto ActualSmall = CallFrame.getArgument<0, std::int16_t, TwelveByteValue, const std::uint64_t*>();
        const auto ActualAggregate = CallFrame.getArgument<1, std::int16_t, TwelveByteValue, const std::uint64_t*>();
        const auto ActualPointer = CallFrame.getArgument<2, std::int16_t, TwelveByteValue, const std::uint64_t*>();

        EXPECT_EQ(ActualSmall, Small);
        EXPECT_EQ(ActualAggregate, Aggregate);
        EXPECT_EQ(ActualPointer, Pointer);
        EXPECT_EQ(Stack, InitialStack);
        EXPECT_EQ(readAddressRegister(CallFrame, RegisterId::SP), InitialSP);
        EXPECT_EQ(readAddressRegister(CallFrame, RegisterId::BP), Stack.data() + 16);
        EXPECT_EQ(readAddressRegister(CallFrame, RegisterId::PC), Code.data());
        EXPECT_EQ(*reinterpret_cast<const std::uint64_t*>(GET_REGISTER(RegisterId::ZERO, CallFrame)), 0U);

        const std::uint8_t Result = 0x3C;
        auto Expected = InitialStack;
        std::memcpy(Expected.data() + 8, &Result, sizeof(Result));
        CallFrame.setReturnValue<std::uint8_t, std::int16_t, TwelveByteValue, const std::uint64_t*>(Result);

        EXPECT_EQ(Stack, Expected);
        EXPECT_EQ(readAddressRegister(CallFrame, RegisterId::SP), InitialSP);
        EXPECT_EQ(readAddressRegister(CallFrame, RegisterId::BP), Stack.data() + 16);
        EXPECT_EQ(readAddressRegister(CallFrame, RegisterId::PC), Code.data());
        EXPECT_EQ(*reinterpret_cast<const std::uint64_t*>(GET_REGISTER(RegisterId::ZERO, CallFrame)), 0U);
    }

    // A twenty-four-byte argument spans three slots, while a twelve-byte result preserves its four padding bytes and all argument bytes.
    TEST(VMFrameTest, PreservesPaddingForAggregateArgumentsAndReturns)
    {
        alignas(8) std::array<std::uint8_t, 64> Stack;
        Stack.fill(0xD3);
        const TwentyFourByteValue Large{0x0123456789ABCDEF, 0xFEDCBA9876543210, 0x13579BDF2468ACE0};
        const std::int32_t Small = -234567;
        // The call area is return [8,24), large [24,48), and small [48,56).
        std::memcpy(Stack.data() + 24, &Large, sizeof(Large));
        std::memcpy(Stack.data() + 48, &Small, sizeof(Small));
        const auto InitialStack = Stack;
        Frame CallFrame{Stack.data() + 56};
        auto* const InitialSP = readAddressRegister(CallFrame, RegisterId::SP);

        const auto ActualLarge = CallFrame.getArgument<0, TwentyFourByteValue, std::int32_t>();
        const auto ActualSmall = CallFrame.getArgument<1, TwentyFourByteValue, std::int32_t>();

        EXPECT_EQ(ActualLarge, Large);
        EXPECT_EQ(ActualSmall, Small);
        EXPECT_EQ(Stack, InitialStack);

        const TwelveByteValue Result{0x10203040, 0x50607080, 0x90A0B0C0};
        auto Expected = InitialStack;
        std::memcpy(Expected.data() + 8, &Result, sizeof(Result));
        CallFrame.setReturnValue<TwelveByteValue, TwentyFourByteValue, std::int32_t>(Result);

        EXPECT_EQ(Stack, Expected);
        EXPECT_EQ(readAddressRegister(CallFrame, RegisterId::SP), InitialSP);
    }

    // A void call contains only its one-byte, twelve-byte, and twenty-four-byte argument slots and reading them leaves the complete stack unchanged.
    TEST(VMFrameTest, ReadsVoidCallArgumentsWithoutChangingStack)
    {
        alignas(8) std::array<std::uint8_t, 64> Stack;
        Stack.fill(0x6B);
        const std::uint8_t Small = 0xE7;
        const TwelveByteValue Medium{0x01020304, 0xA1A2A3A4, 0xF1F2F3F4};
        const TwentyFourByteValue Large{0x1122334455667788, 0x99AABBCCDDEEFF00, 0x0123456789ABCDEF};
        // There is no return slot: small [8,16), medium [16,32), and large [32,56).
        std::memcpy(Stack.data() + 8, &Small, sizeof(Small));
        std::memcpy(Stack.data() + 16, &Medium, sizeof(Medium));
        std::memcpy(Stack.data() + 32, &Large, sizeof(Large));
        const auto InitialStack = Stack;
        Frame CallFrame{Stack.data() + 56};
        auto* const InitialSP = readAddressRegister(CallFrame, RegisterId::SP);

        const auto ActualSmall = CallFrame.getArgument<0, std::uint8_t, TwelveByteValue, TwentyFourByteValue>();
        const auto ActualMedium = CallFrame.getArgument<1, std::uint8_t, TwelveByteValue, TwentyFourByteValue>();
        const auto ActualLarge = CallFrame.getArgument<2, std::uint8_t, TwelveByteValue, TwentyFourByteValue>();

        EXPECT_EQ(ActualSmall, Small);
        EXPECT_EQ(ActualMedium, Medium);
        EXPECT_EQ(ActualLarge, Large);
        EXPECT_EQ(Stack, InitialStack);
        EXPECT_EQ(readAddressRegister(CallFrame, RegisterId::SP), InitialSP);
    }

    // Trivially copyable arguments need no default constructor, and a zero-argument call writes its return slot immediately below SP.
    TEST(VMFrameTest, SupportsNonDefaultConstructibleArgumentsAndZeroArgumentReturns)
    {
        alignas(8) std::array<std::uint8_t, 24> ArgumentStack;
        ArgumentStack.fill(0x2D);
        const NonDefaultConstructibleValue Argument(0x1122334455667788);
        std::memcpy(ArgumentStack.data() + 8, &Argument, sizeof(Argument));
        const auto InitialArgumentStack = ArgumentStack;
        const Frame ArgumentFrame{ArgumentStack.data() + 16};
        auto* const InitialArgumentSP = readAddressRegister(ArgumentFrame, RegisterId::SP);

        const auto Actual = ArgumentFrame.getArgument<0, NonDefaultConstructibleValue>();

        EXPECT_EQ(Actual.Value, Argument.Value);
        EXPECT_EQ(ArgumentStack, InitialArgumentStack);
        EXPECT_EQ(readAddressRegister(ArgumentFrame, RegisterId::SP), InitialArgumentSP);

        alignas(8) std::array<std::uint8_t, 24> ReturnStack;
        ReturnStack.fill(0xB4);
        const NonDefaultConstructibleValue Result(0x8877665544332211);
        auto Expected = ReturnStack;
        std::memcpy(Expected.data() + 8, &Result, sizeof(Result));
        const Frame ReturnFrame{ReturnStack.data() + 16};
        auto* const InitialReturnSP = readAddressRegister(ReturnFrame, RegisterId::SP);

        ReturnFrame.setReturnValue<NonDefaultConstructibleValue>(Result);

        EXPECT_EQ(ReturnStack, Expected);
        EXPECT_EQ(readAddressRegister(ReturnFrame, RegisterId::SP), InitialReturnSP);
    }
} // namespace ink::vm::test
