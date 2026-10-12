#include "ink/core/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/conditional_branch_instruction.h"
#include "ink/ir/instruction/getelementptr_instruction.h"
#include "ink/ir/instruction/icmp_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    class ObservedInstruction final : public Instruction
    {
      public:
        ObservedInstruction(const Type &ResultType, bool &Destroyed) noexcept
            : Instruction(ValueKind::Instruction, &ResultType),
              Destroyed(Destroyed)
        {
        }

        ~ObservedInstruction() noexcept override
        {
          Destroyed = true;
        }

      private:
        bool &Destroyed;
    };
  } // namespace

  static_assert(std::is_base_of_v<Instruction, GetElementPtrInstruction>);
  static_assert(std::is_base_of_v<Instruction, CallInstruction>);
  static_assert(std::is_base_of_v<Instruction, ICmpInstruction>);
  static_assert(std::is_base_of_v<Instruction, BranchInstruction>);
  static_assert(std::is_base_of_v<Instruction, ConditionalBranchInstruction>);
  static_assert(std::is_base_of_v<Instruction, ReturnInstruction>);
  static_assert(std::is_final_v<GetElementPtrInstruction>);
  static_assert(std::is_final_v<CallInstruction>);
  static_assert(std::is_final_v<ICmpInstruction>);
  static_assert(std::is_final_v<BranchInstruction>);
  static_assert(std::is_final_v<ConditionalBranchInstruction>);
  static_assert(std::is_final_v<ReturnInstruction>);
  static_assert(!std::is_base_of_v<BranchInstruction, ConditionalBranchInstruction>);
  static_assert(!std::is_constructible_v<BranchInstruction, const VoidType &, const Value &, BasicBlock &, BasicBlock &>);
  static_assert(!std::is_constructible_v<ConditionalBranchInstruction, const VoidType &, BasicBlock &>);

  // A function body connects an indexed array element, call result, comparison and both branch forms to value-returning blocks.
  TEST(IRConcreteInstructionTest, ConnectsComputedValuesAndControlFlowAcrossBlocks)
  {
    TypeContext Types(Target);
    auto &Constants = Types.constantPool();
    const auto *Array = Types.getArrayType(Types.getInt32Type(), 8);
    ASSERT_NE(Array, nullptr);
    const auto *ArrayPointer = Types.getPointerType(Array);
    const auto *ElementPointer = Types.getPointerType(Types.getInt32Type());
    ASSERT_NE(ArrayPointer, nullptr);
    ASSERT_NE(ElementPointer, nullptr);
    const auto *CalleeSignature = Types.getFunctionType(Types.getInt32Type(), {ElementPointer});
    const auto *CallerSignature = Types.getFunctionType(Types.getInt32Type(), {ArrayPointer, Types.getInt32Type()});
    const auto *Zero = Constants.getSignedIntegerConstant(Types.getInt32Type(), 0);
    ASSERT_NE(CalleeSignature, nullptr);
    ASSERT_NE(CallerSignature, nullptr);
    ASSERT_NE(Zero, nullptr);
    Function Callee(*CalleeSignature);
    Function Caller(*CallerSignature);
    auto *Entry = Caller.createBlock();
    auto *Accepted = Caller.createBlock();
    auto *Rejected = Caller.createBlock();
    auto *Exit = Caller.createBlock();
    ASSERT_NE(Entry, nullptr);
    ASSERT_NE(Accepted, nullptr);
    ASSERT_NE(Rejected, nullptr);
    ASSERT_NE(Exit, nullptr);
    ASSERT_NE(Caller.parameter(0), nullptr);
    ASSERT_NE(Caller.parameter(1), nullptr);

    auto AddressOwner = std::make_unique<GetElementPtrInstruction>(*ElementPointer, *Array, *Caller.parameter(0), std::vector<const Value *>{Zero, Caller.parameter(1)}, true);
    const auto *Address = AddressOwner.get();
    EXPECT_EQ(Entry->appendInstruction(std::move(AddressOwner)), Address);
    auto CallOwner = std::make_unique<CallInstruction>(Callee, std::vector<Argument>{Argument(*Address)});
    const auto *Call = CallOwner.get();
    EXPECT_EQ(Entry->appendInstruction(std::move(CallOwner)), Call);
    auto CompareOwner = std::make_unique<ICmpInstruction>(*Types.getBoolType(), ICmpPredicate::SignedGreaterThan, *Call, *Zero);
    const auto *Compare = CompareOwner.get();
    EXPECT_EQ(Entry->appendInstruction(std::move(CompareOwner)), Compare);
    auto BranchOwner = std::make_unique<ConditionalBranchInstruction>(*Types.getVoidType(), *Compare, *Accepted, *Rejected);
    const auto *Branch = BranchOwner.get();
    EXPECT_EQ(Entry->appendInstruction(std::move(BranchOwner)), Branch);
    auto ReturnOwner = std::make_unique<ReturnInstruction>(*Types.getVoidType(), Call);
    const auto *Return = ReturnOwner.get();
    EXPECT_EQ(Accepted->appendInstruction(std::move(ReturnOwner)), Return);
    auto JumpOwner = std::make_unique<BranchInstruction>(*Types.getVoidType(), *Exit);
    const auto *Jump = JumpOwner.get();
    EXPECT_EQ(Rejected->appendInstruction(std::move(JumpOwner)), Jump);
    EXPECT_NE(Exit->appendInstruction(std::make_unique<ReturnInstruction>(*Types.getVoidType(), Zero)), nullptr);

    EXPECT_EQ(Entry->instructionCount(), 4U);
    EXPECT_EQ(Address->type(), ElementPointer);
    EXPECT_EQ(Address->sourceElementType(), Array);
    EXPECT_EQ(Address->pointer(), Caller.parameter(0));
    EXPECT_TRUE(Address->isInBounds());
    ASSERT_EQ(Address->indices().size(), 2U);
    EXPECT_EQ(Address->indexCount(), 2U);
    EXPECT_EQ(Address->index(0), Zero);
    EXPECT_EQ(Address->index(1), Caller.parameter(1));
    EXPECT_EQ(Address->index(2), nullptr);
    EXPECT_EQ(Call->type(), Types.getInt32Type());
    EXPECT_EQ(Call->functionType(), CalleeSignature);
    EXPECT_EQ(Call->callee(), &Callee);
    EXPECT_EQ(Call->directCallee(), &Callee);
    ASSERT_NE(Call->argument(0), nullptr);
    EXPECT_EQ(Call->argument(0)->value(), Address);
    EXPECT_EQ(Compare->type(), Types.getBoolType());
    EXPECT_EQ(Compare->left(), Call);
    EXPECT_EQ(Compare->right(), Zero);
    EXPECT_EQ(Compare->predicate(), ICmpPredicate::SignedGreaterThan);
    EXPECT_EQ(Branch->type(), Types.getVoidType());
    EXPECT_EQ(Branch->condition(), Compare);
    EXPECT_EQ(Branch->trueTarget(), Accepted);
    EXPECT_EQ(Branch->falseTarget(), Rejected);
    EXPECT_EQ(Branch->successorCount(), 2U);
    EXPECT_EQ(Branch->successor(0), Accepted);
    EXPECT_EQ(Branch->successor(1), Rejected);
    EXPECT_EQ(Branch->successor(2), nullptr);
    EXPECT_EQ(Branch->successor(std::numeric_limits<std::size_t>::max()), nullptr);
    EXPECT_EQ(Return->type(), Types.getVoidType());
    EXPECT_TRUE(Return->hasReturnedValue());
    EXPECT_EQ(Return->returnedValue(), Call);
    EXPECT_EQ(Jump->type(), Types.getVoidType());
    EXPECT_EQ(Jump->target(), Exit);
    EXPECT_EQ(Jump->successorCount(), 1U);
    EXPECT_EQ(Jump->successor(0), Exit);
    EXPECT_EQ(Jump->successor(1), nullptr);
    EXPECT_EQ(Jump->successor(std::numeric_limits<std::size_t>::max()), nullptr);
    EXPECT_FALSE(Entry->instruction(0)->isTerminator());
    EXPECT_FALSE(Entry->instruction(1)->isTerminator());
    EXPECT_FALSE(Entry->instruction(2)->isTerminator());
    EXPECT_TRUE(Entry->instruction(3)->isTerminator());
    EXPECT_TRUE(Accepted->instruction(0)->isTerminator());
    EXPECT_TRUE(Rejected->instruction(0)->isTerminator());
    EXPECT_TRUE(Exit->instruction(0)->isTerminator());
  }

  // An indirect call borrows its callable parameter while owning an independent ordered copy of named actual arguments.
  TEST(IRConcreteInstructionTest, IndirectCallCopiesArgumentsAndKeepsSignature)
  {
    TypeContext Types(Target);
    const auto *Signature = Types.getFunctionType(Types.getInt32Type(), {Types.getInt32Type(), Types.getBoolType()});
    ASSERT_NE(Signature, nullptr);
    const auto *CallablePointer = Types.getPointerType(Signature);
    ASSERT_NE(CallablePointer, nullptr);
    const auto *OwnerSignature = Types.getFunctionType(Types.getVoidType(), {CallablePointer, Types.getInt32Type()});
    ASSERT_NE(OwnerSignature, nullptr);
    Function Owner(*OwnerSignature);
    const auto *True = Types.constantPool().getBoolConstant(true);
    const core::Name ActualName = core::NamePool::instance().intern("IRConcreteInstructionTest.Actual");
    ASSERT_TRUE(ActualName.valid());
    ASSERT_NE(Owner.parameter(0), nullptr);
    ASSERT_NE(Owner.parameter(1), nullptr);
    std::vector<Argument> Arguments = {Argument(*Owner.parameter(1), ActualName), Argument(*True)};
    const CallInstruction Call(*Signature, *Owner.parameter(0), Arguments);
    Arguments.front().setName(core::NAME_None);
    Arguments.clear();
    EXPECT_EQ(Call.callee(), Owner.parameter(0));
    EXPECT_EQ(Call.directCallee(), nullptr);
    EXPECT_EQ(Call.functionType(), Signature);
    EXPECT_EQ(Call.type(), Types.getInt32Type());
    EXPECT_EQ(Call.argumentCount(), 2U);
    ASSERT_EQ(Call.arguments().size(), 2U);
    ASSERT_NE(Call.argument(0), nullptr);
    EXPECT_EQ(Call.argument(0)->value(), Owner.parameter(1));
    EXPECT_EQ(Call.argument(0)->name(), ActualName);
    ASSERT_NE(Call.argument(1), nullptr);
    EXPECT_EQ(Call.argument(1)->value(), True);
    EXPECT_FALSE(Call.argument(1)->hasName());
    EXPECT_EQ(Call.argument(2), nullptr);
    EXPECT_EQ(Call.argument(std::numeric_limits<std::size_t>::max()), nullptr);
  }

  // Empty operand lists and void returns retain concrete types and reject every out-of-range operand lookup.
  TEST(IRConcreteInstructionTest, EmptyOperandListsAndVoidReturnHaveDefinedBoundaries)
  {
    TypeContext Types(Target);
    const auto *Pointer = Types.getPointerType(Types.getInt32Type());
    const auto *Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(Pointer, nullptr);
    ASSERT_NE(Signature, nullptr);
    Function Callee(*Signature);
    const GetElementPtrInstruction Address(*Pointer, *Types.getInt32Type(), *Types.constantPool().getNullPointerConstant(), {});
    const CallInstruction Call(Callee);
    const ReturnInstruction Return(*Types.getVoidType());
    EXPECT_EQ(Address.type(), Pointer);
    EXPECT_FALSE(Address.isInBounds());
    EXPECT_TRUE(Address.indices().empty());
    EXPECT_EQ(Address.indexCount(), 0U);
    EXPECT_EQ(Address.index(0), nullptr);
    EXPECT_EQ(Address.index(std::numeric_limits<std::size_t>::max()), nullptr);
    EXPECT_EQ(Call.type(), Types.getVoidType());
    EXPECT_EQ(Call.directCallee(), &Callee);
    EXPECT_TRUE(Call.arguments().empty());
    EXPECT_EQ(Call.argumentCount(), 0U);
    EXPECT_EQ(Call.argument(0), nullptr);
    EXPECT_FALSE(Call.isTerminator());
    EXPECT_EQ(Return.type(), Types.getVoidType());
    EXPECT_FALSE(Return.hasReturnedValue());
    EXPECT_EQ(Return.returnedValue(), nullptr);
    EXPECT_TRUE(Return.isTerminator());
  }

  // Every signed, unsigned and equality predicate preserves its identity without swapping its integer operands.
  TEST(IRConcreteInstructionTest, PreservesAllIntegerComparisonPredicates)
  {
    TypeContext Types(Target);
    const auto *Left = Types.constantPool().getSignedIntegerConstant(Types.getInt32Type(), -1);
    const auto *Right = Types.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 1);
    ASSERT_NE(Left, nullptr);
    ASSERT_NE(Right, nullptr);
    constexpr ICmpPredicate Predicates[] = {
        ICmpPredicate::Equal,
        ICmpPredicate::NotEqual,
        ICmpPredicate::UnsignedGreaterThan,
        ICmpPredicate::UnsignedGreaterEqual,
        ICmpPredicate::UnsignedLessThan,
        ICmpPredicate::UnsignedLessEqual,
        ICmpPredicate::SignedGreaterThan,
        ICmpPredicate::SignedGreaterEqual,
        ICmpPredicate::SignedLessThan,
        ICmpPredicate::SignedLessEqual,
    };
    for (const auto Predicate : Predicates)
    {
      const ICmpInstruction Compare(*Types.getBoolType(), Predicate, *Left, *Right);
      EXPECT_EQ(Compare.predicate(), Predicate);
      EXPECT_EQ(Compare.left(), Left);
      EXPECT_EQ(Compare.right(), Right);
      EXPECT_EQ(Compare.type(), Types.getBoolType());
      EXPECT_FALSE(Compare.isTerminator());
    }
  }

  // Concrete instruction kinds classify through Instruction while remaining distinct from each other, types, constants and blocks.
  TEST(IRConcreteInstructionTest, ClassifiesConcreteInstructionsWithoutFalsePositives)
  {
    TypeContext Types(Target);
    const auto *Pointer = Types.getPointerType(Types.getInt32Type());
    const auto *Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(Pointer, nullptr);
    ASSERT_NE(Signature, nullptr);
    Function Callee(*Signature);
    BasicBlock Block;
    const auto *Literal = Types.constantPool().getBoolConstant(true);
    const GetElementPtrInstruction Address(*Pointer, *Types.getInt32Type(), *Types.constantPool().getNullPointerConstant(), {});
    const CallInstruction Call(Callee);
    const ICmpInstruction Compare(*Types.getBoolType(), ICmpPredicate::Equal, *Literal, *Literal);
    const BranchInstruction Branch(*Types.getVoidType(), Block);
    const ConditionalBranchInstruction ConditionalBranch(*Types.getVoidType(), Compare, Block, Block);
    const ReturnInstruction Return(*Types.getVoidType());
    EXPECT_EQ(ConditionalBranch.condition(), &Compare);
    EXPECT_EQ(ConditionalBranch.trueTarget(), &Block);
    EXPECT_EQ(ConditionalBranch.falseTarget(), &Block);
    EXPECT_EQ(ConditionalBranch.successorCount(), 2U);
    EXPECT_EQ(ConditionalBranch.successor(0), &Block);
    EXPECT_EQ(ConditionalBranch.successor(1), &Block);
    EXPECT_EQ(ConditionalBranch.successor(2), nullptr);
    EXPECT_TRUE(ConditionalBranch.isTerminator());
    EXPECT_TRUE(Branch.isTerminator());
    struct Classification
    {
        const Instruction *Operation;
        ValueKind Kind;
        std::string_view Name;
        bool (*ClassOf)(const Value *) noexcept;
    };
    const Classification Cases[] = {
        {&Address, ValueKind::GetElementPtrInstruction, "getelementptr", &GetElementPtrInstruction::classof},
        {&Call, ValueKind::CallInstruction, "call", &CallInstruction::classof},
        {&Compare, ValueKind::ICmpInstruction, "icmp", &ICmpInstruction::classof},
        {&Branch, ValueKind::BranchInstruction, "br", &BranchInstruction::classof},
        {&ConditionalBranch, ValueKind::ConditionalBranchInstruction, "cond_br", &ConditionalBranchInstruction::classof},
        {&Return, ValueKind::ReturnInstruction, "ret", &ReturnInstruction::classof},
    };
    for (const auto &Case : Cases)
    {
      EXPECT_EQ(Case.Operation->kind(), Case.Kind);
      EXPECT_EQ(valueKindName(Case.Kind), Case.Name);
      EXPECT_TRUE(Instruction::classof(Case.Operation));
      EXPECT_FALSE(Type::classof(Case.Operation));
      EXPECT_FALSE(Constant::classof(Case.Operation));
      EXPECT_FALSE(Case.ClassOf(nullptr));
      EXPECT_FALSE(Case.ClassOf(Types.getBoolType()));
      EXPECT_FALSE(Case.ClassOf(Literal));
      EXPECT_FALSE(Case.ClassOf(&Block));
      EXPECT_FALSE(Case.ClassOf(&Callee));
      for (const auto &Other : Cases)
      {
        EXPECT_EQ(Case.ClassOf(Other.Operation), Case.Operation == Other.Operation);
      }
    }
    EXPECT_FALSE(Instruction::classof(nullptr));
    EXPECT_FALSE(Instruction::classof(&Block));
    EXPECT_FALSE(Instruction::classof(&Callee));
    EXPECT_FALSE(Instruction::classof(Literal));
    EXPECT_FALSE(Instruction::classof(Types.getBoolType()));
  }

  // Destroying instructions releases their operand containers without deleting the externally owned values or branch targets.
  TEST(IRConcreteInstructionTest, InstructionDestructionDoesNotOwnOperandsOrTargets)
  {
    TypeContext Types(Target);
    const auto *Pointer = Types.getPointerType(Types.getInt32Type());
    const auto *Signature = Types.getFunctionType(Types.getInt32Type(), {Types.getInt32Type()});
    ASSERT_NE(Pointer, nullptr);
    ASSERT_NE(Signature, nullptr);
    const auto *CallablePointer = Types.getPointerType(Signature);
    ASSERT_NE(CallablePointer, nullptr);
    bool PointerDestroyed = false;
    bool IndexDestroyed = false;
    bool CalleeDestroyed = false;
    bool ConditionDestroyed = false;
    auto PointerOperand = std::make_unique<ObservedInstruction>(*Pointer, PointerDestroyed);
    auto IndexOperand = std::make_unique<ObservedInstruction>(*Types.getInt32Type(), IndexDestroyed);
    auto CalleeOperand = std::make_unique<ObservedInstruction>(*CallablePointer, CalleeDestroyed);
    auto ConditionOperand = std::make_unique<ObservedInstruction>(*Types.getBoolType(), ConditionDestroyed);
    BasicBlock TrueTarget;
    BasicBlock FalseTarget;
    {
      std::vector<const Value *> Indices = {IndexOperand.get()};
      const GetElementPtrInstruction Address(*Pointer, *Types.getInt32Type(), *PointerOperand, Indices);
      Indices.clear();
      const CallInstruction Call(*Signature, *CalleeOperand, {Argument(*IndexOperand)});
      const ICmpInstruction Compare(*Types.getBoolType(), ICmpPredicate::Equal, *IndexOperand, *IndexOperand);
      const BranchInstruction Jump(*Types.getVoidType(), TrueTarget);
      const ConditionalBranchInstruction Branch(*Types.getVoidType(), *ConditionOperand, TrueTarget, FalseTarget);
      const ReturnInstruction Return(*Types.getVoidType(), IndexOperand.get());
      EXPECT_EQ(Address.indexCount(), 1U);
      EXPECT_EQ(Address.index(0), IndexOperand.get());
      EXPECT_EQ(Call.callee(), CalleeOperand.get());
      EXPECT_EQ(Call.directCallee(), nullptr);
      EXPECT_EQ(Compare.left(), IndexOperand.get());
      EXPECT_EQ(Jump.target(), &TrueTarget);
      EXPECT_EQ(Branch.condition(), ConditionOperand.get());
      EXPECT_EQ(Return.returnedValue(), IndexOperand.get());
    }
    EXPECT_FALSE(PointerDestroyed);
    EXPECT_FALSE(IndexDestroyed);
    EXPECT_FALSE(CalleeDestroyed);
    EXPECT_FALSE(ConditionDestroyed);
    EXPECT_EQ(TrueTarget.instructionCount(), 0U);
    EXPECT_EQ(FalseTarget.instructionCount(), 0U);
    PointerOperand.reset();
    IndexOperand.reset();
    CalleeOperand.reset();
    ConditionOperand.reset();
    EXPECT_TRUE(PointerDestroyed);
    EXPECT_TRUE(IndexDestroyed);
    EXPECT_TRUE(CalleeDestroyed);
    EXPECT_TRUE(ConditionDestroyed);
  }
} // namespace ink::ir::test
