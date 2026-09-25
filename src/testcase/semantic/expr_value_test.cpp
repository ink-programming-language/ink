#include "ink/semantic/model/context.h"
#include "ink/parser/ast.h"

#include <gtest/gtest.h>

namespace ink::semantic::test
{
  // Reusing an AST in multiple analyses preserves separate identities, types and stable addresses.
  TEST(SemanticExprValueTest, ExpressionValuesRetainOriginAndIndependentIdentity)
  {
    core::CompilationContext Compilation;
    parser::NameExpr Expression({}, {});
    SemanticContext Context(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const IntegerType *Int64 = Context.getIntegerType(64, true);
    const ExprValue *First = Context.createExprValue(*Int32, Expression);
    const ExprValue *Second = Context.createExprValue(*Int32, Expression);
    const ExprValue *Wider = Context.createExprValue(*Int64, Expression);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Wider, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_NE(First, Wider);
    EXPECT_EQ(&First->expression(), &Expression);
    EXPECT_EQ(&Wider->expression(), &Expression);
    EXPECT_EQ(&First->type(), Int32);
    EXPECT_EQ(&Wider->type(), Int64);
    EXPECT_EQ(&First->context(), &Context);
    EXPECT_FALSE(First->sourceId().valid());
    const Value *ValueObject = First;
    EXPECT_EQ(ValueObject->kind(), ValueKind::ExprValue);
    EXPECT_TRUE(ExprValue::classof(ValueObject));
    EXPECT_FALSE(Type::classof(ValueObject));
    EXPECT_FALSE(Constant::classof(ValueObject));
    EXPECT_FALSE(BuiltinType::classof(ValueObject));
    EXPECT_FALSE(UserDefinedType::classof(ValueObject));
    EXPECT_FALSE(ExprValue::classof(nullptr));
    EXPECT_FALSE(ExprValue::classof(Int32));
    EXPECT_FALSE(ExprValue::classof(&Context.getBoolConstant(true)));
    for (std::size_t Index = 0; Index < 1024; ++Index)
    {
      ASSERT_NE(Context.createExprValue(*Int32, Expression), nullptr);
    }
    EXPECT_EQ(&First->expression(), &Expression);
    EXPECT_EQ(&First->type(), Int32);
    EXPECT_TRUE(ExprValue::classof(ValueObject));
  }

  // An expression result cannot borrow a type owned by a different model context.
  TEST(SemanticExprValueTest, ExpressionFactoryRejectsForeignTypes)
  {
    core::CompilationContext Compilation;
    parser::NameExpr Expression({}, {});
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    EXPECT_EQ(Context.createExprValue(Other.getBoolType(), Expression), nullptr);
    const ExprValue *Local = Context.createExprValue(Context.getBoolType(), Expression);
    ASSERT_NE(Local, nullptr);
    EXPECT_EQ(&Local->context(), &Context);
  }

  // Store operands may reference typed expressions, but meta values and foreign expressions are rejected.
  TEST(SemanticExprValueTest, ExpressionsCanBeStoredWithExactLocalTypes)
  {
    core::CompilationContext Compilation;
    parser::NameExpr Expression({}, {});
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    AllocaInstruction *Slot = Context.createAllocaInstruction(*Int32);
    const ExprValue *Computed = Context.createExprValue(*Int32, Expression);
    const ExprValue *Foreign = Other.createExprValue(*Other.getIntegerType(32, true), Expression);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(Computed, nullptr);
    ASSERT_NE(Foreign, nullptr);
    EXPECT_EQ(Context.createStoreInstruction(*Slot, *Foreign), nullptr);
    EXPECT_EQ(Context.createStoreInstruction(*Slot, *Int32), nullptr);
    EXPECT_EQ(Context.createStoreInstruction(*Slot, Context.getBoolConstant(false)), nullptr);
    const StoreInstruction *Store = Context.createStoreInstruction(*Slot, *Computed);
    ASSERT_NE(Store, nullptr);
    EXPECT_EQ(&Store->storedValue(), Computed);
    EXPECT_EQ(&Store->address(), Slot);
    EXPECT_EQ(&Store->type(), &Context.getVoidType());
    EXPECT_EQ(&Computed->expression(), &Expression);
  }
} // namespace ink::semantic::test
