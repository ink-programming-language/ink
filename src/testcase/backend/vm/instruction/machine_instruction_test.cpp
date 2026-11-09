#include "ink/backend/vm/instruction/machine_instructions.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#if defined(OPCODE) || defined(OPCODE_FORM) || defined(INK_VM_OPCODE_DEF_UNDEF_FORM)
#error "VM instruction headers must not leak opcode definition macros"
#endif

namespace ink::vm::test
{
  namespace
  {
    template <typename OperandType>
    constexpr OperandType makeInstructionTestOperand(std::size_t Index)
    {
      if constexpr (std::is_same_v<OperandType, RegisterOperand>)
      {
        return RegisterOperand(static_cast<RegisterId>(Index % 3));
      }
      else if constexpr (std::is_same_v<OperandType, AddressOperand>)
      {
        return AddressOperand(RegisterId::BP, static_cast<std::int64_t>(16 + Index));
      }
      else
      {
        static_assert(std::is_same_v<OperandType, ImmediateOperand>);
        return ImmediateOperand(64 + Index);
      }
    }

    template <Opcode Operation, std::size_t... Indices>
    constexpr MachineInstructionImpl<Operation> makeInstructionTestValue(std::index_sequence<Indices...>)
    {
      using OperandTuple = typename OpcodeTraits<Operation>::OperandTuple;
      return MachineInstructionImpl<Operation>(makeInstructionTestOperand<std::tuple_element_t<Indices, OperandTuple>>(Indices)...);
    }

    constexpr std::array<OperandKind, 3> expectedInstructionOperandKinds(InstructionForm Form)
    {
      switch (Form)
      {
      case InstructionForm::Mm:
        return {OperandKind::Address, OperandKind::Address, OperandKind::Immediate};
      case InstructionForm::Mi:
        return {OperandKind::Address, OperandKind::Immediate, OperandKind::Immediate};
      case InstructionForm::Mmm:
        return {OperandKind::Address, OperandKind::Address, OperandKind::Address};
      case InstructionForm::Mmi:
        return {OperandKind::Address, OperandKind::Address, OperandKind::Immediate};
      case InstructionForm::Mii:
        return {OperandKind::Address, OperandKind::Immediate, OperandKind::Immediate};
      case InstructionForm::Rrr:
        return {OperandKind::Register, OperandKind::Register, OperandKind::Register};
      case InstructionForm::Rrm:
        return {OperandKind::Register, OperandKind::Register, OperandKind::Address};
      case InstructionForm::Rri:
        return {OperandKind::Register, OperandKind::Register, OperandKind::Immediate};
      case InstructionForm::Mrr:
        return {OperandKind::Address, OperandKind::Register, OperandKind::Register};
      case InstructionForm::Mrm:
        return {OperandKind::Address, OperandKind::Register, OperandKind::Address};
      case InstructionForm::Mri:
        return {OperandKind::Address, OperandKind::Register, OperandKind::Immediate};
      }
      return {};
    }

    template <Opcode Operation>
    void expectRegisteredInstruction(std::uint16_t Encoding, std::string_view Name, InstructionForm Form, std::uint32_t Width, std::size_t RegisterCount, std::size_t AddressCount, std::size_t ImmediateCount)
    {
      using Traits = OpcodeTraits<Operation>;
      constexpr std::size_t OperandCount = std::tuple_size_v<typename Traits::OperandTuple>;
      const auto Value = makeInstructionTestValue<Operation>(std::make_index_sequence<OperandCount>{});
      const MachineInstruction &Base = Value;
      SCOPED_TRACE(Name);
      EXPECT_EQ(static_cast<std::uint16_t>(Value.getOpcode()), Encoding);
      EXPECT_EQ(Base.getOpcode(), Operation);
      EXPECT_EQ(Base.getName(), Name);
      EXPECT_EQ(Base.bitWidth(), Width);
      EXPECT_EQ(Traits::Form, Form);
      EXPECT_EQ(Traits::BitWidth, Width);
      EXPECT_EQ(Traits::RegisterCount, RegisterCount);
      EXPECT_EQ(Traits::AddressCount, AddressCount);
      EXPECT_EQ(Traits::ImmediateCount, ImmediateCount);
      EXPECT_EQ(Traits::OperandCount, OperandCount);
      EXPECT_EQ(Base.operandCount(), RegisterCount + AddressCount + ImmediateCount);
      EXPECT_TRUE(MachineInstructionImpl<Operation>::classof(&Base));
      EXPECT_FALSE(MachineInstructionImpl<Operation>::classof(nullptr));
      std::array<const Operand *, OperandCount> NamedOperands{};
      if constexpr (OperandCount == 2)
      {
        NamedOperands = {&Value.getDestination(), &Value.getSource()};
      }
      else
      {
        static_assert(OperandCount == 3);
        NamedOperands = {&Value.getDestination(), &Value.getLHS(), &Value.getRHS()};
      }
      const auto Kinds = expectedInstructionOperandKinds(Form);
      for (std::size_t Index = 0; Index < OperandCount; ++Index)
      {
        SCOPED_TRACE(Index);
        ASSERT_NE(Base.getOperand(Index), nullptr);
        EXPECT_EQ(Base.getOperand(Index), NamedOperands[Index]);
        EXPECT_EQ(Value.getOperand(Index), NamedOperands[Index]);
        EXPECT_EQ(Base.getOperand(Index)->getKind(), Kinds[Index]);
      }
      EXPECT_EQ(Base.getOperand(OperandCount), nullptr);
      EXPECT_EQ(Value.getOperand(OperandCount), nullptr);
    }

