#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_basic_block.h"

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
    static_assert(Address.getKind() == OperandKind::Address);
    static_assert(Immediate.getKind() == OperandKind::Immediate);
    const Operand *Operands[] = {&Register, &Address, &Immediate};
    const OperandKind Kinds[] = {OperandKind::Register, OperandKind::Address, OperandKind::Immediate};
    for (std::size_t Index = 0; Index < 3; ++Index)
    {
      EXPECT_EQ(Operands[Index]->getKind(), Kinds[Index]);
    }
  }

  // Register operands retain each independent logical register identifier, including PC as a source value.
  TEST(VMOperandTest, PreservesDefinedRegisterIdentifiers)
  {
    const RegisterId Registers[] = {RegisterId::BP, RegisterId::SP, RegisterId::PC};
    for (const auto Register : Registers)
    {
      const RegisterOperand Operand(Register);
      EXPECT_EQ(Operand.getKind(), OperandKind::Register);
      EXPECT_EQ(Operand.getRegister(), Register);
    }
  }

  // Register BP and symbol-reference zero remain distinguishable even though their numeric identifiers are equal.
  TEST(VMOperandTest, DistinguishesRegisterAndSymbolBasesAtZero)
  {
    constexpr AddressOperand RegisterAddress(RegisterId::BP);
    constexpr AddressOperand SymbolAddress(std::uint32_t{0});
    static_assert(RegisterAddress.getBaseKind() == AddressBaseKind::Register);
    static_assert(SymbolAddress.getBaseKind() == AddressBaseKind::Symbol);
    static_assert(RegisterAddress.getRegister() == RegisterId::BP);
    static_assert(SymbolAddress.getBaseReferenceIndex() == 0U);
    static_assert(RegisterAddress.getByteOffset() == 0 && SymbolAddress.getByteOffset() == 0);
    EXPECT_EQ(RegisterAddress.getBaseKind(), AddressBaseKind::Register);
    EXPECT_EQ(RegisterAddress.getRegister(), RegisterId::BP);
    EXPECT_FALSE(RegisterAddress.getBaseReferenceIndex().has_value());
    EXPECT_EQ(SymbolAddress.getBaseKind(), AddressBaseKind::Symbol);
    EXPECT_EQ(SymbolAddress.getBaseReferenceIndex(), 0U);
    EXPECT_FALSE(SymbolAddress.getRegister().has_value());
  }

  // Symbol-relative operands preserve the full u32 reference-index range without reserving high bits for registers.
  TEST(VMOperandTest, PreservesFullSymbolReferenceIndexRange)
  {
    const std::uint32_t Indices[] = {
        0,
        255,
        256,
        std::numeric_limits<std::uint32_t>::max(),
    };
    for (const auto Index : Indices)
    {
      SCOPED_TRACE(Index);
      const AddressOperand Address(Index, 8);
      EXPECT_EQ(Address.getBaseKind(), AddressBaseKind::Symbol);
      EXPECT_EQ(Address.getBaseReferenceIndex(), Index);
      EXPECT_EQ(Address.getByteOffset(), 8);
      EXPECT_FALSE(Address.getRegister().has_value());
    }
  }

  // Both address forms retain negative displacements and both signed 64-bit offset boundaries exactly.
  TEST(VMOperandTest, PreservesSignedAddressOffsetBoundaries)
  {
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
      const AddressOperand SymbolAddress(std::uint32_t{5}, Offset);
      EXPECT_EQ(RegisterAddress.getByteOffset(), Offset);
      EXPECT_EQ(SymbolAddress.getByteOffset(), Offset);
      EXPECT_EQ(RegisterAddress.getRegister(), RegisterId::SP);
      EXPECT_EQ(SymbolAddress.getBaseReferenceIndex(), 5U);
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
      EXPECT_EQ(Immediate.bits(), Value);
    }
  }

  // Concrete value copies preserve their payloads, while base assignment cannot overwrite a concrete operand's kind.
  TEST(VMOperandTest, CopiesConcreteValuesWithoutExposingBaseAssignment)
  {
    static_assert(!std::is_assignable_v<Operand &, const Operand &>);
    static_assert(std::is_copy_constructible_v<RegisterOperand> && std::is_copy_assignable_v<RegisterOperand>);
    static_assert(std::is_copy_constructible_v<AddressOperand> && std::is_copy_assignable_v<AddressOperand>);
    static_assert(std::is_copy_constructible_v<ImmediateOperand> && std::is_copy_assignable_v<ImmediateOperand>);
    const RegisterOperand Register(RegisterId::BP);
    auto RegisterCopy = Register;
    RegisterCopy = RegisterOperand(RegisterId::SP);
    EXPECT_EQ(Register.getRegister(), RegisterId::BP);
    EXPECT_EQ(RegisterCopy.getRegister(), RegisterId::SP);
    EXPECT_EQ(RegisterCopy.getKind(), OperandKind::Register);
    const AddressOperand Address(RegisterId::BP, -16);
    auto AddressCopy = Address;
    AddressCopy = AddressOperand(std::uint32_t{7}, 32);
    EXPECT_EQ(Address.getRegister(), RegisterId::BP);
    EXPECT_EQ(Address.getByteOffset(), -16);
    EXPECT_EQ(AddressCopy.getBaseReferenceIndex(), 7U);
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
