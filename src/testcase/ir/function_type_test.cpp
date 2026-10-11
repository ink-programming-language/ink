#include "ink/core/name_pool.h"
#include "ink/target/target_context.h"
#include "ink/ir/type/array_type.h"
#include "ink/ir/type/bool_type.h"
#include "ink/ir/type/float_type.h"
#include "ink/ir/type/function_type.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/pointer_type.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <utility>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    void expectParametersEqual(const std::vector<Parameter> &Actual, const std::vector<Parameter> &Expected)
    {
      ASSERT_EQ(Actual.size(), Expected.size());
      for (std::size_t Index = 0; Index < Expected.size(); ++Index)
      {
        SCOPED_TRACE(Index);
        EXPECT_EQ(Actual[Index].type(), Expected[Index].type());
        EXPECT_EQ(Actual[Index].name(), Expected[Index].name());
        EXPECT_EQ(Actual[Index].hasName(), Expected[Index].hasName());
        EXPECT_EQ(Actual[Index].isVariadic(), Expected[Index].isVariadic());
      }
    }
  } // namespace

  // Omitting parameters creates a valid zero-argument signature with the supplied return type.
  TEST(IRFunctionTypeTest, EmptySignaturePreservesReturnType)
  {
    TypeContext Types(Target64);
    const auto Bool = Types.getBoolType();
    const auto Function = Types.getFunctionType(Bool);
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function->kind(), ValueKind::Function);
    EXPECT_EQ(Function->returnType(), Bool);
    EXPECT_TRUE(Function->parameters().empty());
    EXPECT_EQ(Function->parameterCount(), 0U);
  }

  // Mixed parameter kinds retain their order and repeated type identities alongside an aggregate return type.
  TEST(IRFunctionTypeTest, SignaturePreservesParameterOrderAndDuplicates)
  {
    TypeContext Types(Target64);
    const auto Bool = Types.getBoolType();
    const auto Integer = Types.getIntegerType(32, true);
    const auto Float = Types.getFloatType(64);
    const auto Result = Types.getArrayType(Types.getInt64Type(), 3);
    ASSERT_NE(Integer, nullptr);
    ASSERT_NE(Float, nullptr);
    ASSERT_NE(Result, nullptr);
    const std::vector<Parameter> Expected = {Parameter(Integer), Parameter(Bool), Parameter(Float), Parameter(Integer)};
    const auto Function = Types.getFunctionType(Result, Expected);
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function->returnType(), Result);
    EXPECT_EQ(Function->parameterCount(), Expected.size());
    expectParametersEqual(Function->parameters(), Expected);
  }

  // Copying a parameter vector preserves names and variadic flags after the source is changed and destroyed.
  TEST(IRFunctionTypeTest, SignatureOwnsCopiedParameterList)
  {
    TypeContext Types(Target64);
    const auto Bool = Types.getBoolType();
    const auto Integer = Types.getIntegerType(32, false);
    ASSERT_NE(Integer, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRFunctionTypeTest.CopiedValues");
    ASSERT_TRUE(Name.valid());
    const std::vector<Parameter> Expected = {Parameter(Bool, Name), Parameter(Integer), Parameter(Bool, Name, true)};
    const FunctionType *Function = nullptr;
    {
      std::vector<Parameter> Parameters = Expected;
      Function = Types.getFunctionType(Integer, Parameters);
      ASSERT_NE(Function, nullptr);
      Parameters[0] = Parameter(Integer);
      Parameters.clear();
      expectParametersEqual(Function->parameters(), Expected);
    }
    expectParametersEqual(Function->parameters(), Expected);
    EXPECT_EQ(Function->parameterCount(), Expected.size());
    EXPECT_EQ(Function->returnType(), Integer);
  }

  // Moving a parameter vector transfers complete metadata without retaining a dependency on the source container.
  TEST(IRFunctionTypeTest, SignatureOwnsMovedParameterList)
  {
    TypeContext Types(Target64);
    const auto Bool = Types.getBoolType();
    const auto Pointer = Types.getPointerType(Bool);
    const core::Name Name = core::NamePool::instance().intern("IRFunctionTypeTest.MovedValues");
    ASSERT_TRUE(Name.valid());
    const std::vector<Parameter> Expected = {Parameter(Pointer, Name), Parameter(Bool, core::NAME_None, true)};
    const FunctionType *Function = nullptr;
    {
      std::vector<Parameter> Parameters = Expected;
      Function = Types.getFunctionType(Bool, std::move(Parameters));
      ASSERT_NE(Function, nullptr);
      Parameters.assign(3, Parameter(Bool));
    }
    expectParametersEqual(Function->parameters(), Expected);
    EXPECT_EQ(Function->parameterCount(), Expected.size());
    EXPECT_EQ(Function->returnType(), Bool);
  }

  // Missing return types and null parameters at any position report failure explicitly.
  TEST(IRFunctionTypeTest, RejectsNullSignatureTypes)
  {
    TypeContext Types(Target64);
    const auto Bool = Types.getBoolType();
    EXPECT_EQ(Types.getFunctionType(nullptr), nullptr);
    EXPECT_EQ(Types.getFunctionType(nullptr, {Parameter(Bool)}), nullptr);
    EXPECT_EQ(Types.getFunctionType(Bool, {Parameter(nullptr)}), nullptr);
    for (std::size_t Index = 0; Index < 3; ++Index)
    {
      std::vector<Parameter> Parameters(3, Parameter(Bool));
      Parameters[Index] = Parameter(nullptr, core::NAME_None, true);
      EXPECT_EQ(Types.getFunctionType(Bool, std::move(Parameters)), nullptr);
    }
  }

  // Function types expose a target-sized address through Type and retain callable identity through Value.
  TEST(IRFunctionTypeTest, CallableLayoutUsesTargetPointerWidth)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      TypeContext Types(Target);
      const auto Aggregate = Types.getArrayType(Types.getInt64Type(), 3);
      ASSERT_NE(Aggregate, nullptr);
      const auto Function = Types.getFunctionType(Aggregate, {Parameter(Aggregate), Parameter(Aggregate)});
      ASSERT_NE(Function, nullptr);
      const Type *TypeValue = Function;
      const Value *ValueValue = Function;
      EXPECT_EQ(TypeValue->kind(), ValueKind::Function);
      EXPECT_EQ(TypeValue->size(), Target.pointerByteWidth());
      EXPECT_EQ(TypeValue->alignment(), Target.pointerByteWidth());
      EXPECT_EQ(ValueValue->kind(), ValueKind::Function);
      EXPECT_TRUE(Type::classof(ValueValue));
      EXPECT_TRUE(FunctionType::classof(ValueValue));
    }
  }

  // Higher-order signatures retain callable return and parameter types as well as pointers to callable types.
  TEST(IRFunctionTypeTest, SupportsNestedCallableSignatures)
  {
    TypeContext Types(Target64);
    const auto Bool = Types.getBoolType();
    const auto Callback = Types.getFunctionType(Bool, {Parameter(Bool)});
    ASSERT_NE(Callback, nullptr);
    const auto CallbackPointer = Types.getPointerType(Callback);
    const auto Factory = Types.getFunctionType(Callback, {Parameter(Callback), Parameter(CallbackPointer)});
    ASSERT_NE(Factory, nullptr);
    EXPECT_EQ(Factory->returnType(), Callback);
    ASSERT_EQ(Factory->parameterCount(), 2U);
    EXPECT_EQ(Factory->parameters()[0].type(), Callback);
    EXPECT_EQ(Factory->parameters()[1].type(), CallbackPointer);
    EXPECT_EQ(CallbackPointer->pointeeType(), Callback);
  }

  // Signature storage preserves multiple variadic markers and their positions for later language-specific validation.
  TEST(IRFunctionTypeTest, PreservesVariadicParameterMetadata)
  {
    TypeContext Types(Target64);
    const auto Bool = Types.getBoolType();
    const core::Name Name = core::NamePool::instance().intern("IRFunctionTypeTest.VariadicValues");
    ASSERT_TRUE(Name.valid());
    const std::vector<Parameter> Expected = {Parameter(Bool, Name, true), Parameter(Bool), Parameter(Bool, core::NAME_None, true)};
    const auto Function = Types.getFunctionType(Bool, Expected);
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function->parameterCount(), Expected.size());
    expectParametersEqual(Function->parameters(), Expected);
  }
} // namespace ink::ir::test
