#include "ink/backend/vm/vm_runtime/frame.h"
#include "ink/backend/vm/instruction/register.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
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

        constexpr bool operator==(const TwelveByteValue &) const noexcept = default;
    };

    struct TwentyFourByteValue
    {
        std::uint64_t First;
        std::uint64_t Second;
        std::uint64_t Third;

        constexpr bool operator==(const TwentyFourByteValue &) const noexcept = default;
    };

    struct NonDefaultConstructibleValue
    {
        explicit constexpr NonDefaultConstructibleValue(std::uint64_t Value) noexcept
            : Value(Value)
        {
        }

        std::uint64_t Value;
    };

    static_assert(sizeof(TwelveByteValue) == 12 && alignof(TwelveByteValue) <= 8);
    static_assert(sizeof(TwentyFourByteValue) == 24 && alignof(TwentyFourByteValue) <= 8);
    static_assert(sizeof(NonDefaultConstructibleValue) == 8 && alignof(NonDefaultConstructibleValue) <= 8);
    static_assert(std::is_trivially_copyable_v<NonDefaultConstructibleValue> && !std::is_default_constructible_v<NonDefaultConstructibleValue>);
    static_assert(sizeof(const std::uint64_t *) == 8);
    static_assert(std::is_same_v<decltype(std::declval<const Frame &>().getSP()), std::uint8_t *> && std::is_same_v<decltype(std::declval<const Frame &>().getBP()), std::uint8_t *> && std::is_same_v<decltype(std::declval<const Frame &>().getPC()), std::uint8_t *>);
    static_assert(std::is_same_v<decltype(std::declval<const Frame &>().getEFlags()), EFlags>);
    static_assert(std::is_same_v<decltype(std::declval<Frame &>().getRegisters()), const std::array<std::uint64_t, 5> &> && std::is_same_v<decltype(std::declval<const Frame &>().getRegisters()), const std::array<std::uint64_t, 5> &>);
    static_assert(!std::is_assignable_v<decltype((std::declval<Frame &>().getRegisters()[static_cast<std::size_t>(RegisterId::ZERO)])), std::uint64_t>);
  }

  // Register IDs index canonical pointer and zero-extended flag slots; copies own independent storage and every typed write leaves ZERO unchanged.
  TEST(VMFrameTest, IndexesRegisterStorageAndPreservesIndependentCopiesAndZero)
  {
    Frame Initial;
    EXPECT_EQ(Initial.getRegisters(), (std::array<std::uint64_t, 5>{}));
    EXPECT_EQ(Initial.getBP(), nullptr);
    EXPECT_EQ(Initial.getSP(), nullptr);
    EXPECT_EQ(Initial.getPC(), nullptr);
    EXPECT_EQ(Initial.getEFlags(), 0U);
    std::array<std::uint8_t, 16> Stack{};
    std::array<std::uint8_t, 8> Code{};
    Frame Original{Stack.data() + 12};
    Original.setBP(Stack.data() + 4);
    Original.setPC(Code.data() + 1);
    Original.setEFlags(EFlags{0xFEDCBA98});
    const std::array<std::uint64_t, 5> Expected = {
        static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Stack.data() + 4)),
        static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Stack.data() + 12)),
        static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Code.data() + 1)),
        0xFEDCBA98,
        0,
    };
    constexpr RegisterId RegisterIds[] = {
        RegisterId::BP,
        RegisterId::SP,
        RegisterId::PC,
        RegisterId::EFLAGS,
        RegisterId::ZERO,
    };

    for (const auto Id : RegisterIds)
    {
      const auto Index = static_cast<std::size_t>(Id);
      EXPECT_EQ(Original.getRegisters()[Index], Expected[Index]);
    }
    EXPECT_EQ(Original.getBP(), Stack.data() + 4);
    EXPECT_EQ(Original.getSP(), Stack.data() + 12);
    EXPECT_EQ(Original.getPC(), Code.data() + 1);
    EXPECT_EQ(Original.getEFlags(), EFlags{0xFEDCBA98});

    Frame Copied = Original;
    Frame Assigned;
    Assigned = Original;
    EXPECT_NE(Copied.getRegisters().data(), Original.getRegisters().data());
    EXPECT_NE(Assigned.getRegisters().data(), Original.getRegisters().data());
    Original.setBP(nullptr);
    Original.setSP(nullptr);
    Original.setPC(nullptr);
    Original.setEFlags(0);
    EXPECT_EQ(Original.getRegisters(), (std::array<std::uint64_t, 5>{}));
    EXPECT_EQ(Copied.getRegisters(), Expected);
    EXPECT_EQ(Assigned.getRegisters(), Expected);

    Copied.setBP(Stack.data() + 2);
    Copied.setSP(Stack.data() + 3);
    Copied.setPC(Code.data() + 2);
    Copied.setEFlags(EFlags{0xFFFFFFFF});
    EXPECT_EQ(Copied.getRegisters()[static_cast<std::size_t>(RegisterId::EFLAGS)], 0xFFFFFFFFULL);
    EXPECT_EQ(Copied.getRegisters()[static_cast<std::size_t>(RegisterId::ZERO)], 0U);
    EXPECT_EQ(Assigned.getRegisters(), Expected);
    EXPECT_EQ(Original.getRegisters()[static_cast<std::size_t>(RegisterId::ZERO)], 0U);
  }

  // Mixed arguments and a one-byte result preserve the runtime registers, argument order, and all slot padding.
  TEST(VMFrameTest, ReadsMixedArgumentsAndWritesOnlySmallReturnValue)
  {
    alignas(8) std::array<std::uint8_t, 56> Stack;
    Stack.fill(0xA5);
    const std::int16_t Small = -1234;
    const TwelveByteValue Aggregate{0x01234567, 0x89ABCDEF, 0x76543210};
    const std::uint64_t Pointee = 0xFEDCBA9876543210;
    const std::uint64_t *const Pointer = &Pointee;
    std::array<std::uint8_t, 8> Code{};
    // The call area is return [8,16), small [16,24), aggregate [24,40), and pointer [40,48).
    std::memcpy(Stack.data() + 16, &Small, sizeof(Small));
    std::memcpy(Stack.data() + 24, &Aggregate, sizeof(Aggregate));
    std::memcpy(Stack.data() + 40, &Pointer, sizeof(Pointer));
    const auto InitialStack = Stack;
    Frame MutableCallFrame{Stack.data() + 48};
    MutableCallFrame.setBP(Stack.data() + 16);
    MutableCallFrame.setPC(Code.data());
    MutableCallFrame.setEFlags(ArithmeticEFlagsMask);
    const Frame CallFrame = MutableCallFrame;
    auto *const InitialSP = CallFrame.getSP();

    const auto ActualSmall = CallFrame.getArgument<0, std::int16_t, TwelveByteValue, const std::uint64_t *>();
    const auto ActualAggregate = CallFrame.getArgument<1, std::int16_t, TwelveByteValue, const std::uint64_t *>();
    const auto ActualPointer = CallFrame.getArgument<2, std::int16_t, TwelveByteValue, const std::uint64_t *>();

    EXPECT_EQ(ActualSmall, Small);
    EXPECT_EQ(ActualAggregate, Aggregate);
    EXPECT_EQ(ActualPointer, Pointer);
    EXPECT_EQ(Stack, InitialStack);
    EXPECT_EQ(CallFrame.getSP(), InitialSP);
    EXPECT_EQ(CallFrame.getBP(), Stack.data() + 16);
    EXPECT_EQ(CallFrame.getPC(), Code.data());
    EXPECT_EQ(CallFrame.getEFlags(), ArithmeticEFlagsMask);
    EXPECT_EQ(CallFrame.getRegisters()[static_cast<std::size_t>(RegisterId::ZERO)], 0U);

    const std::uint8_t Result = 0x3C;
    auto Expected = InitialStack;
    std::memcpy(Expected.data() + 8, &Result, sizeof(Result));
    CallFrame.setReturnValue<std::uint8_t, std::int16_t, TwelveByteValue, const std::uint64_t *>(Result);

    EXPECT_EQ(Stack, Expected);
    EXPECT_EQ(CallFrame.getSP(), InitialSP);
    EXPECT_EQ(CallFrame.getBP(), Stack.data() + 16);
    EXPECT_EQ(CallFrame.getPC(), Code.data());
    EXPECT_EQ(CallFrame.getEFlags(), ArithmeticEFlagsMask);
    EXPECT_EQ(CallFrame.getRegisters()[static_cast<std::size_t>(RegisterId::ZERO)], 0U);
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
    auto *const InitialSP = CallFrame.getSP();

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
    EXPECT_EQ(CallFrame.getSP(), InitialSP);
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
    auto *const InitialSP = CallFrame.getSP();

    const auto ActualSmall = CallFrame.getArgument<0, std::uint8_t, TwelveByteValue, TwentyFourByteValue>();
    const auto ActualMedium = CallFrame.getArgument<1, std::uint8_t, TwelveByteValue, TwentyFourByteValue>();
    const auto ActualLarge = CallFrame.getArgument<2, std::uint8_t, TwelveByteValue, TwentyFourByteValue>();

    EXPECT_EQ(ActualSmall, Small);
    EXPECT_EQ(ActualMedium, Medium);
    EXPECT_EQ(ActualLarge, Large);
    EXPECT_EQ(Stack, InitialStack);
    EXPECT_EQ(CallFrame.getSP(), InitialSP);
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
    auto *const InitialArgumentSP = ArgumentFrame.getSP();

    const auto Actual = ArgumentFrame.getArgument<0, NonDefaultConstructibleValue>();

    EXPECT_EQ(Actual.Value, Argument.Value);
    EXPECT_EQ(ArgumentStack, InitialArgumentStack);
    EXPECT_EQ(ArgumentFrame.getSP(), InitialArgumentSP);

    alignas(8) std::array<std::uint8_t, 24> ReturnStack;
    ReturnStack.fill(0xB4);
    const NonDefaultConstructibleValue Result(0x8877665544332211);
    auto Expected = ReturnStack;
    std::memcpy(Expected.data() + 8, &Result, sizeof(Result));
    const Frame ReturnFrame{ReturnStack.data() + 16};
    auto *const InitialReturnSP = ReturnFrame.getSP();

    ReturnFrame.setReturnValue<NonDefaultConstructibleValue>(Result);

    EXPECT_EQ(ReturnStack, Expected);
    EXPECT_EQ(ReturnFrame.getSP(), InitialReturnSP);
  }
} // namespace ink::vm::test
