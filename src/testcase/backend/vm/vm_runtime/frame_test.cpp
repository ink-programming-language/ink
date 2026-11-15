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

    template <typename FrameType>
    concept HasPublicSP = requires(FrameType &Value)
    {
      Value.SP;
    };

    template <typename FrameType>
    concept HasPublicBP = requires(FrameType &Value)
    {
      Value.BP;
    };

    template <typename FrameType>
    concept HasPublicPC = requires(FrameType &Value)
    {
      Value.PC;
    };

    template <typename FrameType>
    concept HasPublicEFlags = requires(FrameType &Value)
    {
      Value.EFlags;
    };

    template <typename FrameType>
    concept HasPublicZERO = requires(FrameType &Value)
    {
      Value.ZERO;
    };

    static_assert(!HasPublicSP<Frame> && !HasPublicBP<Frame> && !HasPublicPC<Frame> && !HasPublicEFlags<Frame> && !HasPublicZERO<Frame>);
    static_assert(sizeof(TwelveByteValue) == 12 && alignof(TwelveByteValue) <= 8);
    static_assert(sizeof(TwentyFourByteValue) == 24 && alignof(TwentyFourByteValue) <= 8);
    static_assert(sizeof(NonDefaultConstructibleValue) == 8 && alignof(NonDefaultConstructibleValue) <= 8);
    static_assert(std::is_trivially_copyable_v<NonDefaultConstructibleValue> && !std::is_default_constructible_v<NonDefaultConstructibleValue>);
    static_assert(sizeof(const std::uint64_t *) == 8);
    static_assert(std::is_same_v<decltype(GET_REGISTER(RegisterId::SP, std::declval<const Frame &>())), const std::uint8_t *>);
    static_assert(std::is_same_v<decltype(SET_REGISTER(RegisterId::SP, std::declval<Frame &>())), std::uint8_t *>);
    static_assert(!std::is_assignable_v<decltype((GET_REGISTER(RegisterId::ZERO, std::declval<Frame &>()))), const std::uint8_t *>);
  }

  // Register slots retain pointer values and member addresses, mutable lookup writes their pointees, and copied frames rebind their own flag address.
  TEST(VMFrameTest, InitializesRegisterAddressesAndRebindsIndependentCopies)
  {
    constexpr RegisterId RegisterIds[] = {
        RegisterId::BP,
        RegisterId::SP,
        RegisterId::PC,
        RegisterId::EFLAGS,
        RegisterId::ZERO,
    };
    Frame Initial;
    EXPECT_EQ(GET_REGISTER(RegisterId::BP, Initial), nullptr);
    EXPECT_EQ(GET_REGISTER(RegisterId::SP, Initial), nullptr);
    EXPECT_EQ(GET_REGISTER(RegisterId::PC, Initial), nullptr);
    ASSERT_NE(GET_REGISTER(RegisterId::EFLAGS, Initial), nullptr);
    EXPECT_EQ(*reinterpret_cast<const EFlags *>(GET_REGISTER(RegisterId::EFLAGS, Initial)), 0U);
    ASSERT_NE(GET_REGISTER(RegisterId::ZERO, Initial), nullptr);
    EXPECT_EQ(*reinterpret_cast<const std::uint64_t *>(GET_REGISTER(RegisterId::ZERO, Initial)), 0U);

    std::array<std::uint8_t, 16> Stack{};
    std::array<std::uint8_t, 8> Code{};
    Frame Original{Stack.data() + 12};
    Original.Register[static_cast<std::size_t>(RegisterId::BP)] = Stack.data() + 4;
    Original.Register[static_cast<std::size_t>(RegisterId::PC)] = Code.data() + 1;
    *reinterpret_cast<EFlags *>(SET_REGISTER(RegisterId::EFLAGS, Original)) = ArithmeticEFlagsMask;
    const std::array<const void *, 5> Expected = {
        Stack.data() + 4,
        Stack.data() + 12,
        Code.data() + 1,
        GET_REGISTER(RegisterId::EFLAGS, Original),
        GET_REGISTER(RegisterId::ZERO, Initial),
    };
    for (const auto Id : RegisterIds)
    {
      const auto Index = static_cast<std::size_t>(Id);
      EXPECT_EQ(GET_REGISTER(Id, Original), Expected[Index]);
      EXPECT_EQ(SET_REGISTER(Id, Original), const_cast<void *>(Expected[Index]));
      EXPECT_EQ(Original.Register[Index], Expected[Index]);
    }

    *SET_REGISTER(RegisterId::BP, Original) = 0xB1;
    *SET_REGISTER(RegisterId::SP, Original) = 0xA2;
    *SET_REGISTER(RegisterId::PC, Original) = 0xC3;
    *reinterpret_cast<EFlags *>(SET_REGISTER(RegisterId::EFLAGS, Original)) = eflagMask(EFlag::CF) | eflagMask(EFlag::ZF);
    EXPECT_EQ(Stack[4], 0xB1);
    EXPECT_EQ(Stack[12], 0xA2);
    EXPECT_EQ(Code[1], 0xC3);
    EXPECT_EQ(GET_REGISTER(RegisterId::BP, Original), Stack.data() + 4);
    EXPECT_EQ(GET_REGISTER(RegisterId::SP, Original), Stack.data() + 12);
    EXPECT_EQ(GET_REGISTER(RegisterId::PC, Original), Code.data() + 1);
    const EFlags OriginalFlags = *reinterpret_cast<const EFlags *>(GET_REGISTER(RegisterId::EFLAGS, Original));
    EXPECT_EQ(*reinterpret_cast<const EFlags *>(GET_REGISTER(RegisterId::EFLAGS, Original)), OriginalFlags);

    Frame Copied = Original;
    Frame Assigned;
    Assigned = Original;
    Frame RvalueCopied = std::move(Original);
    Frame RvalueAssigned;
    RvalueAssigned = std::move(Original);
    const std::array<Frame *, 4> Copies = {
        &Copied,
        &Assigned,
        &RvalueCopied,
        &RvalueAssigned,
    };
    EFlags CopyFlags = eflagMask(EFlag::SF);
    for (auto *Value : Copies)
    {
      EXPECT_EQ(GET_REGISTER(RegisterId::BP, *Value), GET_REGISTER(RegisterId::BP, Original));
      EXPECT_EQ(GET_REGISTER(RegisterId::SP, *Value), GET_REGISTER(RegisterId::SP, Original));
      EXPECT_EQ(GET_REGISTER(RegisterId::PC, *Value), GET_REGISTER(RegisterId::PC, Original));
      EXPECT_NE(GET_REGISTER(RegisterId::EFLAGS, *Value), GET_REGISTER(RegisterId::EFLAGS, Original));
      EXPECT_EQ(GET_REGISTER(RegisterId::ZERO, *Value), GET_REGISTER(RegisterId::ZERO, Initial));
      EXPECT_EQ(*reinterpret_cast<const std::uint64_t *>(GET_REGISTER(RegisterId::ZERO, *Value)), 0U);
      EXPECT_EQ(*reinterpret_cast<const EFlags *>(GET_REGISTER(RegisterId::EFLAGS, *Value)), OriginalFlags);
      *reinterpret_cast<EFlags *>(SET_REGISTER(RegisterId::EFLAGS, *Value)) = CopyFlags++;
      EXPECT_EQ(*reinterpret_cast<const EFlags *>(GET_REGISTER(RegisterId::EFLAGS, Original)), OriginalFlags);
    }
    Copied.Register[static_cast<std::size_t>(RegisterId::BP)] = Stack.data() + 2;
    Copied.Register[static_cast<std::size_t>(RegisterId::SP)] = Stack.data() + 3;
    Copied.Register[static_cast<std::size_t>(RegisterId::PC)] = Code.data() + 2;
    EXPECT_EQ(GET_REGISTER(RegisterId::BP, Copied), Stack.data() + 2);
    EXPECT_EQ(GET_REGISTER(RegisterId::SP, Copied), Stack.data() + 3);
    EXPECT_EQ(GET_REGISTER(RegisterId::PC, Copied), Code.data() + 2);
    EXPECT_EQ(GET_REGISTER(RegisterId::BP, Original), Stack.data() + 4);
    EXPECT_EQ(GET_REGISTER(RegisterId::SP, Original), Stack.data() + 12);
    EXPECT_EQ(GET_REGISTER(RegisterId::PC, Original), Code.data() + 1);

    std::size_t IdEvaluations = 0;
    std::size_t FrameEvaluations = 0;
    auto SelectId = [&]()
    {
      ++IdEvaluations;
      return RegisterId::EFLAGS;
    };
    auto SelectFrame = [&]() -> Frame &
    {
      ++FrameEvaluations;
      return Copied;
    };

    const auto ExpectedFlagAddress = GET_REGISTER(RegisterId::EFLAGS, Copied);
    const auto ReadAddress = GET_REGISTER(SelectId(), SelectFrame());
    EXPECT_EQ(ReadAddress, ExpectedFlagAddress);
    EXPECT_EQ(IdEvaluations, 1U);
    EXPECT_EQ(FrameEvaluations, 1U);
    const auto WriteAddress = SET_REGISTER(SelectId(), SelectFrame());
    EXPECT_EQ(WriteAddress, ExpectedFlagAddress);
    EXPECT_EQ(IdEvaluations, 2U);
    EXPECT_EQ(FrameEvaluations, 2U);
    const Frame &View = Copied;
    EXPECT_EQ(GET_REGISTER(RegisterId::EFLAGS, View), GET_REGISTER(RegisterId::EFLAGS, Copied));
    EXPECT_EQ(GET_REGISTER(RegisterId::ZERO, View), GET_REGISTER(RegisterId::ZERO, Initial));
  }

  // Updating SP through Register changes native argument and return locations, and copies preserve current slots while owning their flags.
  TEST(VMFrameTest, UsesUpdatedRegisterSlotsForNativeCallsAndCopies)
  {
    alignas(8) std::array<std::uint8_t, 48> Stack;
    Stack.fill(0xA7);
    const std::uint64_t Argument = 0x0123456789ABCDEF;
    std::memcpy(Stack.data() + 32, &Argument, sizeof(Argument));
    auto Expected = Stack;
    Frame CallFrame{Stack.data() + 16};
    CallFrame.Register[static_cast<std::size_t>(RegisterId::SP)] = Stack.data() + 40;
    EFlags Flags = ArithmeticEFlagsMask;
    CallFrame.Register[static_cast<std::size_t>(RegisterId::EFLAGS)] = &Flags;

    const auto Actual = CallFrame.getArgument<0, std::uint64_t>();
    EXPECT_EQ(Actual, Argument);
    Frame Copied = CallFrame;
    Frame Assigned;
    Assigned = CallFrame;
    for (auto *Value : {&Copied, &Assigned})
    {
      const auto CopiedArgument = Value->getArgument<0, std::uint64_t>();
      EXPECT_EQ(CopiedArgument, Argument);
      EXPECT_EQ(GET_REGISTER(RegisterId::SP, *Value), Stack.data() + 40);
      EXPECT_NE(GET_REGISTER(RegisterId::EFLAGS, *Value), reinterpret_cast<const std::uint8_t *>(&Flags));
      EXPECT_EQ(*reinterpret_cast<const EFlags *>(GET_REGISTER(RegisterId::EFLAGS, *Value)), Flags);
      *reinterpret_cast<EFlags *>(SET_REGISTER(RegisterId::EFLAGS, *Value)) = 0;
    }
    EXPECT_EQ(Flags, ArithmeticEFlagsMask);

    const std::uint64_t Result = 0xFEDCBA9876543210;
    std::memcpy(Expected.data() + 24, &Result, sizeof(Result));
    CallFrame.setReturnValue<std::uint64_t, std::uint64_t>(Result);
    EXPECT_EQ(Stack, Expected);
    EXPECT_EQ(GET_REGISTER(RegisterId::SP, CallFrame), Stack.data() + 40);
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
    MutableCallFrame.Register[static_cast<std::size_t>(RegisterId::BP)] = Stack.data() + 16;
    MutableCallFrame.Register[static_cast<std::size_t>(RegisterId::PC)] = Code.data();
    *reinterpret_cast<EFlags *>(SET_REGISTER(RegisterId::EFLAGS, MutableCallFrame)) = ArithmeticEFlagsMask;
    const Frame CallFrame = MutableCallFrame;
    auto *const InitialSP = GET_REGISTER(RegisterId::SP, CallFrame);

    const auto ActualSmall = CallFrame.getArgument<0, std::int16_t, TwelveByteValue, const std::uint64_t *>();
    const auto ActualAggregate = CallFrame.getArgument<1, std::int16_t, TwelveByteValue, const std::uint64_t *>();
    const auto ActualPointer = CallFrame.getArgument<2, std::int16_t, TwelveByteValue, const std::uint64_t *>();

    EXPECT_EQ(ActualSmall, Small);
    EXPECT_EQ(ActualAggregate, Aggregate);
    EXPECT_EQ(ActualPointer, Pointer);
    EXPECT_EQ(Stack, InitialStack);
    EXPECT_EQ(GET_REGISTER(RegisterId::SP, CallFrame), InitialSP);
    EXPECT_EQ(GET_REGISTER(RegisterId::BP, CallFrame), Stack.data() + 16);
    EXPECT_EQ(GET_REGISTER(RegisterId::PC, CallFrame), Code.data());
    EXPECT_EQ(*reinterpret_cast<const EFlags *>(GET_REGISTER(RegisterId::EFLAGS, CallFrame)), ArithmeticEFlagsMask);
    EXPECT_EQ(*reinterpret_cast<const std::uint64_t *>(GET_REGISTER(RegisterId::ZERO, CallFrame)), 0U);

    const std::uint8_t Result = 0x3C;
    auto Expected = InitialStack;
    std::memcpy(Expected.data() + 8, &Result, sizeof(Result));
    CallFrame.setReturnValue<std::uint8_t, std::int16_t, TwelveByteValue, const std::uint64_t *>(Result);

    EXPECT_EQ(Stack, Expected);
    EXPECT_EQ(GET_REGISTER(RegisterId::SP, CallFrame), InitialSP);
    EXPECT_EQ(GET_REGISTER(RegisterId::BP, CallFrame), Stack.data() + 16);
    EXPECT_EQ(GET_REGISTER(RegisterId::PC, CallFrame), Code.data());
    EXPECT_EQ(*reinterpret_cast<const EFlags *>(GET_REGISTER(RegisterId::EFLAGS, CallFrame)), ArithmeticEFlagsMask);
    EXPECT_EQ(*reinterpret_cast<const std::uint64_t *>(GET_REGISTER(RegisterId::ZERO, CallFrame)), 0U);
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
    auto *const InitialSP = GET_REGISTER(RegisterId::SP, CallFrame);

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
    EXPECT_EQ(GET_REGISTER(RegisterId::SP, CallFrame), InitialSP);
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
    auto *const InitialSP = GET_REGISTER(RegisterId::SP, CallFrame);

    const auto ActualSmall = CallFrame.getArgument<0, std::uint8_t, TwelveByteValue, TwentyFourByteValue>();
    const auto ActualMedium = CallFrame.getArgument<1, std::uint8_t, TwelveByteValue, TwentyFourByteValue>();
    const auto ActualLarge = CallFrame.getArgument<2, std::uint8_t, TwelveByteValue, TwentyFourByteValue>();

    EXPECT_EQ(ActualSmall, Small);
    EXPECT_EQ(ActualMedium, Medium);
    EXPECT_EQ(ActualLarge, Large);
    EXPECT_EQ(Stack, InitialStack);
    EXPECT_EQ(GET_REGISTER(RegisterId::SP, CallFrame), InitialSP);
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
    auto *const InitialArgumentSP = GET_REGISTER(RegisterId::SP, ArgumentFrame);

    const auto Actual = ArgumentFrame.getArgument<0, NonDefaultConstructibleValue>();

    EXPECT_EQ(Actual.Value, Argument.Value);
    EXPECT_EQ(ArgumentStack, InitialArgumentStack);
    EXPECT_EQ(GET_REGISTER(RegisterId::SP, ArgumentFrame), InitialArgumentSP);

    alignas(8) std::array<std::uint8_t, 24> ReturnStack;
    ReturnStack.fill(0xB4);
    const NonDefaultConstructibleValue Result(0x8877665544332211);
    auto Expected = ReturnStack;
    std::memcpy(Expected.data() + 8, &Result, sizeof(Result));
    const Frame ReturnFrame{ReturnStack.data() + 16};
    auto *const InitialReturnSP = GET_REGISTER(RegisterId::SP, ReturnFrame);

    ReturnFrame.setReturnValue<NonDefaultConstructibleValue>(Result);

    EXPECT_EQ(ReturnStack, Expected);
    EXPECT_EQ(GET_REGISTER(RegisterId::SP, ReturnFrame), InitialReturnSP);
  }
} // namespace ink::vm::test
