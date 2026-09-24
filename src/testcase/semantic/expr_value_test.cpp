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

  // A variable initializer accepts a type value, a known constant or an unevaluated expression result.
  TEST(SemanticExprValueTest, InitializersSupportAllThreeValueCategories)
  {
    core::CompilationContext Compilation;
    parser::NameExpr Expression({}, {});
    SemanticContext Context(Compilation);
    const Name X = Context.namePool().intern("X");
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const Value *Initializers[] = {
        Int32,
        Context.getIntegerConstant(*Int32, IntegerBits(32, 0)),
        Context.createExprValue(*Int32, Expression),
    };
    for (const Value *Initializer : Initializers)
    {
      ASSERT_NE(Initializer, nullptr);
      Variable *Declaration = Context.createVariable(X, BindingMutability::Immutable, Initializer);
      ASSERT_NE(Declaration, nullptr);
      EXPECT_EQ(Declaration->initializer(), Initializer);
      EXPECT_EQ(Declaration->type(), nullptr);
      EXPECT_TRUE(Declaration->setType(Initializer->type()));
      EXPECT_EQ(Declaration->type(), &Initializer->type());
      EXPECT_FALSE(Declaration->isMutable());
    }
  }

  // Late initializer publication is idempotent and failed updates preserve the checked declaration.
  TEST(SemanticExprValueTest, InitializersPublishOnceAfterDeclarationRegistration)
  {
    core::CompilationContext Compilation;
    parser::NameExpr Expression({}, {});
    SemanticContext Context(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const IntegerConstant *Zero = Context.getIntegerConstant(*Int32, IntegerBits(32, 0));
    const ExprValue *Computed = Context.createExprValue(*Int32, Expression);
    Variable *Declaration = Context.createVariable(Context.namePool().intern("X"), BindingMutability::Mutable);
    ASSERT_NE(Declaration, nullptr);
    ASSERT_NE(Zero, nullptr);
    ASSERT_NE(Computed, nullptr);
    ASSERT_TRUE(Declaration->setType(*Int32));
    EXPECT_FALSE(Declaration->setInitializer(Context.getBoolConstant(false)));
    EXPECT_EQ(Declaration->initializer(), nullptr);
    EXPECT_TRUE(Declaration->setInitializer(*Computed));
    EXPECT_TRUE(Declaration->setInitializer(*Computed));
    EXPECT_FALSE(Declaration->setInitializer(*Zero));
    EXPECT_FALSE(Declaration->setType(Context.getBoolType()));
    EXPECT_EQ(Declaration->initializer(), Computed);
    EXPECT_EQ(Declaration->type(), Int32);
  }

  // Publishing an initializer before the type enforces the same type contract in the opposite order.
  TEST(SemanticExprValueTest, InitializerFirstRequiresAMatchingPublishedType)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const IntegerConstant *Zero = Context.getIntegerConstant(*Int32, IntegerBits(32, 0));
    const Name X = Context.namePool().intern("X");
    ASSERT_NE(Zero, nullptr);
    Variable *FromFactory = Context.createVariable(X, BindingMutability::Mutable, Zero);
    Variable *FromSetter = Context.createVariable(X, BindingMutability::Mutable);
    ASSERT_NE(FromFactory, nullptr);
    ASSERT_NE(FromSetter, nullptr);
    ASSERT_TRUE(FromSetter->setInitializer(*Zero));
    Variable *Declarations[] = {
        FromFactory,
        FromSetter,
    };
    for (Variable *Declaration : Declarations)
    {
      EXPECT_FALSE(Declaration->setType(Context.getBoolType()));
      EXPECT_FALSE(Declaration->setType(*Other.getIntegerType(32, true)));
      EXPECT_EQ(Declaration->type(), nullptr);
      EXPECT_EQ(Declaration->initializer(), Zero);
      EXPECT_TRUE(Declaration->setType(*Int32));
      EXPECT_EQ(Declaration->type(), Int32);
    }
  }

  // Both creation and deferred initialization reject foreign values without retaining dangling dependencies.
  TEST(SemanticExprValueTest, ForeignInitializersAreRejectedForEveryValueCategory)
  {
    core::CompilationContext Compilation;
    parser::NameExpr Expression({}, {});
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const Name X = Context.namePool().intern("X");
    const IntegerType *ForeignType = Other.getIntegerType(32, true);
    const Value *ForeignValues[] = {
        ForeignType,
        Other.getIntegerConstant(*ForeignType, IntegerBits(32, 0)),
        Other.createExprValue(*ForeignType, Expression),
    };
    Variable *Declaration = Context.createVariable(X, BindingMutability::Mutable);
    ASSERT_NE(Declaration, nullptr);
    for (const Value *Foreign : ForeignValues)
    {
      ASSERT_NE(Foreign, nullptr);
      EXPECT_EQ(Context.createVariable(X, BindingMutability::Mutable, Foreign), nullptr);
      EXPECT_FALSE(Declaration->setInitializer(*Foreign));
      EXPECT_EQ(Declaration->initializer(), nullptr);
      EXPECT_EQ(Declaration->type(), nullptr);
    }
    EXPECT_TRUE(Declaration->setInitializer(Context.getBoolConstant(true)));
  }
} // namespace ink::semantic::test
