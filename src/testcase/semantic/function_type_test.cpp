#include "ink/semantic/model/context.h"

#include <gtest/gtest.h>

#include <vector>

namespace ink::semantic::test
{
  // Return type, parameter order, multiplicity and exact type identity all participate in signature interning.
  TEST(SemanticFunctionTypeTest, SignaturesAreCanonicalByReturnAndOrderedParameters)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Integer = Context.getIntegerType(32, true);
    const IntegerType *Unsigned = Context.getIntegerType(32, false);
    ASSERT_NE(Integer, nullptr);
    ASSERT_NE(Unsigned, nullptr);
    const Type *Parameters[] = {
        Integer,
        &Context.getBoolType(),
    };
    const Type *Reversed[] = {
        &Context.getBoolType(),
        Integer,
    };
    const Type *Different[] = {
        Unsigned,
        &Context.getBoolType(),
    };
    const Type *Repeated[] = {
        Integer,
        Integer,
    };
    const FunctionType *Signature = Context.getFunctionType(*Integer, Parameters);
    ASSERT_NE(Signature, nullptr);
    EXPECT_EQ(Signature, Context.getFunctionType(*Integer, Parameters));
    EXPECT_NE(Signature, Context.getFunctionType(Context.getBoolType(), Parameters));
    EXPECT_NE(Signature, Context.getFunctionType(*Integer, Reversed));
    EXPECT_NE(Signature, Context.getFunctionType(*Integer, Different));
    EXPECT_NE(Signature, Context.getFunctionType(*Integer, Repeated));
    EXPECT_NE(Signature, Context.getFunctionType(*Integer, std::span(Parameters).first(1)));
    EXPECT_NE(Signature, Context.getFunctionType(*Integer));
    EXPECT_EQ(&Signature->returnType(), Integer);
    ASSERT_EQ(Signature->parameterTypes().size(), 2U);
    EXPECT_EQ(Signature->parameterTypes()[0], Integer);
    EXPECT_EQ(Signature->parameterTypes()[1], &Context.getBoolType());
    EXPECT_EQ(&Signature->type(), &Context.getMetaType());
    EXPECT_EQ(Signature->typeKind(), TypeKind::Function);
    EXPECT_TRUE(Type::classof(Signature));
    EXPECT_TRUE(BuiltinType::classof(Signature));
    EXPECT_TRUE(FunctionType::classof(Signature));
    EXPECT_FALSE(UserDefinedType::classof(Signature));
    const FunctionType *Empty = Context.getFunctionType(Context.getVoidType());
    ASSERT_NE(Empty, nullptr);
    EXPECT_TRUE(Empty->parameterTypes().empty());
    EXPECT_EQ(&Empty->returnType(), &Context.getVoidType());
    EXPECT_EQ(Empty, Context.getFunctionType(Context.getVoidType(), {}));
  }

  // Null parameters and foreign return or parameter types cannot create a local signature.
  TEST(SemanticFunctionTypeTest, InvalidComponentsAreRejectedAndContextsRemainDistinct)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const Type *Parameters[] = {
        &Context.getBoolType(),
        &Context.getBoolType(),
    };
    const FunctionType *Signature = Context.getFunctionType(Context.getBoolType(), Parameters);
    ASSERT_NE(Signature, nullptr);
    EXPECT_EQ(Context.getFunctionType(Other.getBoolType(), Parameters), nullptr);
    for (std::size_t Index = 0; Index < 2; ++Index)
    {
      Parameters[Index] = nullptr;
      EXPECT_EQ(Context.getFunctionType(Context.getBoolType(), Parameters), nullptr);
      Parameters[Index] = &Other.getBoolType();
      EXPECT_EQ(Context.getFunctionType(Context.getBoolType(), Parameters), nullptr);
      Parameters[Index] = &Context.getBoolType();
    }
    EXPECT_EQ(Signature, Context.getFunctionType(Context.getBoolType(), Parameters));
    const Type *OtherParameters[] = {
        &Other.getBoolType(),
        &Other.getBoolType(),
    };
    const FunctionType *Foreign = Other.getFunctionType(Other.getBoolType(), OtherParameters);
    ASSERT_NE(Foreign, nullptr);
    EXPECT_NE(Signature, Foreign);
    EXPECT_EQ(&Foreign->context(), &Other);
    EXPECT_FALSE(FunctionType::classof(nullptr));
    EXPECT_FALSE(FunctionType::classof(&Context.getBoolType()));
  }

  // Parameter storage is copied once and borrowed signature views survive caller changes and type-map growth.
  TEST(SemanticFunctionTypeTest, OwnedParameterListsAndAddressesSurviveGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const FunctionType *Signature = nullptr;
    {
      std::vector<const Type *> Parameters(3, &Context.getBoolType());
      Signature = Context.getFunctionType(Context.getVoidType(), Parameters);
      ASSERT_NE(Signature, nullptr);
      Parameters[0] = &Context.getMetaType();
      Parameters.clear();
    }
    const auto Parameters = Signature->parameterTypes();
    ASSERT_EQ(Parameters.size(), 3U);
    for (std::uint32_t Width = 1; Width <= 2048; ++Width)
    {
      const IntegerType *Integer = Context.getIntegerType(Width, false);
      ASSERT_NE(Integer, nullptr);
      ASSERT_NE(Context.getFunctionType(*Integer, Parameters), nullptr);
    }
    EXPECT_EQ(Signature->parameterTypes().data(), Parameters.data());
    for (const Type *Parameter : Parameters)
    {
      EXPECT_EQ(Parameter, &Context.getBoolType());
    }
    EXPECT_EQ(Signature, Context.getFunctionType(Context.getVoidType(), Parameters));
  }

  // Function signatures compose with other function types, nominal types and pointer/reference constructors.
  TEST(SemanticFunctionTypeTest, SignaturesSupportNestedAndNominalComponents)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const ClassType *Record = Context.createClassType(Context.namePool().intern("Record"));
    ASSERT_NE(Record, nullptr);
    const FunctionType *Factory = Context.getFunctionType(*Record);
    ASSERT_NE(Factory, nullptr);
    const Type *Parameters[] = {
        Factory,
        Record,
    };
    const FunctionType *HigherOrder = Context.getFunctionType(*Factory, Parameters);
    ASSERT_NE(HigherOrder, nullptr);
    EXPECT_EQ(&HigherOrder->returnType(), Factory);
    EXPECT_EQ(HigherOrder->parameterTypes()[0], Factory);
    EXPECT_EQ(HigherOrder->parameterTypes()[1], Record);
    EXPECT_TRUE(BuiltinType::classof(HigherOrder));
    const PointerType *Pointer = Context.getPointerType(*Factory, AccessKind::ReadOnly);
    const ReferenceType *Reference = Context.getReferenceType(*Factory, AccessKind::ReadOnly);
    ASSERT_NE(Pointer, nullptr);
    ASSERT_NE(Reference, nullptr);
    EXPECT_EQ(&Pointer->pointeeType(), Factory);
    EXPECT_EQ(&Reference->referentType(), Factory);
  }
} // namespace ink::semantic::test
