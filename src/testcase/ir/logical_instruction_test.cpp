#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

namespace ink::ir::test
{
  namespace
  {
    struct LogicalTestContext
    {
        core::CompilationContext Compilation;
        IRContext Context{Compilation};
        IRBuilder Builder{Context};
        const BoolConstant &True = Context.constantPool().getBoolConstant(true);
        const BoolConstant &False = Context.constantPool().getBoolConstant(false);
        const IntegerType &Int32 = *Context.typePool().getType<TypeKind::Integer>(32, true);
        const IntegerConstant &One = *Context.constantPool().getIntegerConstant(Int32, IntegerBits(32, 1));
    };
  } // namespace

  // Detached logical operations preserve operand identity and bool result types without consuming the insertion point.
  TEST(IRLogicalInstructionTest, RetainsBoolOperandsAndConcreteKinds)
  {
    LogicalTestContext Test;
    auto Block = Test.Builder.createBasicBlock();
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Block));
    auto Not = Test.Builder.createDetachedLogicalNotInstruction(Test.True);
    auto And = Test.Builder.createDetachedLogicalAndInstruction(Test.True, Test.False);
    auto Or = Test.Builder.createDetachedLogicalOrInstruction(Test.False, Test.True);
    ASSERT_NE(Not, nullptr);
    ASSERT_NE(And, nullptr);
    ASSERT_NE(Or, nullptr);
    EXPECT_EQ(&Not->operand(), &Test.True);
    EXPECT_EQ(&And->left(), &Test.True);
    EXPECT_EQ(&And->right(), &Test.False);
    EXPECT_EQ(&Or->left(), &Test.False);
    EXPECT_EQ(&Or->right(), &Test.True);
    EXPECT_TRUE(LogicalNotInstruction::classof(Not.get()));
    EXPECT_TRUE(LogicalAndInstruction::classof(And.get()));
    EXPECT_TRUE(LogicalOrInstruction::classof(Or.get()));
    EXPECT_FALSE(LogicalNotInstruction::classof(nullptr));
    EXPECT_FALSE(LogicalAndInstruction::classof(Not.get()));
    EXPECT_FALSE(LogicalOrInstruction::classof(And.get()));
    const Value *Operations[] = {Not.get(), And.get(), Or.get()};
    for (const auto *Operation : Operations)
    {
      EXPECT_EQ(&Operation->type(), &Test.True.type());
      EXPECT_EQ(Operation->outer(), nullptr);
      EXPECT_FALSE(Operation->isTerminator());
    }
    EXPECT_TRUE(Block->values().empty());
    EXPECT_EQ(Test.Builder.insertBlock(), Block.get());
  }

  // Logical factories reject integer operands and either foreign operand without modifying the selected block.
  TEST(IRLogicalInstructionTest, RejectsNonBoolAndForeignOperands)
  {
    LogicalTestContext Test;
    LogicalTestContext Foreign;
    auto Block = Test.Builder.createBasicBlock();
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Block));
    EXPECT_EQ(Test.Builder.createDetachedLogicalNotInstruction(Test.One), nullptr);
    EXPECT_EQ(Test.Builder.createDetachedLogicalNotInstruction(Foreign.True), nullptr);
    EXPECT_EQ(Test.Builder.createDetachedLogicalAndInstruction(Test.One, Test.One), nullptr);
    EXPECT_EQ(Test.Builder.createDetachedLogicalOrInstruction(Test.One, Test.One), nullptr);
    const Value *Invalid[] = {&Test.One, &Foreign.True};
    for (const auto *Operand : Invalid)
    {
      EXPECT_EQ(Test.Builder.createLogicalNotInstruction(*Operand), nullptr);
      EXPECT_EQ(Test.Builder.createLogicalAndInstruction(*Operand, Test.True), nullptr);
      EXPECT_EQ(Test.Builder.createLogicalAndInstruction(Test.True, *Operand), nullptr);
      EXPECT_EQ(Test.Builder.createLogicalOrInstruction(*Operand, Test.True), nullptr);
      EXPECT_EQ(Test.Builder.createLogicalOrInstruction(Test.True, *Operand), nullptr);
    }
    EXPECT_TRUE(Block->values().empty());
  }

  // Comparison factories accept all integer predicates and bool equality, retaining operands while producing canonical bool results.
  TEST(IRCompareInstructionTest, PreservesPredicatesAndBoolResultTypes)
  {
    LogicalTestContext Test;
    const core::ComparisonPredicate Predicates[] = {
        core::ComparisonPredicate::Equal,
        core::ComparisonPredicate::NotEqual,
        core::ComparisonPredicate::Less,
        core::ComparisonPredicate::LessEqual,
        core::ComparisonPredicate::Greater,
        core::ComparisonPredicate::GreaterEqual,
    };
    for (auto Predicate : Predicates)
    {
      auto Compare = Test.Builder.createDetachedCompareInstruction(Predicate, Test.One, Test.One);
      ASSERT_NE(Compare, nullptr);
      EXPECT_EQ(Compare->predicate(), Predicate);
      EXPECT_EQ(&Compare->left(), &Test.One);
      EXPECT_EQ(&Compare->right(), &Test.One);
      EXPECT_EQ(&Compare->type(), &Test.True.type());
      EXPECT_EQ(Compare->outer(), nullptr);
      EXPECT_FALSE(Compare->isTerminator());
      EXPECT_TRUE(CompareInstruction::classof(Compare.get()));
    }
    EXPECT_NE(Test.Builder.createDetachedCompareInstruction(core::ComparisonPredicate::Equal, Test.True, Test.False), nullptr);
    EXPECT_NE(Test.Builder.createDetachedCompareInstruction(core::ComparisonPredicate::NotEqual, Test.True, Test.False), nullptr);
    EXPECT_FALSE(CompareInstruction::classof(&Test.One));
    EXPECT_FALSE(CompareInstruction::classof(nullptr));
  }

  // Comparisons reject unknown predicates, ordered bools, floating values, different widths or signedness, and foreign operands.
  TEST(IRCompareInstructionTest, RejectsInvalidPredicatesAndOperandTypes)
  {
    LogicalTestContext Test;
    LogicalTestContext Foreign;
    const auto *Unsigned = Test.Context.typePool().getType<TypeKind::Integer>(32, false);
    const auto *Wide = Test.Context.typePool().getType<TypeKind::Integer>(64, true);
    const auto *Float = Test.Context.typePool().getType<TypeKind::Float>(32);
    const auto *UnsignedOne = Test.Context.constantPool().getIntegerConstant(*Unsigned, IntegerBits(32, 1));
    const auto *WideOne = Test.Context.constantPool().getIntegerConstant(*Wide, IntegerBits(64, 1));
    const auto *FloatZero = Test.Context.constantPool().getFloatConstant(*Float, FloatBits(32, 0));
    const Value *Invalid[] = {&Test.True, UnsignedOne, WideOne, FloatZero, &Foreign.One};
    for (const auto *Operand : Invalid)
    {
      EXPECT_EQ(Test.Builder.createDetachedCompareInstruction(core::ComparisonPredicate::Equal, Test.One, *Operand), nullptr);
      EXPECT_EQ(Test.Builder.createDetachedCompareInstruction(core::ComparisonPredicate::Equal, *Operand, Test.One), nullptr);
    }
    EXPECT_EQ(Test.Builder.createDetachedCompareInstruction(core::ComparisonPredicate::Equal, *FloatZero, *FloatZero), nullptr);
    EXPECT_EQ(Test.Builder.createDetachedCompareInstruction(core::ComparisonPredicate::Equal, Test.True, Foreign.True), nullptr);
    for (auto Predicate : {core::ComparisonPredicate::Less, core::ComparisonPredicate::LessEqual, core::ComparisonPredicate::Greater, core::ComparisonPredicate::GreaterEqual})
    {
      EXPECT_EQ(Test.Builder.createDetachedCompareInstruction(Predicate, Test.True, Test.False), nullptr);
    }
    EXPECT_EQ(Test.Builder.createDetachedCompareInstruction(static_cast<core::ComparisonPredicate>(99), Test.One, Test.One), nullptr);
    EXPECT_EQ(Test.Builder.createDetachedCompareInstruction(static_cast<core::ComparisonPredicate>(99), Test.True, Test.False), nullptr);
  }

  // Every inserting factory requires a live point, respects terminators, and permits ordinary operations before a return.
  TEST(IRLogicalInstructionTest, EnforcesInsertionPointsAndTerminalBoundaries)
  {
    LogicalTestContext Test;
    EXPECT_EQ(Test.Builder.createLogicalNotInstruction(Test.True), nullptr);
    EXPECT_EQ(Test.Builder.createLogicalAndInstruction(Test.True, Test.False), nullptr);
    EXPECT_EQ(Test.Builder.createLogicalOrInstruction(Test.True, Test.False), nullptr);
    EXPECT_EQ(Test.Builder.createCompareInstruction(core::ComparisonPredicate::Equal, Test.One, Test.One), nullptr);
    auto Function = Test.Builder.createFunction(Test.Context.namePool().intern("Logical"), *Test.Context.typePool().getType<TypeKind::Function>(Test.True.type()));
    ASSERT_NE(Function, nullptr);
    auto *Block = Test.Builder.createFunctionBody(*Function);
    ASSERT_NE(Block, nullptr);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Block));
    auto *Return = Test.Builder.createReturnInstruction(&Test.True);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Test.Builder.createLogicalNotInstruction(Test.True), nullptr);
    EXPECT_EQ(Test.Builder.createLogicalAndInstruction(Test.True, Test.False), nullptr);
    EXPECT_EQ(Test.Builder.createLogicalOrInstruction(Test.True, Test.False), nullptr);
    EXPECT_EQ(Test.Builder.createCompareInstruction(core::ComparisonPredicate::Equal, Test.One, Test.One), nullptr);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Return));
    EXPECT_NE(Test.Builder.createLogicalNotInstruction(Test.True), nullptr);
    EXPECT_NE(Test.Builder.createLogicalAndInstruction(Test.True, Test.False), nullptr);
    EXPECT_NE(Test.Builder.createLogicalOrInstruction(Test.True, Test.False), nullptr);
    EXPECT_NE(Test.Builder.createCompareInstruction(core::ComparisonPredicate::Equal, Test.One, Test.One), nullptr);
    EXPECT_EQ(Block->values().size(), 5U);
    EXPECT_EQ(Block->terminator(), Return);
  }
} // namespace ink::ir::test
