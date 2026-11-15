#include "../../../core/support/environment_test_support.h"

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/interpreter/interpreter.h"

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
    constexpr std::size_t Add64Bytes = 29;

    void appendRegister(std::vector<std::uint8_t> &Bytes, RegisterOperand Operand)
    {
      Bytes.push_back(static_cast<std::uint8_t>(Operand.getRegister()));
      const auto OffsetBits = static_cast<std::uint64_t>(Operand.getByteOffset());
      for (std::size_t Index = 0; Index < 8; ++Index)
      {
        Bytes.push_back(static_cast<std::uint8_t>(OffsetBits >> (Index * 8)));
      }
    }

    void appendAdd64rrr(std::vector<std::uint8_t> &Bytes, RegisterOperand Destination, RegisterOperand LHS, RegisterOperand RHS)
    {
      Bytes.push_back(0x19);
      Bytes.push_back(0x00);
      appendRegister(Bytes, Destination);
      appendRegister(Bytes, LHS);
      appendRegister(Bytes, RHS);
    }

    void appendAdd64rrm(std::vector<std::uint8_t> &Bytes, RegisterOperand Destination, RegisterOperand LHS, AddressOperand RHS)
    {
      Bytes.push_back(0x1A);
      Bytes.push_back(0x00);
      appendRegister(Bytes, Destination);
      appendRegister(Bytes, LHS);
      appendRegister(Bytes, RegisterOperand(*RHS.getRegister(), RHS.getByteOffset()));
    }

    void appendAdd64mrr(std::vector<std::uint8_t> &Bytes, AddressOperand Destination, RegisterOperand LHS, RegisterOperand RHS)
    {
      Bytes.push_back(0x1C);
      Bytes.push_back(0x00);
      appendRegister(Bytes, RegisterOperand(*Destination.getRegister(), Destination.getByteOffset()));
      appendRegister(Bytes, LHS);
      appendRegister(Bytes, RHS);
    }

    void appendRet(std::vector<std::uint8_t> &Bytes)
    {
      Bytes.push_back(0x3A);
      Bytes.push_back(0x00);
    }

    std::uint64_t registerAddress(const Interpreter &VM, RegisterId Id) noexcept
    {
      std::uint64_t Value;
      std::memcpy(&Value, GET_REGISTER(Id, VM.getFrame()), sizeof(Value));
      return Value;
    }

    EFlags arithmeticFlags(const Interpreter &VM) noexcept
    {
      return *reinterpret_cast<const EFlags *>(GET_REGISTER(RegisterId::EFLAGS, VM.getFrame()));
    }
  }

  // Several additions advance PC by their encoded lengths and apply signed offsets to the address slots themselves.
  TEST(VMDispatchTest, ExecutesSequentialRegisterAdditionsWithSignedOffsets)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_VM_STACK_BYTES");
    ASSERT_TRUE(Environment.set("64"));
    Interpreter VM;
    ExecutableFile Executable;
    auto &Bytes = Executable.getImage();
    appendRet(Bytes);
    appendAdd64rrr(Bytes, RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::SP, 16), RegisterOperand(RegisterId::ZERO, 8));
    appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::SP, -8), RegisterOperand(RegisterId::ZERO));
    appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::BP, 4), RegisterOperand(RegisterId::ZERO, -4));
    appendRet(Bytes);
    const auto ExpectedImage = Bytes;
    ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
    const auto *Entry = VM.getCodeSegment().data() + 2;

    ASSERT_TRUE(VM.execute(Entry));

    const auto StackTop = registerAddress(VM, RegisterId::SP);
    ASSERT_NE(StackTop, 0U);
    EXPECT_EQ(registerAddress(VM, RegisterId::BP), StackTop - 8);
    EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + 3 * Add64Bytes));
    EXPECT_EQ(arithmeticFlags(VM), eflagMask(EFlag::CF));
    EXPECT_TRUE(std::ranges::equal(VM.getCodeSegment(), ExpectedImage));
  }

  // A destination used by both sources is read twice before commit, and PC sources use the current instruction's start address.
  TEST(VMDispatchTest, ReadsAliasedSourcesAndCurrentInstructionPC)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_VM_STACK_BYTES");
    ASSERT_TRUE(Environment.set("64"));
    Interpreter VM;
    ExecutableFile Executable;
    auto &Bytes = Executable.getImage();
    appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO), RegisterOperand(RegisterId::ZERO, 8));
    appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::BP));
    appendRet(Bytes);
    const auto PCProgramOffset = Bytes.size();
    appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::PC, -static_cast<std::int64_t>(PCProgramOffset)), RegisterOperand(RegisterId::ZERO, 7));
    appendRet(Bytes);
    ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
    const auto *Entry = VM.getCodeSegment().data();

    ASSERT_TRUE(VM.execute(Entry));

    EXPECT_EQ(registerAddress(VM, RegisterId::BP), 16U);
    EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + 2 * Add64Bytes));
    EXPECT_EQ(arithmeticFlags(VM), 0U);

    ASSERT_TRUE(VM.execute(Entry + PCProgramOffset));

    EXPECT_EQ(registerAddress(VM, RegisterId::BP), reinterpret_cast<std::uintptr_t>(Entry + 7));
    EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + PCProgramOffset + Add64Bytes));
    EXPECT_EQ(arithmeticFlags(VM), 0U);
  }

  // ZERO-relative integer bit patterns exercise unsigned carry, zero, sign, and signed overflow while RET preserves the resulting flags.
  TEST(VMDispatchTest, ComputesArithmeticFlagsAt64BitBoundaries)
  {
    struct AdditionCase
    {
        std::int64_t LHS;
        std::int64_t RHS;
        std::uint64_t Result;
        EFlags Flags;
    };
    const AdditionCase Cases[] = {
        {0, 0, 0, eflagMask(EFlag::ZF)},
        {1, -1, 0, eflagMask(EFlag::CF) | eflagMask(EFlag::ZF)},
        {-1, 1, 0, eflagMask(EFlag::CF) | eflagMask(EFlag::ZF)},
        {0, -1, std::numeric_limits<std::uint64_t>::max(), eflagMask(EFlag::SF)},
        {std::numeric_limits<std::int64_t>::max(), 1, std::uint64_t{1} << 63, eflagMask(EFlag::SF) | eflagMask(EFlag::OF)},
        {std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::min(), 0, eflagMask(EFlag::CF) | eflagMask(EFlag::ZF) | eflagMask(EFlag::OF)},
        {-1, 0, std::numeric_limits<std::uint64_t>::max(), eflagMask(EFlag::SF)},
        {std::numeric_limits<std::int64_t>::min(), -1, static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()), eflagMask(EFlag::CF) | eflagMask(EFlag::OF)},
    };
    core::test::ScopedEnvironmentVariable Environment("INK_VM_STACK_BYTES");
    ASSERT_TRUE(Environment.set("64"));
    for (const auto &Case : Cases)
    {
      SCOPED_TRACE(Case.LHS);
      SCOPED_TRACE(Case.RHS);
      Interpreter VM;
      ExecutableFile Executable;
      auto &Bytes = Executable.getImage();
      appendAdd64rrr(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO, Case.LHS), RegisterOperand(RegisterId::ZERO, Case.RHS));
      appendRet(Bytes);
      ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
      const auto *Entry = VM.getCodeSegment().data();

      ASSERT_TRUE(VM.execute(Entry));

      EXPECT_EQ(registerAddress(VM, RegisterId::BP), Case.Result);
      EXPECT_EQ(arithmeticFlags(VM), Case.Flags);
      EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + Add64Bytes));
      EXPECT_EQ(*reinterpret_cast<const std::uint64_t *>(GET_REGISTER(RegisterId::ZERO, VM.getFrame())), 0U);
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
      auto &Bytes = Executable.getImage();
      appendAdd64rrr(Bytes, RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::ZERO, Offset));
      appendRet(Bytes);
      ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
      const auto *Entry = VM.getCodeSegment().data();

      ASSERT_TRUE(VM.execute(Entry));

      EXPECT_EQ(registerAddress(VM, RegisterId::SP), registerAddress(VM, RegisterId::BP) + static_cast<std::uint64_t>(Offset));
      EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + Add64Bytes));
      EXPECT_EQ(arithmeticFlags(VM), Offset < 0 ? eflagMask(EFlag::CF) : 0U);
    }
  }

  // Memory forms access unaligned stack bytes while register destinations update stored values and preserve every register binding.
  TEST(VMDispatchTest, DistinguishesStackMemoryOperandsFromRegisterStorage)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_VM_STACK_BYTES");
    ASSERT_TRUE(Environment.set("64"));
    Interpreter VM;
    ExecutableFile Executable;
    auto &Bytes = Executable.getImage();
    appendRet(Bytes);
    appendAdd64mrr(Bytes, AddressOperand(RegisterId::SP, 3), RegisterOperand(RegisterId::ZERO, 17), RegisterOperand(RegisterId::ZERO, 25));
    appendAdd64rrm(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO, 8), AddressOperand(RegisterId::BP, 3));
    appendAdd64mrr(Bytes, AddressOperand(RegisterId::SP, 17), RegisterOperand(RegisterId::BP, -1), RegisterOperand(RegisterId::ZERO, 2));
    appendRet(Bytes);
    const auto ExpectedImage = Bytes;
    ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
    const auto *Entry = VM.getCodeSegment().data();
    ASSERT_TRUE(VM.execute(Entry));
    const auto StackAddress = registerAddress(VM, RegisterId::SP);
    ASSERT_NE(StackAddress, 0U);
    auto *const Stack = reinterpret_cast<std::uint8_t *>(static_cast<std::uintptr_t>(StackAddress));
    std::array<std::uint8_t, 64> Expected;
    Expected.fill(0xA5);
    std::memcpy(Stack, Expected.data(), Expected.size());
    const std::uint64_t FirstResult = 42;
    const std::uint64_t SecondResult = 51;
    std::memcpy(Expected.data() + 3, &FirstResult, sizeof(FirstResult));
    std::memcpy(Expected.data() + 17, &SecondResult, sizeof(SecondResult));
    const auto RegisterBindings = VM.getFrame().Register;

    ASSERT_TRUE(VM.execute(Entry + 2));

    EXPECT_EQ(registerAddress(VM, RegisterId::SP), StackAddress);
    EXPECT_EQ(registerAddress(VM, RegisterId::BP), 50U);
    EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + 2 + 3 * Add64Bytes));
    EXPECT_EQ(VM.getFrame().Register, RegisterBindings);
    EXPECT_TRUE(std::ranges::equal(std::span<const std::uint8_t>(Stack, Expected.size()), Expected));
    EXPECT_TRUE(std::ranges::equal(VM.getCodeSegment(), ExpectedImage));
    EXPECT_EQ(arithmeticFlags(VM), 0U);
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
    auto &Bytes = Executable.getImage();
    appendAdd64rrm(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO, 7), AddressOperand(RegisterId::ZERO, InputAddress));
    appendAdd64mrr(Bytes, AddressOperand(RegisterId::ZERO, OutputAddress), RegisterOperand(RegisterId::BP, -2), RegisterOperand(RegisterId::ZERO, 9));
    appendAdd64rrm(Bytes, RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::ZERO), AddressOperand(RegisterId::ZERO, OutputAddress));
    appendRet(Bytes);
    ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
    const auto *Entry = VM.getCodeSegment().data();

    ASSERT_TRUE(VM.execute(Entry));

    EXPECT_EQ(Memory, Expected);
    EXPECT_EQ(registerAddress(VM, RegisterId::BP), Result);
    EXPECT_EQ(registerAddress(VM, RegisterId::PC), reinterpret_cast<std::uintptr_t>(Entry + 3 * Add64Bytes));
    EXPECT_EQ(*reinterpret_cast<const std::uint64_t *>(GET_REGISTER(RegisterId::ZERO, VM.getFrame())), 0U);
    EXPECT_EQ(arithmeticFlags(VM), 0U);
  }
} // namespace ink::vm::test
