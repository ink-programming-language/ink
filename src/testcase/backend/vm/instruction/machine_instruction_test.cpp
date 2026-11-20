#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/module/machine_basic_block.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#if defined(OPCODE) || defined(OPCODE_FORM) || defined(INK_VM_OPCODE_DEF_UNDEF_FORM) || defined(COMPARE_BRANCH) || defined(INK_COMPARE_BRANCH_Mmb) || defined(INK_COMPARE_BRANCH_Mib) || defined(INTEGER_COMPARE) || defined(INK_INTEGER_COMPARE_Rrr) || defined(INK_INTEGER_COMPARE_Rri) || defined(REGISTER_COMPARE_BRANCH) || defined(INK_REGISTER_COMPARE_BRANCH_Rrb) || defined(INK_REGISTER_COMPARE_BRANCH_Rib) || defined(FLOAT_COMPARE)
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
                return RegisterOperand(static_cast<RegisterId>(static_cast<std::uint8_t>(RegisterId::R0) + Index));
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
            case InstructionForm::R:
                return {OperandKind::Register, OperandKind::Immediate, OperandKind::Immediate};
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
            case InstructionForm::Mmb:
                return {OperandKind::Address, OperandKind::Address, OperandKind::BasicBlock};
            case InstructionForm::Mib:
                return {OperandKind::Address, OperandKind::Immediate, OperandKind::BasicBlock};
            case InstructionForm::Rrb:
                return {OperandKind::Register, OperandKind::Register, OperandKind::BasicBlock};
            case InstructionForm::Rib:
                return {OperandKind::Register, OperandKind::Immediate, OperandKind::BasicBlock};
            }
            return {};
        }

        template <Opcode Operation>
        void expectRegisteredInstruction(std::uint16_t Encoding, std::string_view Name, InstructionForm Form, std::uint32_t Width, std::size_t RegisterCount, std::size_t AddressCount, std::size_t ImmediateCount, InstructionCategory Category, InstructionDomain Domain)
        {
            using Traits = OpcodeTraits<Operation>;
            constexpr std::size_t OperandCount = std::tuple_size_v<typename Traits::OperandTuple>;
            const auto Value = makeInstructionTestValue<Operation>(std::make_index_sequence<OperandCount>{});
            const MachineInstruction& Base = Value;
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
            EXPECT_EQ(Traits::Category, Category);
            EXPECT_EQ(Base.domain(), Domain);
            EXPECT_EQ(Traits::Domain, Domain);
            EXPECT_EQ(Base.isFloatingPoint(), Domain == InstructionDomain::FloatingPoint);
            EXPECT_EQ(Traits::IsFloatingPoint, Domain == InstructionDomain::FloatingPoint);
            EXPECT_EQ(Base.isConditionalBranch(), Category == InstructionCategory::ConditionalBranch);
            EXPECT_EQ(Base.isBranch(), Category == InstructionCategory::Branch || Category == InstructionCategory::ConditionalBranch);
            EXPECT_EQ(Base.isTerminator(), Category == InstructionCategory::Branch || Category == InstructionCategory::ConditionalBranch || Category == InstructionCategory::Return);
            EXPECT_TRUE(MachineInstructionImpl<Operation>::classof(&Base));
            EXPECT_FALSE(MachineInstructionImpl<Operation>::classof(nullptr));
            std::array<const Operand*, OperandCount> NamedOperands{};
            if constexpr (Traits::HasDestination && OperandCount == 2)
            {
                NamedOperands = {&Value.getDestination(), &Value.getSource()};
            }
            else if constexpr (Traits::HasDestination)
            {
                static_assert(OperandCount == 3);
                NamedOperands = {&Value.getDestination(), &Value.getLHS(), &Value.getRHS()};
            }
            else if constexpr (Traits::Category == InstructionCategory::ConditionalBranch)
            {
                NamedOperands = {&Value.getLHS(), &Value.getRHS(), &Value.getTargetOperand()};
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
        inline constexpr bool HasInstructionSource<InstructionType, std::void_t<decltype(std::declval<const InstructionType&>().getSource())>> = true;

        template <typename InstructionType, typename = void>
        inline constexpr bool HasInstructionLHS = false;

        template <typename InstructionType>
        inline constexpr bool HasInstructionLHS<InstructionType, std::void_t<decltype(std::declval<const InstructionType&>().getLHS())>> = true;

        template <typename InstructionType, typename = void>
        inline constexpr bool HasInstructionRHS = false;

        template <typename InstructionType>
        inline constexpr bool HasInstructionRHS<InstructionType, std::void_t<decltype(std::declval<const InstructionType&>().getRHS())>> = true;
    } // namespace

    template <typename InstructionType>
    concept HasInstructionDestination = requires(const InstructionType& Value) {
        Value.getDestination();
    };

    template <typename InstructionType>
    concept HasInstructionTarget = requires(const InstructionType& Value) {
        Value.getTarget();
    };

    template <typename InstructionType>
    concept HasLegacyFlagState = requires(const InstructionType& Value) {
        Value.branchCondition();
    } || requires(const InstructionType& Value) {
        Value.readEFlagsMask();
    } || requires(const InstructionType& Value) {
        Value.writtenEFlagsMask();
    } || requires(const InstructionType& Value) {
        Value.isCompare();
    };

    template <typename Traits>
    concept HasLegacyConditionMetadata = requires {
        Traits::Condition;
    } || requires {
        Traits::ReadEFlagsMask;
    } || requires {
        Traits::WrittenEFlagsMask;
    };

    // Every opcode owns a typed layout whose semantic order agrees with its declared Form and base traversal.
    TEST(VMMachineInstructionTest, ConnectsEveryOpcodeToItsTypedOperandLayout)
    {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) expectRegisteredInstruction<Opcode::Kind>(Encoding, Spelling, InstructionForm::Form, Width, RegisterCount, AddressCount, ImmediateCount, InstructionCategory::Category, InstructionDomain::Domain);
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
        const MachineInstruction& RegisterBase = RegisterDestination;
        const MachineInstruction& MemoryBase = MemoryDestination;
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
        static_assert(std::is_constructible_v<MachineStore8iInstruction, const AddressOperand&, const ImmediateOperand&>);
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
        static_assert(std::is_same_v<decltype(std::declval<const MachineAdd64rriInstruction&>().getDestination()), const RegisterOperand&>);
        static_assert(std::is_same_v<decltype(std::declval<const MachineAdd64rriInstruction&>().getLHS()), const RegisterOperand&>);
        static_assert(std::is_same_v<decltype(std::declval<const MachineAdd64rriInstruction&>().getRHS()), const ImmediateOperand&>);
        static_assert(std::is_same_v<decltype(std::declval<const MachineLoad64Instruction&>().getSource()), const AddressOperand&>);
        static_assert(std::is_same_v<decltype(std::declval<const MachineStore8iInstruction&>().getSource()), const ImmediateOperand&>);
        static_assert(HasInstructionSource<MachineLoad64Instruction> && HasInstructionSource<MachineStore8iInstruction>);
        static_assert(!HasInstructionLHS<MachineLoad64Instruction> && !HasInstructionRHS<MachineStore8iInstruction>);
        static_assert(HasInstructionLHS<MachineAdd64rriInstruction> && HasInstructionRHS<MachineAdd64rriInstruction>);
        static_assert(!HasInstructionSource<MachineAdd64rriInstruction>);
        static_assert(HasInstructionDestination<MachineFadd64rrrInstruction> && HasInstructionDestination<MachineFsqrt64rrInstruction>);
        static_assert(HasInstructionLHS<MachineFadd64rrrInstruction> && HasInstructionRHS<MachineFadd64rrrInstruction>);
        static_assert(!HasInstructionSource<MachineFadd64rrrInstruction>);
        static_assert(HasInstructionSource<MachineFsqrt64rrInstruction>);
        static_assert(!HasInstructionLHS<MachineFsqrt64rrInstruction> && !HasInstructionRHS<MachineFsqrt64rrInstruction>);
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
        const MachineInstruction* Values[] = {&Binary, &Ternary};
        for (const auto* Value : Values)
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
        const MachineInstruction& Base = Value;
        EXPECT_EQ(static_cast<const AddressOperand*>(Base.getOperand(0))->getSymbol(), Symbol);
        EXPECT_EQ(static_cast<const ImmediateOperand*>(Base.getOperand(2))->bits(), std::numeric_limits<std::uint64_t>::max());
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

    // Block jumps retain their target identity independently of executable byte layout and expose no value destination.
    TEST(VMMachineInstructionTest, ModelsBlockPointerJumpTargets)
    {
        static const MachineBasicBlock Target;
        constexpr MachineJmpInstruction Jump(&Target);
        constexpr MachineJmpInstruction Wrapped{BasicBlockOperand(&Target)};
        static_assert(OpcodeTraits<Opcode::Jmp>::Form == InstructionForm::B);
        static_assert(OpcodeTraits<Opcode::Jmp>::BlockCount == 1);
        static_assert(std::is_same_v<decltype(Jump.getTarget()), const MachineBasicBlock*>);
        static_assert(std::is_same_v<decltype(Jump.getTargetOperand()), const BasicBlockOperand&>);
        static_assert(Jump.getTarget() == &Target && Wrapped.getTarget() == &Target);
        EXPECT_EQ(Jump.getOperand(0), &Jump.getTargetOperand());
        EXPECT_EQ(Jump.getTargetOperand().getKind(), OperandKind::BasicBlock);
        EXPECT_TRUE(Jump.isBranch());
        EXPECT_TRUE(Jump.isTerminator());
        static_assert(!HasInstructionDestination<MachineJmpInstruction>);
        static_assert(!HasInstructionSource<MachineJmpInstruction> && !HasInstructionLHS<MachineJmpInstruction> && !HasInstructionRHS<MachineJmpInstruction>);
        static_assert(!std::is_constructible_v<MachineJmpInstruction, ImmediateOperand>);
        static_assert(!std::is_constructible_v<MachineJmpInstruction, RegisterOperand>);
        static_assert(!std::is_constructible_v<MachineJmpInstruction, AddressOperand>);
        static_assert(std::is_constructible_v<MachineJmpInstruction, std::nullptr_t>);
        constexpr MachineJmpInstruction Unresolved(nullptr);
        static_assert(Unresolved.getTarget() == nullptr);
    }

    // Explicit jump forms preserve immediate bits and signed register or memory displacements without accepting block-pointer construction.
    TEST(VMMachineInstructionTest, ModelsImmediateRegisterAndMemoryJumpTargets)
    {
        constexpr MachineJmpiInstruction Immediate{ImmediateOperand(std::numeric_limits<std::uint64_t>::max())};
        constexpr MachineJmprInstruction Register{RegisterOperand(RegisterId::PC, -11)};
        constexpr MachineJmpmInstruction Memory{AddressOperand(RegisterId::BP, 24)};
        static_assert(OpcodeTraits<Opcode::Jmpi>::Form == InstructionForm::I);
        static_assert(OpcodeTraits<Opcode::Jmpr>::Form == InstructionForm::R);
        static_assert(OpcodeTraits<Opcode::Jmpm>::Form == InstructionForm::M);
        static_assert(OpcodeTraits<Opcode::Jmpi>::BlockCount == 0 && OpcodeTraits<Opcode::Jmpi>::ImmediateCount == 1);
        static_assert(OpcodeTraits<Opcode::Jmpr>::BlockCount == 0 && OpcodeTraits<Opcode::Jmpr>::RegisterCount == 1);
        static_assert(OpcodeTraits<Opcode::Jmpm>::BlockCount == 0 && OpcodeTraits<Opcode::Jmpm>::AddressCount == 1);
        static_assert(std::is_same_v<decltype(Immediate.getTarget()), const ImmediateOperand&>);
        static_assert(std::is_same_v<decltype(Register.getTarget()), const RegisterOperand&>);
        static_assert(std::is_same_v<decltype(Memory.getTarget()), const AddressOperand&>);
        static_assert(Immediate.getTarget().bits() == std::numeric_limits<std::uint64_t>::max());
        static_assert(Register.getTarget().getRegister() == RegisterId::PC && Register.getTarget().getByteOffset() == -11);
        static_assert(Memory.getTarget().getRegister() == RegisterId::BP && Memory.getTarget().getByteOffset() == 24);
        static_assert(!std::is_constructible_v<MachineJmpiInstruction, const MachineBasicBlock*>);
        static_assert(!std::is_constructible_v<MachineJmprInstruction, const MachineBasicBlock*>);
        static_assert(!std::is_constructible_v<MachineJmpmInstruction, const MachineBasicBlock*>);
        static_assert(!std::is_constructible_v<MachineJmpiInstruction, RegisterOperand>);
        static_assert(!std::is_constructible_v<MachineJmprInstruction, AddressOperand>);
        static_assert(!std::is_constructible_v<MachineJmpmInstruction, ImmediateOperand>);
        const MachineInstruction* Jumps[] = {&Immediate, &Register, &Memory};
        for (const auto* Jump : Jumps)
        {
            EXPECT_TRUE(Jump->isBranch());
            EXPECT_TRUE(Jump->isTerminator());
            EXPECT_FALSE(Jump->isCall());
            EXPECT_FALSE(Jump->hasDestination());
            EXPECT_EQ(Jump->bitWidth(), 64U);
            EXPECT_EQ(Jump->operandCount(), 1U);
            EXPECT_EQ(Jump->getOperand(1), nullptr);
        }
        const auto Symbol = core::NamePool::instance().intern("instruction.jump.target");
        const MachineJmpiInstruction Symbolic{ImmediateOperand(Symbol)};
        EXPECT_EQ(Symbolic.getTarget().getSymbol(), Symbol);
        EXPECT_EQ(Register.getOperand(0), &Register.getTargetOperand());
        EXPECT_EQ(Memory.getOperand(0), &Memory.getTargetOperand());
    }

    // Instructions and opcode traits expose value effects and unconditional control flow without legacy comparison or condition state APIs.
    TEST(VMMachineInstructionTest, OmitsFlagStateInterfaces)
    {
        static_assert(!HasLegacyFlagState<MachineInstruction>);
        static_assert(!HasLegacyFlagState<MachineAdd64rrrInstruction>);
        static_assert(!HasLegacyFlagState<MachineJmpInstruction>);
        static_assert(!HasLegacyFlagState<MachineJe64mibInstruction>);
        static_assert(!HasLegacyConditionMetadata<OpcodeTraits<Opcode::Add64rrr>>);
        static_assert(!HasLegacyConditionMetadata<OpcodeTraits<Opcode::Jmpi>>);
        SUCCEED();
    }

    // The comparison manifest reserves eighty consecutive encodings across all relations, widths and memory-source forms.
    TEST(VMMachineInstructionTest, RegistersEveryExplicitComparisonBranchEncoding)
    {
        constexpr Opcode Operations[] = {
#define COMPARE_BRANCH(Kind, Encoding, Spelling, Form, Width, Operator, Signed) Opcode::Kind,
#include "ink/backend/vm/instruction/compare_branch.def"
#undef COMPARE_BRANCH
        };
        static_assert(std::size(Operations) == 80);
        for (std::size_t Index = 0; Index < std::size(Operations); ++Index)
        {
            EXPECT_EQ(static_cast<std::uint16_t>(Operations[Index]), 0x0042U + Index);
        }
        static_assert(OpcodeTraits<Opcode::Je8mmb>::BitWidth == 8);
        static_assert(OpcodeTraits<Opcode::Je16mib>::BitWidth == 16);
        static_assert(OpcodeTraits<Opcode::Jlt32mmb>::BitWidth == 32);
        static_assert(OpcodeTraits<Opcode::Jgeu64mib>::BitWidth == 64);
    }

    // Comparison branches own their two source operands and final block target, expose no destination, and reject mismatched target positions or source kinds.
    TEST(VMMachineInstructionTest, ModelsExplicitComparisonSourcesAndFinalBlockTargets)
    {
        static const MachineBasicBlock Target;
        static const MachineBasicBlock OtherTarget;
        AddressOperand Left(RegisterId::BP, -16);
        ImmediateOperand Right(std::numeric_limits<std::uint64_t>::max());
        BasicBlockOperand Destination(&Target);
        const MachineJe64mibInstruction Value(Left, Right, Destination);
        Left = AddressOperand(RegisterId::SP, 8);
        Right = ImmediateOperand(0);
        Destination = BasicBlockOperand(&OtherTarget);
        EXPECT_EQ(Value.getLHS().getRegister(), RegisterId::BP);
        EXPECT_EQ(Value.getLHS().getByteOffset(), -16);
        EXPECT_EQ(Value.getRHS().bits(), std::numeric_limits<std::uint64_t>::max());
        EXPECT_EQ(Value.getTarget(), &Target);
        EXPECT_EQ(Value.getOperand(0), &Value.getLHS());
        EXPECT_EQ(Value.getOperand(1), &Value.getRHS());
        EXPECT_EQ(Value.getOperand(2), &Value.getTargetOperand());
        EXPECT_EQ(Value.getTargetOperand().getBlock(), &Target);
        EXPECT_TRUE(Value.isConditionalBranch());
        EXPECT_TRUE(Value.isBranch());
        EXPECT_TRUE(Value.isTerminator());
        EXPECT_FALSE(Value.hasDestination());
        constexpr MachineJltu8mmbInstruction Memory(AddressOperand(RegisterId::BP, 1), AddressOperand(RegisterId::SP, 3), BasicBlockOperand(&Target));
        static_assert(Memory.getLHS().getByteOffset() == 1 && Memory.getRHS().getByteOffset() == 3);
        static_assert(Memory.getTarget() == &Target);
        static_assert(std::is_same_v<decltype(Value.getLHS()), const AddressOperand&>);
        static_assert(std::is_same_v<decltype(Value.getRHS()), const ImmediateOperand&>);
        static_assert(std::is_same_v<decltype(Memory.getRHS()), const AddressOperand&>);
        static_assert(std::is_same_v<decltype(Value.getTarget()), const MachineBasicBlock*>);
        static_assert(std::is_same_v<decltype(Value.getTargetOperand()), const BasicBlockOperand&>);
        static_assert(!HasInstructionDestination<MachineJe64mibInstruction> && !HasInstructionSource<MachineJe64mibInstruction>);
        static_assert(HasInstructionLHS<MachineJe64mibInstruction> && HasInstructionRHS<MachineJltu8mmbInstruction>);
        static_assert(!std::is_constructible_v<MachineJe64mibInstruction, const MachineBasicBlock*>);
        static_assert(!std::is_constructible_v<MachineJe64mibInstruction, AddressOperand, ImmediateOperand>);
        static_assert(!std::is_constructible_v<MachineJe64mibInstruction, AddressOperand, ImmediateOperand, const MachineBasicBlock*>);
        static_assert(!std::is_constructible_v<MachineJe64mibInstruction, BasicBlockOperand, AddressOperand, ImmediateOperand>);
        static_assert(!std::is_constructible_v<MachineJe64mibInstruction, AddressOperand, AddressOperand, BasicBlockOperand>);
        static_assert(!std::is_constructible_v<MachineJltu8mmbInstruction, AddressOperand, ImmediateOperand, BasicBlockOperand>);
        static_assert(!std::is_constructible_v<MachineJe64mibInstruction, RegisterOperand, ImmediateOperand, BasicBlockOperand>);
    }

    // RET is a zero-operand terminator with no destination or target.
    TEST(VMMachineInstructionTest, SupportsZeroOperandReturn)
    {
        constexpr MachineRetInstruction Return;
        static_assert(Return.getOpcode() == Opcode::Ret);
        static_assert(Return.getName() == "ret");
        static_assert(Return.operandCount() == 0 && Return.bitWidth() == 0);
        static_assert(Return.getOperand(0) == nullptr);
        static_assert(Return.isReturn() && Return.isTerminator() && !Return.isBranch());
        static_assert(!HasInstructionDestination<MachineRetInstruction> && !HasInstructionTarget<MachineRetInstruction>);
        static_assert(!HasInstructionSource<MachineRetInstruction> && !HasInstructionLHS<MachineRetInstruction> && !HasInstructionRHS<MachineRetInstruction>);
        static_assert(!std::is_constructible_v<MachineRetInstruction, ImmediateOperand>);
        const MachineInstruction& Base = Return;
        EXPECT_EQ(Base.getOperand(0), nullptr);
        EXPECT_EQ(Base.getOperand(std::numeric_limits<std::size_t>::max()), nullptr);
    }

    // Immediate, VM-direct, and memory calls share full-width targets without terminating their block.
    TEST(VMMachineInstructionTest, ModelsImmediateAndIndirectCalls)
    {
        const core::Name Target = core::NamePool::instance().intern("instruction.call.target");
        const MachineCalliInstruction Direct{ImmediateOperand(Target)};
        const MachineCallvmiInstruction DirectVM{ImmediateOperand(Target)};
        constexpr MachineCallmInstruction Indirect{AddressOperand(RegisterId::SP, 8)};
        const MachineInstruction* Calls[] = {&Direct, &DirectVM, &Indirect};
        for (const auto* Value : Calls)
        {
            EXPECT_TRUE(Value->isCall());
            EXPECT_FALSE(Value->isTerminator());
            EXPECT_FALSE(Value->isBranch());
            EXPECT_FALSE(Value->isReturn());
            EXPECT_FALSE(Value->hasDestination());
            EXPECT_EQ(Value->category(), InstructionCategory::Call);
            EXPECT_EQ(Value->operandCount(), 1U);
            EXPECT_EQ(Value->bitWidth(), 64U);
            ASSERT_NE(Value->getOperand(0), nullptr);
            EXPECT_EQ(Value->getOperand(1), nullptr);
            EXPECT_EQ(Value->getName(), Value == &DirectVM ? "callvm" : "call");
        }
        EXPECT_EQ(Direct.getTarget().getSymbol(), Target);
        EXPECT_EQ(DirectVM.getTarget().getSymbol(), Target);
        EXPECT_EQ(Direct.getTargetOperand().getKind(), OperandKind::Immediate);
        EXPECT_EQ(DirectVM.getTargetOperand().getKind(), OperandKind::Immediate);
        EXPECT_EQ(Indirect.getTargetOperand().getKind(), OperandKind::Address);
        EXPECT_EQ(Direct.getOperand(0), &Direct.getTarget());
        EXPECT_EQ(DirectVM.getOperand(0), &DirectVM.getTarget());
        EXPECT_EQ(Indirect.getOperand(0), &Indirect.getTarget());
        static_assert(Indirect.getTarget().getRegister() == RegisterId::SP);
        static_assert(Indirect.getTarget().getByteOffset() == 8);
        static_assert(OpcodeTraits<Opcode::Calli>::Form == InstructionForm::I);
        static_assert(OpcodeTraits<Opcode::Calli>::ImmediateCount == 1);
        static_assert(OpcodeTraits<Opcode::Callvmi>::Form == InstructionForm::I);
        static_assert(OpcodeTraits<Opcode::Callvmi>::ImmediateCount == 1);
        static_assert(OpcodeTraits<Opcode::Callm>::Form == InstructionForm::M);
        static_assert(OpcodeTraits<Opcode::Callm>::AddressCount == 1);
        static_assert(std::is_same_v<decltype(Direct.getTarget()), const ImmediateOperand&>);
        static_assert(std::is_same_v<decltype(Indirect.getTarget()), const AddressOperand&>);
        static_assert(!HasInstructionDestination<MachineCalliInstruction> && !HasInstructionSource<MachineCallmInstruction>);
        static_assert(!std::is_constructible_v<MachineCalliInstruction, AddressOperand>);
        static_assert(!std::is_constructible_v<MachineCallmInstruction, ImmediateOperand>);
        static_assert(!std::is_constructible_v<MachineCalliInstruction, core::Name>);
        static_assert(!std::is_constructible_v<MachineCallmInstruction, RegisterOperand>);
        static_assert(!std::is_constructible_v<MachineCalliInstruction, std::uint64_t>);
        static_assert(!std::is_constructible_v<MachineCalliInstruction, ImmediateOperand, ImmediateOperand>);
        constexpr MachineCalliInstruction Numeric{ImmediateOperand(std::numeric_limits<std::uint64_t>::max())};
        static_assert(Numeric.getTarget().bits() == std::numeric_limits<std::uint64_t>::max());
        static_assert(Numeric.isCall());
        constexpr MachineCallrInstruction RegisterIndirect{RegisterOperand(RegisterId::R0, 16)};
        static_assert(OpcodeTraits<Opcode::Callr>::Form == InstructionForm::R);
        static_assert(OpcodeTraits<Opcode::Callr>::RegisterCount == 1);
        static_assert(std::is_same_v<decltype(RegisterIndirect.getTarget()), const RegisterOperand&>);
        static_assert(RegisterIndirect.getTarget().getRegister() == RegisterId::R0 && RegisterIndirect.getTarget().getByteOffset() == 16);
        static_assert(!HasInstructionDestination<MachineCallrInstruction> && !HasInstructionSource<MachineCallrInstruction>);
        static_assert(!std::is_constructible_v<MachineCallrInstruction, AddressOperand>);
        static_assert(!std::is_constructible_v<MachineCallrInstruction, ImmediateOperand>);
        EXPECT_TRUE(RegisterIndirect.isCall());
        EXPECT_FALSE(RegisterIndirect.isTerminator());
        EXPECT_EQ(RegisterIndirect.getName(), "call");
        EXPECT_EQ(RegisterIndirect.getOperand(0), &RegisterIndirect.getTarget());
    }

    // P0 comparison, register-branch, and floating-point comparison manifests occupy contiguous non-overlapping opcode ranges.
    TEST(VMMachineInstructionTest, RegistersContiguousP0ManifestEncodings)
    {
        constexpr Opcode IntegerCompares[] = {
#define INTEGER_COMPARE(Kind, Encoding, Spelling, Form, Operator, Signed) Opcode::Kind,
#include "ink/backend/vm/instruction/integer_compare.def"
#undef INTEGER_COMPARE
        };
        static_assert(std::size(IntegerCompares) == 20);
        for (std::size_t Index = 0; Index < std::size(IntegerCompares); ++Index)
        {
            EXPECT_EQ(static_cast<std::uint16_t>(IntegerCompares[Index]), 0x00B8U + Index);
        }
        constexpr Opcode RegisterBranches[] = {
#define REGISTER_COMPARE_BRANCH(Kind, Encoding, Spelling, Form, Operator, Signed) Opcode::Kind,
#include "ink/backend/vm/instruction/register_compare_branch.def"
#undef REGISTER_COMPARE_BRANCH
        };
        static_assert(std::size(RegisterBranches) == 20);
        for (std::size_t Index = 0; Index < std::size(RegisterBranches); ++Index)
        {
            EXPECT_EQ(static_cast<std::uint16_t>(RegisterBranches[Index]), 0x00CCU + Index);
        }
        constexpr Opcode FloatCompares[] = {
#define FLOAT_COMPARE(Kind, Encoding, Spelling, Expression) Opcode::Kind,
#include "ink/backend/vm/instruction/float_compare.def"
#undef FLOAT_COMPARE
        };
        static_assert(std::size(FloatCompares) == 14);
        for (std::size_t Index = 0; Index < std::size(FloatCompares); ++Index)
        {
            EXPECT_EQ(static_cast<std::uint16_t>(FloatCompares[Index]), 0x00E1U + Index);
        }
        static_assert(OpcodeTraits<Opcode::Add64rrr>::Category == InstructionCategory::Arithmetic && OpcodeTraits<Opcode::Add64rrr>::Domain == InstructionDomain::Integer && !OpcodeTraits<Opcode::Add64rrr>::IsFloatingPoint);
        static_assert(OpcodeTraits<Opcode::Fadd64rrr>::Category == InstructionCategory::Arithmetic && OpcodeTraits<Opcode::Fadd64rrr>::Domain == InstructionDomain::FloatingPoint && OpcodeTraits<Opcode::Fadd64rrr>::IsFloatingPoint);
        static_assert(OpcodeTraits<Opcode::Fneg64rr>::Category == InstructionCategory::Arithmetic && OpcodeTraits<Opcode::Fneg64rr>::Domain == InstructionDomain::FloatingPoint && OpcodeTraits<Opcode::Fneg64rr>::IsFloatingPoint);
        static_assert(OpcodeTraits<Opcode::Mov64rr>::Category == InstructionCategory::Move && OpcodeTraits<Opcode::Mov64rr>::Domain == InstructionDomain::Integer && OpcodeTraits<Opcode::Mov64rr>::HasDestination);
        static_assert(OpcodeTraits<Opcode::Load8srm>::Category == InstructionCategory::Load && OpcodeTraits<Opcode::Load8srm>::Domain == InstructionDomain::Integer && OpcodeTraits<Opcode::Load8srm>::BitWidth == 8);
        static_assert(OpcodeTraits<Opcode::Store32mr>::Category == InstructionCategory::Store && OpcodeTraits<Opcode::Store32mr>::Domain == InstructionDomain::Integer && OpcodeTraits<Opcode::Store32mr>::BitWidth == 32);
        static_assert(OpcodeTraits<Opcode::Sdiv64rrr>::Category == InstructionCategory::Arithmetic && OpcodeTraits<Opcode::Sdiv64rrr>::Domain == InstructionDomain::Integer && OpcodeTraits<Opcode::Sdiv64rrr>::HasDestination);
        static_assert(OpcodeTraits<Opcode::Ashr64rri>::Category == InstructionCategory::Bitwise && OpcodeTraits<Opcode::Ashr64rri>::Domain == InstructionDomain::Integer && OpcodeTraits<Opcode::Ashr64rri>::HasDestination);
        static_assert(OpcodeTraits<Opcode::Cmplt64rrr>::Category == InstructionCategory::Compare && OpcodeTraits<Opcode::Cmplt64rrr>::Domain == InstructionDomain::Integer && OpcodeTraits<Opcode::Cmplt64rrr>::HasDestination);
        static_assert(OpcodeTraits<Opcode::Je64rrb>::Category == InstructionCategory::ConditionalBranch && OpcodeTraits<Opcode::Je64rrb>::Domain == InstructionDomain::Integer && !OpcodeTraits<Opcode::Je64rrb>::HasDestination);
        static_assert(OpcodeTraits<Opcode::Fcmp64oeqrrr>::Category == InstructionCategory::Compare && OpcodeTraits<Opcode::Fcmp64oeqrrr>::Domain == InstructionDomain::FloatingPoint && OpcodeTraits<Opcode::Fcmp64oeqrrr>::HasDestination);
        static_assert(OpcodeTraits<Opcode::F64tosi64rr>::Category == InstructionCategory::Convert && OpcodeTraits<Opcode::F64tosi64rr>::Domain == InstructionDomain::FloatingPoint && OpcodeTraits<Opcode::F64tosi64rr>::HasDestination);
        static_assert(OpcodeTraits<Opcode::Jmp>::Category == InstructionCategory::Branch && OpcodeTraits<Opcode::Jmp>::Domain == InstructionDomain::None);
        static_assert(OpcodeTraits<Opcode::Callr>::Category == InstructionCategory::Call && OpcodeTraits<Opcode::Callr>::Domain == InstructionDomain::None);
        static_assert(OpcodeTraits<Opcode::Ret>::Category == InstructionCategory::Return && OpcodeTraits<Opcode::Ret>::Domain == InstructionDomain::None);
    }

} // namespace ink::vm::test
