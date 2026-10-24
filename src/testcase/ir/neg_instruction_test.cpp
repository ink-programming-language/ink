#include "ink/core/numeric/integer_bits.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/neg_instruction.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  static_assert(std::is_base_of_v<Instruction, NegInstruction>);
  static_assert(std::is_same_v<decltype(std::declval<IRBuilder &>().createNeg(std::declval<const Value &>())), NegInstruction *>);
  static_assert(std::is_same_v<decltype(std::declval<const NegInstruction &>().operand()), const Value *>);

  // A directly constructed negation borrows its operand and result type, identifies as a nonterminating instruction and formats its operation name.
  TEST(IRNegInstructionTest, RecordsBorrowedOperandAndInstructionIdentity)
  {
    IRContext Context(Target);
    const auto *Integer = Context.typePool().get<ink::ir::IntegerType>(16, true);
    Function Owner(*Context.typePool().get<ink::ir::FunctionType>(Integer, {Integer}));
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

  // Signed constants of each builtin width, including each minimum, remain borrowed operands of distinct instructions without being evaluated or folded.
  TEST(IRNegInstructionTest, EmitsForSignedIntegerConstantsWithoutEvaluation)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    auto &Constants = Context.constantPool();
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>()));
    auto *Entry = Owner.createBlock();
    ASSERT_NE(Entry, nullptr);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    for (const std::uint32_t Width : {8U, 16U, 32U, 64U})
    {
      SCOPED_TRACE(Width);
      const auto *Integer = Types.get<ink::ir::IntegerType>(Width, true);
      ASSERT_NE(Integer, nullptr);
      const auto Maximum = Integer->maximumValue().tryGetInt64();
      const auto Minimum = Integer->minimumValue().tryGetInt64();
      ASSERT_TRUE(Maximum.has_value());
      ASSERT_TRUE(Minimum.has_value());
      const std::int64_t Values[] = {0, 1, -1, *Maximum, -*Maximum, *Minimum};
      for (const auto Value : Values)
      {
        SCOPED_TRACE(Value);
        const auto *Operand = Constants.get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(Value));
        ASSERT_NE(Operand, nullptr);
        const auto OriginalBits = Operand->bits();
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

  // Unsigned zero and nonzero constants in each builtin width produce instructions while preserving their original operand values.
  TEST(IRNegInstructionTest, EmitsForUnsignedIntegerConstantsWithoutEvaluation)
  {
    IRContext Context(Target);
    auto &Constants = Context.constantPool();
    Function Owner(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()));
    auto *Entry = Owner.createBlock();
    ASSERT_NE(Entry, nullptr);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    for (const std::uint32_t Width : {8U, 16U, 32U, 64U})
    {
      SCOPED_TRACE(Width);
      const auto *Integer = Context.typePool().get<ink::ir::IntegerType>(Width, false);
      ASSERT_NE(Integer, nullptr);
      const auto Maximum = Integer->maximumValue().tryGetUInt64();
      ASSERT_TRUE(Maximum.has_value());
      for (const std::uint64_t Value : {std::uint64_t{0}, std::uint64_t{1}, *Maximum})
      {
        const auto *Operand = Constants.get<ink::ir::IntegerConstant>(Integer, static_cast<std::uint64_t>(Value));
        ASSERT_NE(Operand, nullptr);
        const std::size_t PreviousCount = Entry->instructionCount();
        const auto *Result = Builder.createNeg(*Operand);
        ASSERT_NE(Result, nullptr);
        EXPECT_EQ(Result->operand(), Operand);
        EXPECT_EQ(Result->type(), Integer);
        EXPECT_EQ(Operand->bits().tryGetUInt64(), Value);
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
    Function Owner(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()));
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
      const auto *Float = Context.typePool().get<ink::ir::FloatType>(Entry.Width);
      const auto Bits = core::IntegerBits::fromUnsigned(Entry.Width, Entry.Bits);
      ASSERT_TRUE(Bits.has_value());
      const auto *Operand = Constants.get<ink::ir::FloatConstant>(Float, *Bits);
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

  // Runtime negation preserves the type and borrowed operand of current-function numeric parameters across integer and floating widths.
  TEST(IRNegInstructionTest, EmitsRuntimeNegationForNumericParameterTypes)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const Type *NumericTypes[] = {
        Types.get<ink::ir::IntegerType>(8, true),
        Types.get<ink::ir::IntegerType>(8, false),
        Types.get<ink::ir::IntegerType>(16, true),
        Types.get<ink::ir::IntegerType>(16, false),
        Types.get<ink::ir::IntegerType>(32, true),
        Types.get<ink::ir::IntegerType>(32, false),
        Types.get<ink::ir::IntegerType>(64, true),
        Types.get<ink::ir::IntegerType>(64, false),
        Types.get<ink::ir::FloatType>(32),
        Types.get<ink::ir::FloatType>(64),
    };
    IRBuilder Builder(Context);
    for (const auto *Numeric : NumericTypes)
    {
      SCOPED_TRACE(Numeric->toString());
      Function Owner(*Types.get<ink::ir::FunctionType>(Numeric, {Numeric}));
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
    const auto *Integer = Context.typePool().get<ink::ir::IntegerType>(32, true);
    const auto *Signature = Context.typePool().get<ink::ir::FunctionType>(Integer, {Integer});
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
} // namespace ink::ir::test
