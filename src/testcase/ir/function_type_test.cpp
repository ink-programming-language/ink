#include "ink/ir/ir_builder.h"
#include "ink/ir/function/function_type.h"
#include "ink/ir/context.h"

#include <gtest/gtest.h>

#include <vector>

namespace ink::ir::test
{
  // Return type, parameter order, multiplicity and exact type identity all participate in signature interning.
  TEST(IRFunctionTypeTest, SignaturesAreCanonicalByReturnAndOrderedParameters)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const IntegerType *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    const IntegerType *Unsigned = Context.typePool().getType<TypeKind::Integer>(32, false);
    ASSERT_NE(Integer, nullptr);
    ASSERT_NE(Unsigned, nullptr);
    const Type *Parameters[] = {
        Integer,
        &Context.typePool().getType<TypeKind::Bool>(),
    };
    const Type *Reversed[] = {
        &Context.typePool().getType<TypeKind::Bool>(),
        Integer,
    };
    const Type *Different[] = {
        Unsigned,
        &Context.typePool().getType<TypeKind::Bool>(),
    };
    const Type *Repeated[] = {
        Integer,
        Integer,
    };
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(*Integer, Parameters);
    ASSERT_NE(Signature, nullptr);
    EXPECT_EQ(Signature, Context.typePool().getType<TypeKind::Function>(*Integer, Parameters));
    EXPECT_NE(Signature, Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>(), Parameters));
    EXPECT_NE(Signature, Context.typePool().getType<TypeKind::Function>(*Integer, Reversed));
    EXPECT_NE(Signature, Context.typePool().getType<TypeKind::Function>(*Integer, Different));
    EXPECT_NE(Signature, Context.typePool().getType<TypeKind::Function>(*Integer, Repeated));
    EXPECT_NE(Signature, Context.typePool().getType<TypeKind::Function>(*Integer, std::span(Parameters).first(1)));
    EXPECT_NE(Signature, Context.typePool().getType<TypeKind::Function>(*Integer));
    EXPECT_EQ(&Signature->returnType(), Integer);
    ASSERT_EQ(Signature->parameterTypes().size(), 2U);
    EXPECT_EQ(Signature->parameterTypes()[0], Integer);
    EXPECT_EQ(Signature->parameterTypes()[1], &Context.typePool().getType<TypeKind::Bool>());
    EXPECT_EQ(&Signature->type(), &Context.typePool().getType<TypeKind::Meta>());
    EXPECT_EQ(Signature->typeKind(), TypeKind::Function);
    EXPECT_TRUE(Type::classof(Signature));
    EXPECT_TRUE(BuiltinType::classof(Signature));
    EXPECT_TRUE(FunctionType::classof(Signature));
    EXPECT_FALSE(UserDefinedType::classof(Signature));
    const FunctionType *Empty = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Empty, nullptr);
    EXPECT_TRUE(Empty->parameterTypes().empty());
    EXPECT_EQ(&Empty->returnType(), &Context.typePool().getType<TypeKind::Void>());
    EXPECT_EQ(Empty, Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>(), {}));
  }

  // Null parameters and foreign return or parameter types cannot create a local signature.
  TEST(IRFunctionTypeTest, InvalidComponentsAreRejectedAndContextsRemainDistinct)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRContext Other(Compilation);
    const Type *Parameters[] = {
        &Context.typePool().getType<TypeKind::Bool>(),
        &Context.typePool().getType<TypeKind::Bool>(),
    };
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>(), Parameters);
    ASSERT_NE(Signature, nullptr);
    EXPECT_EQ(Context.typePool().getType<TypeKind::Function>(Other.typePool().getType<TypeKind::Bool>(), Parameters), nullptr);
    for (std::size_t Index = 0; Index < 2; ++Index)
    {
      Parameters[Index] = nullptr;
      EXPECT_EQ(Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>(), Parameters), nullptr);
      Parameters[Index] = &Other.typePool().getType<TypeKind::Bool>();
      EXPECT_EQ(Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>(), Parameters), nullptr);
      Parameters[Index] = &Context.typePool().getType<TypeKind::Bool>();
    }
    EXPECT_EQ(Signature, Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>(), Parameters));
    const Type *OtherParameters[] = {
        &Other.typePool().getType<TypeKind::Bool>(),
        &Other.typePool().getType<TypeKind::Bool>(),
    };
    const FunctionType *Foreign = Other.typePool().getType<TypeKind::Function>(Other.typePool().getType<TypeKind::Bool>(), OtherParameters);
    ASSERT_NE(Foreign, nullptr);
    EXPECT_NE(Signature, Foreign);
    EXPECT_EQ(&Foreign->context(), &Other);
    EXPECT_FALSE(FunctionType::classof(nullptr));
    EXPECT_FALSE(FunctionType::classof(&Context.typePool().getType<TypeKind::Bool>()));
  }

  // Parameter storage is copied once and borrowed signature views survive caller changes and type-map growth.
  TEST(IRFunctionTypeTest, OwnedParameterListsAndAddressesSurviveGrowth)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const FunctionType *Signature = nullptr;
    {
      std::vector<const Type *> Parameters(3, &Context.typePool().getType<TypeKind::Bool>());
      Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>(), Parameters);
      ASSERT_NE(Signature, nullptr);
      Parameters[0] = &Context.typePool().getType<TypeKind::Meta>();
      Parameters.clear();
    }
    const auto Parameters = Signature->parameterTypes();
    ASSERT_EQ(Parameters.size(), 3U);
    for (std::uint32_t Width = 1; Width <= 2048; ++Width)
    {
      const IntegerType *Integer = Context.typePool().getType<TypeKind::Integer>(Width, false);
      ASSERT_NE(Integer, nullptr);
      ASSERT_NE(Context.typePool().getType<TypeKind::Function>(*Integer, Parameters), nullptr);
    }
    EXPECT_EQ(Signature->parameterTypes().data(), Parameters.data());
    for (const Type *Parameter : Parameters)
    {
      EXPECT_EQ(Parameter, &Context.typePool().getType<TypeKind::Bool>());
    }
    EXPECT_EQ(Signature, Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>(), Parameters));
  }

  // Function signatures compose with other function types, nominal types and pointer/reference constructors.
  TEST(IRFunctionTypeTest, SignaturesSupportNestedAndNominalComponents)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const ClassType *Record = Builder.createClassType(Context.namePool().intern("Record"));
    ASSERT_NE(Record, nullptr);
    const FunctionType *Factory = Context.typePool().getType<TypeKind::Function>(*Record);
    ASSERT_NE(Factory, nullptr);
    const Type *Parameters[] = {
        Factory,
        Record,
    };
    const FunctionType *HigherOrder = Context.typePool().getType<TypeKind::Function>(*Factory, Parameters);
    ASSERT_NE(HigherOrder, nullptr);
    EXPECT_EQ(&HigherOrder->returnType(), Factory);
    EXPECT_EQ(HigherOrder->parameterTypes()[0], Factory);
    EXPECT_EQ(HigherOrder->parameterTypes()[1], Record);
    EXPECT_TRUE(BuiltinType::classof(HigherOrder));
    const PointerType *Pointer = Context.typePool().getType<TypeKind::Pointer>(*Factory, AccessKind::ReadOnly);
    const ReferenceType *Reference = Context.typePool().getType<TypeKind::Reference>(*Factory, AccessKind::ReadOnly);
    ASSERT_NE(Pointer, nullptr);
    ASSERT_NE(Reference, nullptr);
    EXPECT_EQ(&Pointer->pointeeType(), Factory);
    EXPECT_EQ(&Reference->referentType(), Factory);
  }
} // namespace ink::ir::test
