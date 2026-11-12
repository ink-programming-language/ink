#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_basic_block.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace ink::vm::test
{
  // A borrowed base reference identifies each concrete operand without introducing virtual dispatch.
  TEST(VMOperandTest, IdentifiesConcreteKindsThroughBaseReferences)
  {
    static_assert(!std::is_polymorphic_v<Operand>);
    constexpr RegisterOperand Register(RegisterId::BP);
    constexpr AddressOperand Address(RegisterId::SP, 16);
    constexpr ImmediateOperand Immediate(42);
    static_assert(Register.getKind() == OperandKind::Register);
    static_assert(Register.getByteOffset() == 0);
    static_assert(Address.getKind() == OperandKind::Address);
    static_assert(Immediate.getKind() == OperandKind::Immediate);
    const Operand *Operands[] = {&Register, &Address, &Immediate};
    const OperandKind Kinds[] = {OperandKind::Register, OperandKind::Address, OperandKind::Immediate};
    for (std::size_t Index = 0; Index < 3; ++Index)
    {
      EXPECT_EQ(Operands[Index]->getKind(), Kinds[Index]);
    }
  }

  // Register expressions retain every source register and the full signed 64-bit byte offset range.
  TEST(VMOperandTest, PreservesRegisterIdentifiersAndSignedOffsetBoundaries)
  {
    const RegisterId Registers[] = {RegisterId::BP, RegisterId::SP, RegisterId::PC, RegisterId::ZERO};
    const std::int64_t Offsets[] = {
        std::numeric_limits<std::int64_t>::min(),
        -1,
        0,
        1,
        std::numeric_limits<std::int64_t>::max(),
    };
    for (const auto Register : Registers)
    {
      for (const auto Offset : Offsets)
      {
        const RegisterOperand Operand(Register, Offset);
        EXPECT_EQ(Operand.getKind(), OperandKind::Register);
        EXPECT_EQ(Operand.getRegister(), Register);
        EXPECT_EQ(Operand.getByteOffset(), Offset);
      }
    }
  }

  // Register-relative addresses and named VM symbols retain distinct base kinds and expose only their own payload.
  TEST(VMOperandTest, DistinguishesRegisterAndSymbolBases)
  {
    constexpr AddressOperand RegisterAddress(RegisterId::BP);
    const core::Name Symbol = core::NamePool::instance().intern("operand.symbol");
    const AddressOperand SymbolAddress(Symbol);
    static_assert(RegisterAddress.getBaseKind() == AddressBaseKind::Register);
    static_assert(RegisterAddress.getRegister() == RegisterId::BP);
    static_assert(RegisterAddress.getByteOffset() == 0);
    EXPECT_EQ(RegisterAddress.getBaseKind(), AddressBaseKind::Register);
    EXPECT_EQ(RegisterAddress.getRegister(), RegisterId::BP);
    EXPECT_FALSE(RegisterAddress.getSymbol().has_value());
    EXPECT_EQ(SymbolAddress.getBaseKind(), AddressBaseKind::Symbol);
    EXPECT_EQ(SymbolAddress.getSymbol(), Symbol);
    EXPECT_EQ(SymbolAddress.getByteOffset(), 0);
    EXPECT_FALSE(SymbolAddress.getRegister().has_value());
  }

  // Symbol names retain their identity even when their spelling resembles a register or a number.
  TEST(VMOperandTest, PreservesNamedSymbolIdentity)
  {
    const core::Name Symbols[] = {
        core::NamePool::instance().intern("bp"),
        core::NamePool::instance().intern("255"),
        core::NamePool::instance().intern("operand.external"),
    };
    for (const auto Symbol : Symbols)
    {
      const AddressOperand Address(Symbol, 8);
      EXPECT_EQ(Address.getBaseKind(), AddressBaseKind::Symbol);
      EXPECT_EQ(Address.getSymbol(), Symbol);
      EXPECT_EQ(Address.getByteOffset(), 8);
      EXPECT_FALSE(Address.getRegister().has_value());
    }
  }

  // Both address forms retain negative displacements and both signed 64-bit offset boundaries exactly.
  TEST(VMOperandTest, PreservesSignedAddressOffsetBoundaries)
  {
    const core::Name Symbol = core::NamePool::instance().intern("operand.offset");
    const std::int64_t Offsets[] = {
        std::numeric_limits<std::int64_t>::min(),
        -1,
        0,
        1,
        std::numeric_limits<std::int64_t>::max(),
    };
    for (const auto Offset : Offsets)
    {
      SCOPED_TRACE(Offset);
      const AddressOperand RegisterAddress(RegisterId::SP, Offset);
      const AddressOperand SymbolAddress(Symbol, Offset);
      EXPECT_EQ(RegisterAddress.getByteOffset(), Offset);
      EXPECT_EQ(SymbolAddress.getByteOffset(), Offset);
      EXPECT_EQ(RegisterAddress.getRegister(), RegisterId::SP);
      EXPECT_EQ(SymbolAddress.getSymbol(), Symbol);
    }
    const AddressOperand ProgramCounterAddress(RegisterId::PC, -8);
    EXPECT_EQ(ProgramCounterAddress.getRegister(), RegisterId::PC);
    EXPECT_EQ(ProgramCounterAddress.getByteOffset(), -8);
  }

  // Immediate operands retain all 64 payload bits; instruction width and signed interpretation are applied elsewhere.
  TEST(VMOperandTest, PreservesImmediatePayloadBits)
  {
    constexpr ImmediateOperand Maximum(std::numeric_limits<std::uint64_t>::max());
    static_assert(Maximum.bits() == std::numeric_limits<std::uint64_t>::max());
    static_assert(Maximum.getValueKind() == ImmediateValueKind::Numeric);
    static_assert(!Maximum.getSymbol().has_value() && !Maximum.getByteOffset().has_value());
    const std::uint64_t Values[] = {
        0,
        1,
        std::uint64_t{1} << 63,
        std::numeric_limits<std::uint64_t>::max(),
    };
    for (const auto Value : Values)
    {
      SCOPED_TRACE(Value);
      const ImmediateOperand Immediate(Value);
      EXPECT_EQ(Immediate.getKind(), OperandKind::Immediate);
      EXPECT_EQ(Immediate.getValueKind(), ImmediateValueKind::Numeric);
      EXPECT_EQ(Immediate.bits(), Value);
      EXPECT_FALSE(Immediate.getSymbol().has_value());
      EXPECT_FALSE(Immediate.getByteOffset().has_value());
    }
  }

  // Unresolved address immediates preserve a symbol and the entire signed offset range without exposing numeric bits.
  TEST(VMOperandTest, PreservesSymbolImmediateOffsetBoundaries)
  {
    const core::Name Symbol = core::NamePool::instance().intern("operand.immediate");
    const std::int64_t Offsets[] = {
        std::numeric_limits<std::int64_t>::min(),
        -1,
        0,
        1,
        std::numeric_limits<std::int64_t>::max(),
    };
    for (const auto Offset : Offsets)
    {
      const ImmediateOperand Immediate(Symbol, Offset);
      EXPECT_EQ(Immediate.getKind(), OperandKind::Immediate);
      EXPECT_EQ(Immediate.getValueKind(), ImmediateValueKind::Symbol);
      EXPECT_EQ(Immediate.getSymbol(), Symbol);
      EXPECT_EQ(Immediate.getByteOffset(), Offset);
      EXPECT_FALSE(Immediate.bits().has_value());
    }
    const ImmediateOperand ZeroOffset(Symbol);
    EXPECT_EQ(ZeroOffset.getByteOffset(), 0);
  }

  // Copy assignment can switch between numeric and symbolic forms without leaking an inactive payload through accessors.
  TEST(VMOperandTest, ReassignsNumericAndSymbolicImmediatesIndependently)
  {
    const core::Name Symbol = core::NamePool::instance().intern("operand.copy");
    const ImmediateOperand Original(Symbol, -8);
    auto Copy = Original;
    Copy = ImmediateOperand(std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(Original.getSymbol(), Symbol);
    EXPECT_EQ(Original.getByteOffset(), -8);
    EXPECT_FALSE(Original.bits().has_value());
    EXPECT_EQ(Copy.bits(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_FALSE(Copy.getSymbol().has_value());
    Copy = Original;
    EXPECT_EQ(Copy.getSymbol(), Symbol);
    EXPECT_EQ(Copy.getByteOffset(), -8);
    EXPECT_FALSE(Copy.bits().has_value());
  }

  // Concrete value copies preserve their payloads, while base assignment cannot overwrite a concrete operand's kind.
  TEST(VMOperandTest, CopiesConcreteValuesWithoutExposingBaseAssignment)
  {
    static_assert(!std::is_assignable_v<Operand &, const Operand &>);
    static_assert(std::is_copy_constructible_v<RegisterOperand> && std::is_copy_assignable_v<RegisterOperand>);
    static_assert(std::is_copy_constructible_v<AddressOperand> && std::is_copy_assignable_v<AddressOperand>);
    static_assert(std::is_copy_constructible_v<ImmediateOperand> && std::is_copy_assignable_v<ImmediateOperand>);
    const RegisterOperand Register(RegisterId::BP, -8);
    auto RegisterCopy = Register;
    EXPECT_EQ(RegisterCopy.getByteOffset(), -8);
    RegisterCopy = RegisterOperand(RegisterId::SP, 16);
    EXPECT_EQ(Register.getRegister(), RegisterId::BP);
    EXPECT_EQ(Register.getByteOffset(), -8);
    EXPECT_EQ(RegisterCopy.getRegister(), RegisterId::SP);
    EXPECT_EQ(RegisterCopy.getByteOffset(), 16);
    EXPECT_EQ(RegisterCopy.getKind(), OperandKind::Register);
    const AddressOperand Address(RegisterId::BP, -16);
    auto AddressCopy = Address;
    const core::Name Symbol = core::NamePool::instance().intern("operand.address.copy");
    AddressCopy = AddressOperand(Symbol, 32);
    EXPECT_EQ(Address.getRegister(), RegisterId::BP);
    EXPECT_EQ(Address.getByteOffset(), -16);
    EXPECT_EQ(AddressCopy.getSymbol(), Symbol);
    EXPECT_EQ(AddressCopy.getByteOffset(), 32);
    EXPECT_FALSE(AddressCopy.getRegister().has_value());
    EXPECT_EQ(AddressCopy.getKind(), OperandKind::Address);
    const ImmediateOperand Immediate(std::numeric_limits<std::uint64_t>::max());
    auto ImmediateCopy = Immediate;
    ImmediateCopy = ImmediateOperand(0);
    EXPECT_EQ(Immediate.bits(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(ImmediateCopy.bits(), 0U);
    EXPECT_EQ(ImmediateCopy.getKind(), OperandKind::Immediate);
  }

  // Block operands retain borrowed identity through copying, including a null raw value before builder validation.
  TEST(VMOperandTest, PreservesBorrowedBasicBlockTargets)
  {
    MachineBasicBlock First;
    MachineBasicBlock Second;
    const BasicBlockOperand Target(&First);
    const Operand &Base = Target;
    EXPECT_EQ(Base.getKind(), OperandKind::BasicBlock);
    EXPECT_EQ(Target.getBlock(), &First);
    auto Copy = Target;
    Copy = BasicBlockOperand(&Second);
    EXPECT_EQ(Copy.getBlock(), &Second);
    EXPECT_EQ(Target.getBlock(), &First);
    EXPECT_EQ(Copy.getKind(), OperandKind::BasicBlock);
    constexpr BasicBlockOperand NullTarget(nullptr);
    static_assert(NullTarget.getBlock() == nullptr);
    static_assert(NullTarget.getKind() == OperandKind::BasicBlock);
    static_assert(!std::is_default_constructible_v<BasicBlockOperand>);
    static_assert(std::is_copy_constructible_v<BasicBlockOperand> && std::is_copy_assignable_v<BasicBlockOperand>);
    static_assert(std::is_same_v<decltype(Target.getBlock()), const MachineBasicBlock *>);
  }
} // namespace ink::vm::test
