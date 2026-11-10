#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/module/machine_basic_block.h"
#include "ink/core/name/name_pool.h"

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
    OperandType makeInstructionTestOperand(std::size_t Index)
    {
      if constexpr (std::is_same_v<OperandType, RegisterOperand>)
      {
        return RegisterOperand(static_cast<RegisterId>(Index % 3));
      }
      else if constexpr (std::is_same_v<OperandType, AddressOperand>)
      {
        return AddressOperand(RegisterId::BP, static_cast<std::int64_t>(16 + Index));
      }
      else if constexpr (std::is_same_v<OperandType, BasicBlockOperand>)
      {
        static const MachineBasicBlock Target;
        return BasicBlockOperand(&Target);
      }
      else
      {
        static_assert(std::is_same_v<OperandType, ImmediateOperand>);
        return ImmediateOperand(64 + Index);
      }
    }

    template <Opcode Operation, std::size_t... Indices>
    MachineInstructionImpl<Operation> makeInstructionTestValue(std::index_sequence<Indices...>)
    {
      using OperandTuple = typename OpcodeTraits<Operation>::OperandTuple;
      return MachineInstructionImpl<Operation>(makeInstructionTestOperand<std::tuple_element_t<Indices, OperandTuple>>(Indices)...);
    }

    constexpr std::array<OperandKind, 3> expectedInstructionOperandKinds(InstructionForm Form)
    {
      switch (Form)
      {
      case InstructionForm::None:
        return {};
      case InstructionForm::B:
        return {OperandKind::BasicBlock, OperandKind::Immediate, OperandKind::Immediate};
      case InstructionForm::I:
        return {OperandKind::Immediate, OperandKind::Immediate, OperandKind::Immediate};
      case InstructionForm::M:
        return {OperandKind::Address, OperandKind::Immediate, OperandKind::Immediate};
      case InstructionForm::Rr:
        return {OperandKind::Register, OperandKind::Register, OperandKind::Immediate};
      case InstructionForm::Rm:
        return {OperandKind::Register, OperandKind::Address, OperandKind::Immediate};
      case InstructionForm::Ri:
        return {OperandKind::Register, OperandKind::Immediate, OperandKind::Immediate};
      case InstructionForm::Mr:
        return {OperandKind::Address, OperandKind::Register, OperandKind::Immediate};
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
    void expectRegisteredInstruction(std::uint16_t Encoding, std::string_view Name, InstructionForm Form, std::uint32_t Width, std::size_t RegisterCount, std::size_t AddressCount, std::size_t ImmediateCount, InstructionCategory Category, ConditionCode Condition)
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
      EXPECT_EQ(Base.operandCount(), RegisterCount + AddressCount + ImmediateCount + Traits::BlockCount);
      EXPECT_EQ(Base.category(), Category);
      EXPECT_EQ(Base.branchCondition(), Condition);
      EXPECT_EQ(Traits::Category, Category);
      EXPECT_EQ(Traits::Condition, Condition);
      EXPECT_EQ(Base.readEFlagsMask(), Traits::ReadEFlagsMask);
      EXPECT_EQ(Base.writtenEFlagsMask(), Traits::WrittenEFlagsMask);
      EXPECT_TRUE(MachineInstructionImpl<Operation>::classof(&Base));
      EXPECT_FALSE(MachineInstructionImpl<Operation>::classof(nullptr));
      std::array<const Operand *, OperandCount> NamedOperands{};
      if constexpr (Traits::HasDestination && OperandCount == 2)
      {
        NamedOperands = {&Value.getDestination(), &Value.getSource()};
      }
      else if constexpr (Traits::HasDestination)
      {
        static_assert(OperandCount == 3);
        NamedOperands = {&Value.getDestination(), &Value.getLHS(), &Value.getRHS()};
      }
      else if constexpr (Traits::Category == InstructionCategory::Compare)
      {
        NamedOperands = {&Value.getLHS(), &Value.getRHS()};
      }
      else if constexpr (Traits::Category == InstructionCategory::Branch || Traits::Category == InstructionCategory::Call)
      {
        NamedOperands = {&Value.getTargetOperand()};
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

  template <typename InstructionType>
  concept HasInstructionDestination = requires(const InstructionType &Value) {
    Value.getDestination();
  };

  template <typename InstructionType>
  concept HasInstructionTarget = requires(const InstructionType &Value) {
    Value.getTarget();
  };

  // Every opcode owns a typed layout whose semantic order agrees with its declared Form and base traversal.
  TEST(VMMachineInstructionTest, ConnectsEveryOpcodeToItsTypedOperandLayout)
  {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Condition) expectRegisteredInstruction<Opcode::Kind>(Encoding, Spelling, InstructionForm::Form, Width, RegisterCount, AddressCount, ImmediateCount, InstructionCategory::Category, ConditionCode::Condition);
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
    const core::Name SourceSymbol = core::NamePool::instance().intern("instruction.source");
    const core::Name DestinationSymbol = core::NamePool::instance().intern("instruction.destination");
    const MachineAdd64rrmInstruction RegisterDestination(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::BP), AddressOperand(SourceSymbol, -8));
    const MachineAdd64mrrInstruction MemoryDestination(AddressOperand(DestinationSymbol, 16), RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::SP));
    const MachineInstruction &RegisterBase = RegisterDestination;
    const MachineInstruction &MemoryBase = MemoryDestination;
    EXPECT_EQ(RegisterBase.getOperand(0)->getKind(), OperandKind::Register);
    EXPECT_EQ(RegisterBase.getOperand(2)->getKind(), OperandKind::Address);
    EXPECT_EQ(MemoryBase.getOperand(0)->getKind(), OperandKind::Address);
    EXPECT_EQ(MemoryBase.getOperand(2)->getKind(), OperandKind::Register);
    EXPECT_EQ(RegisterDestination.getRHS().getSymbol(), SourceSymbol);
    EXPECT_EQ(RegisterDestination.getRHS().getByteOffset(), -8);
    EXPECT_EQ(MemoryDestination.getDestination().getSymbol(), DestinationSymbol);
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

  // Inline operand storage preserves symbol names, signed displacement boundaries, and high immediate bits together.
  TEST(VMMachineInstructionTest, PreservesFullOperandPayloadBoundaries)
  {
    const core::Name Symbol = core::NamePool::instance().intern("instruction.boundaries");
    const MachineAdd64miiInstruction Value(AddressOperand(Symbol, std::numeric_limits<std::int64_t>::min()), ImmediateOperand(std::uint64_t{1} << 63), ImmediateOperand(std::numeric_limits<std::uint64_t>::max()));
    EXPECT_EQ(Value.getDestination().getSymbol(), Symbol);
    EXPECT_EQ(Value.getDestination().getByteOffset(), std::numeric_limits<std::int64_t>::min());
    EXPECT_EQ(Value.getLHS().bits(), std::uint64_t{1} << 63);
    EXPECT_EQ(Value.getRHS().bits(), std::numeric_limits<std::uint64_t>::max());
    const MachineInstruction &Base = Value;
    EXPECT_EQ(static_cast<const AddressOperand *>(Base.getOperand(0))->getSymbol(), Symbol);
    EXPECT_EQ(static_cast<const ImmediateOperand *>(Base.getOperand(2))->bits(), std::numeric_limits<std::uint64_t>::max());
  }

  // ADD distinguishes reading symbol-relative memory from adding the same symbol's address as an immediate value.
  TEST(VMMachineInstructionTest, DistinguishesSymbolAddressesFromMemoryReads)
  {
    const core::Name Symbol = core::NamePool::instance().intern("instruction.counter");
    const AddressOperand Destination(RegisterId::BP, 16);
    const MachineAdd64mmiInstruction ReadValue(Destination, AddressOperand(Symbol, 8), ImmediateOperand(1));
    const MachineAdd64miiInstruction UseAddress(Destination, ImmediateOperand(Symbol, 8), ImmediateOperand(1));
    EXPECT_EQ(ReadValue.getLHS().getKind(), OperandKind::Address);
    EXPECT_EQ(ReadValue.getLHS().getSymbol(), Symbol);
    EXPECT_EQ(ReadValue.getLHS().getByteOffset(), 8);
    EXPECT_EQ(UseAddress.getLHS().getKind(), OperandKind::Immediate);
    EXPECT_EQ(UseAddress.getLHS().getValueKind(), ImmediateValueKind::Symbol);
    EXPECT_EQ(UseAddress.getLHS().getSymbol(), Symbol);
    EXPECT_EQ(UseAddress.getLHS().getByteOffset(), 8);
    EXPECT_FALSE(UseAddress.getLHS().bits().has_value());
    EXPECT_EQ(UseAddress.getRHS().bits(), 1U);
  }
  // Compare operands are ordered sources rather than a writable destination, and their full width controls flags.
  TEST(VMMachineInstructionTest, ModelsCompareAsTwoSourcesAndImplicitFlags)
  {
    const MachineCmp32miInstruction Compare(AddressOperand(RegisterId::BP, 4), ImmediateOperand(42));
    EXPECT_EQ(Compare.getLHS().getByteOffset(), 4);
    EXPECT_EQ(Compare.getRHS().bits(), 42U);
    EXPECT_TRUE(Compare.isCompare());
    EXPECT_FALSE(Compare.hasDestination());
    EXPECT_FALSE(Compare.isTerminator());
    EXPECT_EQ(Compare.readEFlagsMask(), 0U);
    EXPECT_EQ(Compare.writtenEFlagsMask(), ArithmeticEFlagsMask);
    static_assert(!HasInstructionDestination<MachineCmp32miInstruction>);
    static_assert(!HasInstructionSource<MachineCmp32miInstruction>);
    static_assert(HasInstructionLHS<MachineCmp32miInstruction> && HasInstructionRHS<MachineCmp32miInstruction>);
    static_assert(std::is_same_v<decltype(Compare.getLHS()), const AddressOperand &>);
    static_assert(std::is_same_v<decltype(Compare.getRHS()), const ImmediateOperand &>);
    static_assert(OpcodeTraits<Opcode::Cmp64rr>::Form == InstructionForm::Rr);
    static_assert(OpcodeTraits<Opcode::Cmp64mr>::Form == InstructionForm::Mr);
  }

  // Branches borrow block pointers independently of byte layout and read only the flags needed by their condition.
  TEST(VMMachineInstructionTest, ModelsBranchTargetAndImplicitFlagDependencies)
  {
    static const MachineBasicBlock Target;
    constexpr MachineJmpInstruction Jump(&Target);
    constexpr MachineJeInstruction Equal{BasicBlockOperand(&Target)};
    constexpr MachineJlInstruction Less(&Target);
    constexpr MachineJgInstruction Greater(&Target);
    static_assert(OpcodeTraits<Opcode::Jmp>::Form == InstructionForm::B);
    static_assert(OpcodeTraits<Opcode::Jmp>::AddressCount == 0);
    static_assert(OpcodeTraits<Opcode::Jmp>::BlockCount == 1);
    static_assert(std::is_same_v<decltype(Jump.getTarget()), const MachineBasicBlock *>);
    static_assert(std::is_same_v<decltype(Jump.getTargetOperand()), const BasicBlockOperand &>);
    static_assert(Jump.getTarget() == &Target);
    EXPECT_EQ(Jump.getTargetOperand().getKind(), OperandKind::BasicBlock);
    EXPECT_EQ(Equal.getTarget(), &Target);
    EXPECT_EQ(Jump.getOperand(0), &Jump.getTargetOperand());
    EXPECT_TRUE(Jump.isBranch());
    EXPECT_TRUE(Jump.isTerminator());
    EXPECT_FALSE(Jump.isConditionalBranch());
    EXPECT_EQ(Jump.branchCondition(), ConditionCode::Always);
    EXPECT_EQ(Jump.readEFlagsMask(), 0U);
    EXPECT_EQ(Jump.writtenEFlagsMask(), 0U);
    EXPECT_TRUE(Equal.isConditionalBranch());
    EXPECT_EQ(Equal.readEFlagsMask(), eflagMask(EFlag::ZF));
    EXPECT_EQ(Less.readEFlagsMask(), eflagMask(EFlag::SF) | eflagMask(EFlag::OF));
    EXPECT_EQ(Greater.readEFlagsMask(), eflagMask(EFlag::ZF) | eflagMask(EFlag::SF) | eflagMask(EFlag::OF));
    static_assert(HasInstructionTarget<MachineJmpInstruction>);
    static_assert(!HasInstructionDestination<MachineJmpInstruction>);
    static_assert(!HasInstructionSource<MachineJmpInstruction> && !HasInstructionLHS<MachineJmpInstruction> && !HasInstructionRHS<MachineJmpInstruction>);
    static_assert(!std::is_constructible_v<MachineJmpInstruction, ImmediateOperand>);
    static_assert(!std::is_constructible_v<MachineJmpInstruction, RegisterOperand>);
    static_assert(!std::is_constructible_v<MachineJmpInstruction, AddressOperand>);
    static_assert(std::is_constructible_v<MachineJmpInstruction, const MachineBasicBlock *>);
    static_assert(std::is_constructible_v<MachineJmpInstruction, MachineBasicBlock *>);
    static_assert(std::is_constructible_v<MachineJmpInstruction, std::nullptr_t>);
    constexpr MachineJmpInstruction Unresolved(nullptr);
    static_assert(Unresolved.getTarget() == nullptr);
  }

  // RET is a zero-operand terminator with no destination or target and preserves EFLAGS.
  TEST(VMMachineInstructionTest, SupportsZeroOperandReturn)
  {
    constexpr MachineRetInstruction Return;
    static_assert(Return.getOpcode() == Opcode::Ret);
    static_assert(Return.getName() == "ret");
    static_assert(Return.operandCount() == 0 && Return.bitWidth() == 0);
    static_assert(Return.getOperand(0) == nullptr);
    static_assert(Return.isReturn() && Return.isTerminator() && !Return.isBranch());
    static_assert(Return.readEFlagsMask() == 0 && Return.writtenEFlagsMask() == 0);
    static_assert(!HasInstructionDestination<MachineRetInstruction> && !HasInstructionTarget<MachineRetInstruction>);
    static_assert(!HasInstructionSource<MachineRetInstruction> && !HasInstructionLHS<MachineRetInstruction> && !HasInstructionRHS<MachineRetInstruction>);
    static_assert(!std::is_constructible_v<MachineRetInstruction, ImmediateOperand>);
    const MachineInstruction &Base = Return;
    EXPECT_EQ(Base.getOperand(0), nullptr);
    EXPECT_EQ(Base.getOperand(std::numeric_limits<std::size_t>::max()), nullptr);
  }

  // Immediate and memory calls reuse value operands, distinguish VM and native targets, and clobber flags without terminating the block.
  TEST(VMMachineInstructionTest, ModelsImmediateAndIndirectVMAndNativeCalls)
  {
    const core::Name Target = core::NamePool::instance().intern("instruction.call.target");
    const MachineCalliInstruction Direct{ImmediateOperand(Target)};
    const MachineCallNativeiInstruction NativeDirect{ImmediateOperand(Target)};
    constexpr MachineCallmInstruction Indirect{AddressOperand(RegisterId::SP)};
    constexpr MachineCallNativemInstruction NativeIndirect{AddressOperand(RegisterId::SP, 8)};
    const MachineInstruction *Calls[] = {&Direct, &NativeDirect, &Indirect, &NativeIndirect};
    for (const auto *Value : Calls)
    {
      EXPECT_TRUE(Value->isCall());
      EXPECT_FALSE(Value->isTerminator());
      EXPECT_FALSE(Value->isBranch());
      EXPECT_FALSE(Value->isReturn());
      EXPECT_FALSE(Value->hasDestination());
      EXPECT_EQ(Value->category(), InstructionCategory::Call);
      EXPECT_EQ(Value->operandCount(), 1U);
      EXPECT_EQ(Value->bitWidth(), Value->getOpcode() == Opcode::CallNativei ? 32U : 64U);
      EXPECT_EQ(Value->branchCondition(), ConditionCode::None);
      EXPECT_EQ(Value->readEFlagsMask(), 0U);
      EXPECT_EQ(Value->writtenEFlagsMask(), ArithmeticEFlagsMask);
      ASSERT_NE(Value->getOperand(0), nullptr);
      EXPECT_EQ(Value->getOperand(1), nullptr);
    }
    EXPECT_EQ(Direct.getName(), "call");
    EXPECT_EQ(Indirect.getName(), "call");
    EXPECT_EQ(NativeDirect.getName(), "callNative");
    EXPECT_EQ(NativeIndirect.getName(), "callNative");
    EXPECT_FALSE(Direct.isNativeCall());
    EXPECT_FALSE(Indirect.isNativeCall());
    EXPECT_TRUE(NativeDirect.isNativeCall());
    EXPECT_TRUE(NativeIndirect.isNativeCall());
    EXPECT_EQ(Direct.getTarget().getSymbol(), Target);
    EXPECT_EQ(NativeDirect.getTarget().getSymbol(), Target);
    EXPECT_EQ(Direct.getTargetOperand().getKind(), OperandKind::Immediate);
    EXPECT_EQ(Indirect.getTargetOperand().getKind(), OperandKind::Address);
    EXPECT_EQ(Direct.getOperand(0), &Direct.getTarget());
    EXPECT_EQ(NativeDirect.getOperand(0), &NativeDirect.getTargetOperand());
    EXPECT_EQ(Indirect.getOperand(0), &Indirect.getTarget());
    EXPECT_EQ(NativeIndirect.getOperand(0), &NativeIndirect.getTargetOperand());
    static_assert(Indirect.getTarget().getRegister() == RegisterId::SP);
    static_assert(Indirect.getTarget().getByteOffset() == 0);
    static_assert(NativeIndirect.getTarget().getByteOffset() == 8);
    static_assert(OpcodeTraits<Opcode::Calli>::Form == InstructionForm::I);
    static_assert(OpcodeTraits<Opcode::CallNativei>::ImmediateCount == 1);
    static_assert(OpcodeTraits<Opcode::Callm>::Form == InstructionForm::M);
    static_assert(OpcodeTraits<Opcode::CallNativem>::AddressCount == 1);
    static_assert(std::is_same_v<decltype(Direct.getTarget()), const ImmediateOperand &>);
    static_assert(std::is_same_v<decltype(Indirect.getTarget()), const AddressOperand &>);
    static_assert(!HasInstructionDestination<MachineCalliInstruction> && !HasInstructionSource<MachineCallNativemInstruction>);
    static_assert(!std::is_constructible_v<MachineCalliInstruction, AddressOperand>);
    static_assert(!std::is_constructible_v<MachineCallmInstruction, ImmediateOperand>);
    static_assert(!std::is_constructible_v<MachineCallNativeiInstruction, core::Name>);
    static_assert(!std::is_constructible_v<MachineCallNativemInstruction, RegisterOperand>);
    static_assert(!std::is_constructible_v<MachineCallNativeiInstruction, std::uint32_t>);
    static_assert(!std::is_constructible_v<MachineCalliInstruction, ImmediateOperand, ImmediateOperand>);
    constexpr MachineCalliInstruction Numeric{ImmediateOperand(std::numeric_limits<std::uint64_t>::max())};
    static_assert(Numeric.getTarget().bits() == std::numeric_limits<std::uint64_t>::max());
    static_assert(Numeric.isCall() && !Numeric.isNativeCall());
  }

  // Existing ADD forms define the arithmetic flags, while data movement preserves them.
  TEST(VMMachineInstructionTest, DescribesArithmeticAndMovementFlagEffects)
  {
    constexpr MachineAdd64rriInstruction Add(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::SP), ImmediateOperand(8));
    constexpr MachineStore8iInstruction Store(AddressOperand(RegisterId::BP), ImmediateOperand(1));
    constexpr MachineLoad8Instruction Load{AddressOperand(RegisterId::BP), AddressOperand(RegisterId::SP)};
    static_assert(Add.hasDestination() && Add.writtenEFlagsMask() == ArithmeticEFlagsMask && Add.readEFlagsMask() == 0);
    static_assert(Store.writtenEFlagsMask() == 0 && Store.readEFlagsMask() == 0);
    static_assert(Load.writtenEFlagsMask() == 0 && Load.readEFlagsMask() == 0);
    EXPECT_FALSE(Add.isTerminator());
  }
} // namespace ink::vm::test
