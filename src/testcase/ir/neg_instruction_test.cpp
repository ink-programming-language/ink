#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/neg_instruction.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    class UnpooledNumericConstant final : public Constant
    {
      public:
        explicit UnpooledNumericConstant(const Type *ConstantType) noexcept
            : Constant(ValueKind::IntegerConstant, ConstantType)
        {
        }
    };

    class DetachedNumericInstruction final : public Instruction
    {
      public:
        explicit DetachedNumericInstruction(const Type *ResultType) noexcept
            : Instruction(ValueKind::Instruction, ResultType)
        {
        }
    };
  } // namespace

  static_assert(std::is_base_of_v<Instruction, NegInstruction>);
  static_assert(std::is_same_v<decltype(std::declval<IRBuilder &>().createNeg(std::declval<const Value &>())), NegInstruction *>);
  static_assert(std::is_same_v<decltype(std::declval<const NegInstruction &>().operand()), const Value *>);

  // A directly constructed negation borrows its operand and result type, identifies as a nonterminating instruction and formats its operation name.
  TEST(IRNegInstructionTest, RecordsBorrowedOperandAndInstructionIdentity)
  {
    IRContext Context(Target);
    const auto *Integer = Context.typePool().getInt16Type();
    Function Owner(*Context.typePool().getFunctionType(Integer, {Integer}));
    NegInstruction Negation(*Owner.parameter(0));
    const Value *ValueView = &Negation;
    EXPECT_EQ(Negation.operand(), Owner.parameter(0));
    EXPECT_EQ(Negation.type(), Integer);
    EXPECT_EQ(Negation.kind(), ValueKind::NegInstruction);
    EXPECT_EQ(valueKindName(Negation.kind()), "neg");
    EXPECT_EQ(ValueView->toString(), "neg");
    EXPECT_TRUE(Instruction::classof(ValueView));
    EXPECT_TRUE(NegInstruction::classof(ValueView));
    EXPECT_FALSE(Negation.isTerminator());
    EXPECT_FALSE(Constant::classof(ValueView));
    EXPECT_FALSE(Type::classof(ValueView));
    EXPECT_FALSE(NegInstruction::classof(nullptr));
    EXPECT_FALSE(NegInstruction::classof(Owner.parameter(0)));
    EXPECT_FALSE(NegInstruction::classof(&Owner));
  }

  // Signed constants of every width, including each minimum, remain borrowed operands of distinct instructions without being evaluated or folded.
  TEST(IRNegInstructionTest, EmitsForSignedIntegerConstantsWithoutEvaluation)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    auto &Constants = Context.constantPool();
    Function Owner(*Types.getFunctionType(Types.getVoidType()));
    auto *Entry = Owner.createBlock();
    ASSERT_NE(Entry, nullptr);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    for (const std::uint32_t Width : {8U, 16U, 32U, 64U})
    {
      SCOPED_TRACE(Width);
      const auto *Integer = Types.getIntegerType(Width, true);
      ASSERT_NE(Integer, nullptr);
      const auto Maximum = static_cast<std::int64_t>(Integer->maximumValue());
      const std::int64_t Values[] = {0, 1, -1, Maximum, -Maximum, Integer->minimumValue()};
      for (const auto Value : Values)
      {
        SCOPED_TRACE(Value);
        const auto *Operand = Constants.getSignedIntegerConstant(Integer, Value);
        ASSERT_NE(Operand, nullptr);
        const std::uint64_t OriginalBits = Operand->bits();
        const std::size_t PreviousCount = Entry->instructionCount();
        const auto *Result = Builder.createNeg(*Operand);
        ASSERT_NE(Result, nullptr);
        EXPECT_EQ(Result->operand(), Operand);
        EXPECT_EQ(Result->type(), Integer);
        EXPECT_TRUE(NegInstruction::classof(Result));
        EXPECT_FALSE(Constant::classof(Result));
        EXPECT_EQ(Operand->bits(), OriginalBits);
        EXPECT_EQ(Entry->instruction(PreviousCount), Result);
        const auto *Second = Builder.createNeg(*Result);
        ASSERT_NE(Second, nullptr);
        EXPECT_NE(Second, Result);
        EXPECT_EQ(Second->operand(), Result);
        EXPECT_EQ(Second->type(), Integer);
        EXPECT_EQ(Entry->instructionCount(), PreviousCount + 2);
      }
    }
    EXPECT_EQ(Builder.insertBlock(), Entry);
    EXPECT_EQ(Builder.insertFunction(), &Owner);
  }

  // Unsigned zero and nonzero constants in every supported width produce instructions while preserving their original operand values.
  TEST(IRNegInstructionTest, EmitsForUnsignedIntegerConstantsWithoutEvaluation)
  {
    IRContext Context(Target);
    auto &Constants = Context.constantPool();
    Function Owner(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    auto *Entry = Owner.createBlock();
    ASSERT_NE(Entry, nullptr);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    for (const std::uint32_t Width : {8U, 16U, 32U, 64U})
    {
      SCOPED_TRACE(Width);
      const auto *Integer = Context.typePool().getIntegerType(Width, false);
      for (const std::uint64_t Value : {std::uint64_t{0}, std::uint64_t{1}, Integer->maximumValue()})
      {
        const auto *Operand = Constants.getUnsignedIntegerConstant(Integer, Value);
        ASSERT_NE(Operand, nullptr);
        const std::size_t PreviousCount = Entry->instructionCount();
        const auto *Result = Builder.createNeg(*Operand);
        ASSERT_NE(Result, nullptr);
        EXPECT_EQ(Result->operand(), Operand);
        EXPECT_EQ(Result->type(), Integer);
        EXPECT_EQ(Operand->bits(), Value);
        EXPECT_EQ(Entry->instruction(PreviousCount), Result);
        EXPECT_EQ(Entry->instructionCount(), PreviousCount + 1);
      }
    }
    EXPECT_EQ(Builder.insertBlock(), Entry);
  }

  // Floating constants retain their original signed-zero, infinity and quiet or signaling NaN bits as borrowed instruction operands in both widths.
  TEST(IRNegInstructionTest, EmitsForFloatingConstantsWithoutChangingOperandBits)
  {
    IRContext Context(Target);
    auto &Constants = Context.constantPool();
    Function Owner(*Context.typePool().getFunctionType(Context.typePool().getVoidType()));
    auto *Block = Owner.createBlock();
    ASSERT_NE(Block, nullptr);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    struct Case
    {
        std::uint32_t Width;
        std::uint64_t Bits;
    };
    constexpr Case Cases[] = {
        {32, 0x00000000ULL},
        {32, 0x80000000ULL},
        {32, 0x3FC00000ULL},
        {32, 0x7F800000ULL},
        {32, 0x7FC00123ULL},
        {32, 0xFF800123ULL},
        {64, 0x0000000000000000ULL},
        {64, 0x8000000000000000ULL},
        {64, 0x3FF8000000000000ULL},
        {64, 0x7FF0000000000000ULL},
        {64, 0x7FF8000000000123ULL},
        {64, 0xFFF0000000000123ULL},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Width);
      SCOPED_TRACE(Entry.Bits);
      const auto *Float = Context.typePool().getFloatType(Entry.Width);
      const auto *Operand = Constants.getFloatConstantFromBits(Float, Entry.Bits);
      ASSERT_NE(Operand, nullptr);
      const std::size_t PreviousCount = Block->instructionCount();
      const auto *Result = Builder.createNeg(*Operand);
      ASSERT_NE(Result, nullptr);
      EXPECT_EQ(Result->operand(), Operand);
      EXPECT_EQ(Result->type(), Float);
      EXPECT_EQ(Operand->bits(), Entry.Bits);
      EXPECT_EQ(Block->instruction(PreviousCount), Result);
      EXPECT_EQ(Block->instructionCount(), PreviousCount + 1);
    }
    EXPECT_EQ(Builder.insertBlock(), Block);
  }

  // Negation rejects foreign, unregistered and nonnumeric operands without changing a valid insertion block.
  TEST(IRNegInstructionTest, RejectsForeignUnpooledAndNonnumericOperands)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    Module Owner(Context);
    Function FunctionValue(*Types.getFunctionType(Types.getVoidType()));
    auto *Entry = FunctionValue.createBlock();
    ASSERT_NE(Entry, nullptr);
    UnpooledNumericConstant Unpooled(Types.getInt32Type());
    const auto *ForeignInteger = Foreign.constantPool().getSignedIntegerConstant(Foreign.typePool().getInt32Type(), 1);
    const auto *ForeignFloat = Foreign.constantPool().getFloatConstant(1.5);
    const Value *Operands[] = {
        ForeignInteger,
        ForeignFloat,
        &Unpooled,
        Context.constantPool().getBoolConstant(true),
        Context.constantPool().getNullPointerConstant(),
        Context.constantPool().getStringConstant("text"),
        Types.getInt32Type(),
        &FunctionValue,
        Entry,
        &Owner,
    };
    IRBuilder Builder(Context);
    EXPECT_EQ(Builder.insertBlock(), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    for (const auto *Operand : Operands)
    {
      ASSERT_NE(Operand, nullptr);
      EXPECT_EQ(Builder.createNeg(*Operand), nullptr);
    }
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
  }

  // Runtime negation preserves the type and borrowed operand of current-function numeric parameters across integer and floating widths.
  TEST(IRNegInstructionTest, EmitsRuntimeNegationForNumericParameterTypes)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const Type *NumericTypes[] = {
        Types.getInt8Type(),
        Types.getUInt8Type(),
        Types.getInt16Type(),
        Types.getUInt16Type(),
        Types.getInt32Type(),
        Types.getUInt32Type(),
        Types.getInt64Type(),
        Types.getUInt64Type(),
        Types.getFloat32Type(),
        Types.getFloat64Type(),
    };
    IRBuilder Builder(Context);
    for (const auto *Numeric : NumericTypes)
    {
      SCOPED_TRACE(Numeric->toString());
      Function Owner(*Types.getFunctionType(Numeric, {Numeric}));
      auto *Entry = Owner.createBlock();
      ASSERT_NE(Entry, nullptr);
      ASSERT_TRUE(Builder.setInsertPoint(*Entry));
      const auto *Result = Builder.createNeg(*Owner.parameter(0));
      ASSERT_NE(Result, nullptr);
      ASSERT_TRUE(NegInstruction::classof(Result));
      EXPECT_EQ(Result->type(), Numeric);
      EXPECT_EQ(static_cast<const NegInstruction *>(Result)->operand(), Owner.parameter(0));
      EXPECT_EQ(Entry->instructionCount(), 1U);
      EXPECT_EQ(Entry->instruction(0), Result);
      EXPECT_NE(Builder.createReturn(*Result), nullptr);
      Builder.clearInsertPoint();
    }
  }

  // Negation results can feed another negation, call arguments, integer comparisons and return values in the same function.
  TEST(IRNegInstructionTest, ConnectsRuntimeNegationWithOtherInstructions)
  {
    IRContext Context(Target);
    const auto *Integer = Context.typePool().getInt32Type();
    const auto *Signature = Context.typePool().getFunctionType(Integer, {Integer});
    Function Caller(*Signature);
    Function Callee(*Signature);
    auto *Entry = Caller.createBlock();
    ASSERT_NE(Entry, nullptr);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    const auto *First = Builder.createNeg(*Caller.parameter(0));
    ASSERT_NE(First, nullptr);
    const auto *Second = Builder.createNeg(*First);
    ASSERT_NE(Second, nullptr);
    ASSERT_TRUE(NegInstruction::classof(Second));
    EXPECT_EQ(static_cast<const NegInstruction *>(Second)->operand(), First);
    const auto *Call = Builder.createCall(Callee, {Argument(*Second)});
    ASSERT_NE(Call, nullptr);
    const auto *Third = Builder.createNeg(*Call);
    ASSERT_NE(Third, nullptr);
    const auto *Compare = Builder.createICmp(ICmpPredicate::SignedLessThan, *Third, *First);
    ASSERT_NE(Compare, nullptr);
    const auto *Return = Builder.createReturn(*Third);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Call->argument(0)->value(), Second);
    EXPECT_EQ(Compare->left(), Third);
    EXPECT_EQ(Compare->right(), First);
    EXPECT_EQ(Return->returnedValue(), Third);
    ASSERT_EQ(Entry->instructionCount(), 6U);
    EXPECT_EQ(Entry->instruction(0), First);
    EXPECT_EQ(Entry->instruction(1), Second);
    EXPECT_EQ(Entry->instruction(2), Call);
    EXPECT_EQ(Entry->instruction(3), Third);
    EXPECT_EQ(Entry->instruction(4), Compare);
    EXPECT_EQ(Entry->instruction(5), Return);
  }

  // Runtime negation rejects foreign-function, detached, storage and variadic operands without changing a valid block.
  TEST(IRNegInstructionTest, RejectsUnusableRuntimeOperandsWithoutMutation)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.getInt32Type();
    const auto *Signature = Types.getFunctionType(Integer, {Integer, Types.getBoolType(), Types.getPointerType(Integer), {Integer, true}});
    ASSERT_NE(Signature, nullptr);
    Function Owner(*Signature);
    Function Other(*Types.getFunctionType(Integer, {Integer}));
    Function ForeignOwner(*Foreign.typePool().getFunctionType(Foreign.typePool().getInt32Type(), {Foreign.typePool().getInt32Type()}));
    auto *Entry = Owner.createBlock();
    auto *OtherEntry = Other.createBlock();
    ASSERT_NE(Entry, nullptr);
    ASSERT_NE(OtherEntry, nullptr);
    const auto *OtherInstruction = OtherEntry->appendInstruction(std::make_unique<NegInstruction>(*Other.parameter(0)));
    ASSERT_NE(OtherInstruction, nullptr);
    DetachedNumericInstruction Detached(Integer);
    GlobalVariable Storage(*Integer);
    IRBuilder Builder(Context);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    const Value *Operands[] = {
        Other.parameter(0),
        ForeignOwner.parameter(0),
        OtherInstruction,
        &Detached,
        &Storage,
        Owner.parameter(1),
        Owner.parameter(2),
        Owner.parameter(3),
    };
    for (const auto *Operand : Operands)
    {
      ASSERT_NE(Operand, nullptr);
      EXPECT_EQ(Builder.createNeg(*Operand), nullptr);
    }
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_EQ(OtherEntry->instructionCount(), 1U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
  }
} // namespace ink::ir::test