    template <typename InstructionType, typename = void>
    inline constexpr bool HasInstructionSource = false;

    template <typename InstructionType>
    inline constexpr bool HasInstructionSource<InstructionType, std::void_t<decltype(std::declval<const InstructionType &>().getSource())>> = true;

    template <typename InstructionType, typename = void>
    inline constexpr bool HasInstructionLHS = false;

    template <typename InstructionType>
    inline constexpr bool HasInstructionLHS<InstructionType, std::void_t<decltype(std::declval<const InstructionType &>().getLHS())>> = true;

    template <typename InstructionType, typename = void>
    inline constexpr bool HasInstructionRHS = false;

    template <typename InstructionType>
    inline constexpr bool HasInstructionRHS<InstructionType, std::void_t<decltype(std::declval<const InstructionType &>().getRHS())>> = true;
  } // namespace

  // Every opcode owns a typed layout whose semantic order agrees with its declared Form and base traversal.
  TEST(VMMachineInstructionTest, ConnectsEveryOpcodeToItsTypedOperandLayout)
  {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount) expectRegisteredInstruction<Opcode::Kind>(Encoding, Spelling, InstructionForm::Form, Width, RegisterCount, AddressCount, ImmediateCount);
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
  }

  // Opcodes with different semantics or widths share a Form whenever their operand type sequence is identical.
  TEST(VMMachineInstructionTest, SharesSignatureLayoutsAcrossOperationsAndWidths)
  {
    static_assert(OpcodeTraits<Opcode::Load64>::Form == InstructionForm::Mm);
    static_assert(OpcodeTraits<Opcode::Store64m>::Form == InstructionForm::Mm);
    static_assert(std::is_same_v<OpcodeTraits<Opcode::Load64>::OperandTuple, OpcodeTraits<Opcode::Store64m>::OperandTuple>);
    static_assert(OpcodeTraits<Opcode::Store8i>::Form == InstructionForm::Mi);
    static_assert(OpcodeTraits<Opcode::Store64i>::Form == InstructionForm::Mi);
    static_assert(std::is_same_v<OpcodeTraits<Opcode::Store8i>::OperandTuple, OpcodeTraits<Opcode::Store64i>::OperandTuple>);
    static_assert(OpcodeTraits<Opcode::Add64rri>::Form == InstructionForm::Rri);
    const MachineLoad64Instruction Load(AddressOperand(RegisterId::BP, 0), AddressOperand(RegisterId::SP, 8));
    const MachineStore64mInstruction Store(AddressOperand(RegisterId::BP, 0), AddressOperand(RegisterId::SP, 8));
    EXPECT_NE(Load.getOpcode(), Store.getOpcode());
    EXPECT_EQ(Load.operandCount(), Store.operandCount());
    EXPECT_EQ(Load.getDestination().getBaseKind(), Store.getDestination().getBaseKind());
    EXPECT_EQ(Load.getSource().getByteOffset(), Store.getSource().getByteOffset());
  }

  // Equal register/address counts cannot exchange the rrm and mrr destination or source positions.
  TEST(VMMachineInstructionTest, DistinguishesOperandOrderForEqualKindCounts)
  {
    const MachineAdd64rrmInstruction RegisterDestination(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::BP), AddressOperand(std::uint32_t{5}, -8));
    const MachineAdd64mrrInstruction MemoryDestination(AddressOperand(std::uint32_t{7}, 16), RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::SP));
    const MachineInstruction &RegisterBase = RegisterDestination;
    const MachineInstruction &MemoryBase = MemoryDestination;
    EXPECT_EQ(RegisterBase.getOperand(0)->getKind(), OperandKind::Register);
    EXPECT_EQ(RegisterBase.getOperand(2)->getKind(), OperandKind::Address);
    EXPECT_EQ(MemoryBase.getOperand(0)->getKind(), OperandKind::Address);
    EXPECT_EQ(MemoryBase.getOperand(2)->getKind(), OperandKind::Register);
    EXPECT_EQ(RegisterDestination.getRHS().getBaseReferenceIndex(), 5U);
    EXPECT_EQ(RegisterDestination.getRHS().getByteOffset(), -8);
    EXPECT_EQ(MemoryDestination.getDestination().getBaseReferenceIndex(), 7U);
    EXPECT_EQ(MemoryDestination.getRHS().getRegister(), RegisterId::SP);
    EXPECT_FALSE(MachineAdd64rrmInstruction::classof(&MemoryBase));
    EXPECT_FALSE(MachineAdd64mrrInstruction::classof(&RegisterBase));
  }

  // Construction rejects missing, extra, raw, or wrongly ordered operands before a runtime instruction exists.
  TEST(VMMachineInstructionTest, RestrictsConstructionToExactOperandKindsAndArity)
  {
    static_assert(std::is_constructible_v<MachineAdd64rriInstruction, RegisterOperand, RegisterOperand, ImmediateOperand>);
    static_assert(std::is_constructible_v<MachineLoad64Instruction, AddressOperand, AddressOperand>);
    static_assert(std::is_constructible_v<MachineStore8iInstruction, const AddressOperand &, const ImmediateOperand &>);
    static_assert(!std::is_default_constructible_v<MachineAdd64rriInstruction>);
    static_assert(!std::is_constructible_v<MachineAdd64rriInstruction, RegisterOperand, RegisterOperand>);
    static_assert(!std::is_constructible_v<MachineAdd64rriInstruction, RegisterOperand, RegisterOperand, ImmediateOperand, ImmediateOperand>);
    static_assert(!std::is_constructible_v<MachineAdd64rriInstruction, RegisterOperand, AddressOperand, ImmediateOperand>);
    static_assert(!std::is_constructible_v<MachineAdd64rriInstruction, RegisterId, RegisterId, std::uint64_t>);
    static_assert(!std::is_constructible_v<MachineAdd64mrrInstruction, RegisterOperand, RegisterOperand, AddressOperand>);
    static_assert(!std::is_constructible_v<MachineStore8iInstruction, RegisterId, ImmediateOperand>);
    static_assert(!std::is_constructible_v<MachineStore8iInstruction, std::uint32_t, ImmediateOperand>);
    static_assert(std::is_final_v<MachineAdd64rriInstruction>);
    static_assert(!std::is_polymorphic_v<MachineInstruction> && !std::is_polymorphic_v<MachineAdd64rriInstruction>);
    static_assert(!std::is_constructible_v<MachineInstruction, Opcode>);
    static_assert(!std::is_copy_constructible_v<MachineAdd64rriInstruction>);
    SUCCEED();
  }

  // Named getters expose read-only concrete operand types and only the source names belonging to their arity.
  TEST(VMMachineInstructionTest, ExposesReadOnlyTypedGettersForEachArity)
  {
    static_assert(std::is_same_v<decltype(std::declval<const MachineAdd64rriInstruction &>().getDestination()), const RegisterOperand &>);
    static_assert(std::is_same_v<decltype(std::declval<const MachineAdd64rriInstruction &>().getLHS()), const RegisterOperand &>);
    static_assert(std::is_same_v<decltype(std::declval<const MachineAdd64rriInstruction &>().getRHS()), const ImmediateOperand &>);
    static_assert(std::is_same_v<decltype(std::declval<const MachineLoad64Instruction &>().getSource()), const AddressOperand &>);
    static_assert(std::is_same_v<decltype(std::declval<const MachineStore8iInstruction &>().getSource()), const ImmediateOperand &>);
    static_assert(HasInstructionSource<MachineLoad64Instruction> && HasInstructionSource<MachineStore8iInstruction>);
    static_assert(!HasInstructionLHS<MachineLoad64Instruction> && !HasInstructionRHS<MachineStore8iInstruction>);
    static_assert(HasInstructionLHS<MachineAdd64rriInstruction> && HasInstructionRHS<MachineAdd64rriInstruction>);
    static_assert(!HasInstructionSource<MachineAdd64rriInstruction>);
    SUCCEED();
  }

  // Concrete instruction construction, metadata, and direct operand access remain usable in constant evaluation.
  TEST(VMMachineInstructionTest, SupportsConstantEvaluationOfConcreteInstructions)
  {
    constexpr MachineAdd64rriInstruction Value(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::SP), ImmediateOperand(16));
    static_assert(Value.getOpcode() == Opcode::Add64rri);
    static_assert(Value.getName() == "add64rri");
    static_assert(Value.bitWidth() == 64 && Value.operandCount() == 3);
    static_assert(Value.getDestination().getRegister() == RegisterId::SP);
    static_assert(Value.getRHS().bits() == 16);
    static_assert(Value.getOperand(0) == &Value.getDestination());
    static_assert(Value.getOperand(1) == &Value.getLHS());
    static_assert(Value.getOperand(2) == &Value.getRHS());
    static_assert(Value.getOperand(3) == nullptr);
    EXPECT_EQ(Value.getOperand(2)->getKind(), OperandKind::Immediate);
  }

  // Instructions own copies of supplied operands and remain valid after caller-owned values are replaced.
  TEST(VMMachineInstructionTest, OwnsOperandValuesIndependentlyOfInputs)
  {
    RegisterOperand Destination(RegisterId::SP);
    RegisterOperand LHS(RegisterId::BP);
    ImmediateOperand RHS(16);
    const MachineAdd64rriInstruction Value(Destination, LHS, RHS);
    Destination = RegisterOperand(RegisterId::PC);
    LHS = RegisterOperand(RegisterId::SP);
    RHS = ImmediateOperand(64);
    EXPECT_EQ(Value.getDestination().getRegister(), RegisterId::SP);
    EXPECT_EQ(Value.getLHS().getRegister(), RegisterId::BP);
    EXPECT_EQ(Value.getRHS().bits(), 16U);
    EXPECT_NE(&Value.getDestination(), &Destination);
    EXPECT_NE(&Value.getLHS(), &LHS);
    EXPECT_NE(&Value.getRHS(), &RHS);
    const MachineStore64iInstruction TemporaryOperands(AddressOperand(RegisterId::BP, -24), ImmediateOperand(42));
    EXPECT_EQ(TemporaryOperands.getDestination().getByteOffset(), -24);
    EXPECT_EQ(TemporaryOperands.getSource().bits(), 42U);
  }

  // Binary and ternary base traversal reject both the first absent slot and the largest possible index.
  TEST(VMMachineInstructionTest, ReturnsNullForOutOfRangeOperandIndices)
  {
    const MachineLoad64Instruction Binary(AddressOperand(RegisterId::BP), AddressOperand(RegisterId::SP, -8));
    const MachineAdd64rrrInstruction Ternary{RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::PC)};
    const MachineInstruction *Values[] = {&Binary, &Ternary};
    for (const auto *Value : Values)
    {
      EXPECT_EQ(Value->getOperand(Value->operandCount()), nullptr);
      EXPECT_EQ(Value->getOperand(std::numeric_limits<std::size_t>::max()), nullptr);
    }
    EXPECT_EQ(Binary.getOperand(std::numeric_limits<std::size_t>::max()), nullptr);
    EXPECT_EQ(Ternary.getOperand(std::numeric_limits<std::size_t>::max()), nullptr);
  }

  // Inline operand storage preserves full symbol indices, signed displacements, and high immediate bits together.
  TEST(VMMachineInstructionTest, PreservesFullOperandPayloadBoundaries)
  {
    const MachineAdd64miiInstruction Value(AddressOperand(std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::int64_t>::min()), ImmediateOperand(std::uint64_t{1} << 63), ImmediateOperand(std::numeric_limits<std::uint64_t>::max()));
    EXPECT_EQ(Value.getDestination().getBaseReferenceIndex(), std::numeric_limits<std::uint32_t>::max());
    EXPECT_EQ(Value.getDestination().getByteOffset(), std::numeric_limits<std::int64_t>::min());
    EXPECT_EQ(Value.getLHS().bits(), std::uint64_t{1} << 63);
    EXPECT_EQ(Value.getRHS().bits(), std::numeric_limits<std::uint64_t>::max());
    const MachineInstruction &Base = Value;
    EXPECT_EQ(static_cast<const AddressOperand *>(Base.getOperand(0))->getBaseReferenceIndex(), std::numeric_limits<std::uint32_t>::max());
    EXPECT_EQ(static_cast<const ImmediateOperand *>(Base.getOperand(2))->bits(), std::numeric_limits<std::uint64_t>::max());
  }
} // namespace ink::vm::test
