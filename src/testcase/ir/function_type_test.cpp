#include "ink/target/target_context.h"
#include "ink/ir/type/structure_type/array_type.h"
#include "ink/ir/type/builtin_type/bool_type.h"
#include "ink/ir/type/builtin_type/float_type.h"
#include "ink/ir/type/structure_type/function_type.h"
#include "ink/ir/type/builtin_type/integer_type.h"
#include "ink/ir/type/structure_type/pointer_type.h"
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

    void expectParametersEqual(const std::vector<const Type *> &Actual, const std::vector<const Type *> &Expected)
    {
      ASSERT_EQ(Actual.size(), Expected.size());
      for (std::size_t Index = 0; Index < Expected.size(); ++Index)
      {
        SCOPED_TRACE(Index);
        EXPECT_EQ(Actual[Index], Expected[Index]);
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
    EXPECT_EQ(Function->kind(), ValueKind::FunctionType);
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
    const std::vector<const Type *> Expected = {Integer, Bool, Float, Integer};
    const auto Function = Types.getFunctionType(Result, Expected);
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function->returnType(), Result);
    EXPECT_EQ(Function->parameterCount(), Expected.size());
    expectParametersEqual(Function->parameters(), Expected);
  }

  // Copying a type list preserves its entries after the source vector is changed and destroyed.
  TEST(IRFunctionTypeTest, SignatureOwnsCopiedParameterList)
  {
    TypeContext Types(Target64);
    const auto Bool = Types.getBoolType();
    const auto Integer = Types.getIntegerType(32, false);
    ASSERT_NE(Integer, nullptr);
    const std::vector<const Type *> Expected = {Bool, Integer, Bool};
    const FunctionType *Function = nullptr;
    {
      std::vector<const Type *> Parameters = Expected;
      Function = Types.getFunctionType(Integer, Parameters);
      ASSERT_NE(Function, nullptr);
      Parameters[0] = Integer;
      Parameters.clear();
      expectParametersEqual(Function->parameters(), Expected);
    }
    expectParametersEqual(Function->parameters(), Expected);
    EXPECT_EQ(Function->parameterCount(), Expected.size());
    EXPECT_EQ(Function->returnType(), Integer);
  }

  // Moving a type list preserves its entries without retaining a dependency on the source container.
  TEST(IRFunctionTypeTest, SignatureOwnsMovedParameterList)
  {
    TypeContext Types(Target64);
    const auto Bool = Types.getBoolType();
    const auto Pointer = Types.getPointerType(Bool);
    ASSERT_NE(Pointer, nullptr);
    const std::vector<const Type *> Expected = {Pointer, Bool};
    const FunctionType *Function = nullptr;
    {
      std::vector<const Type *> Parameters = Expected;
      Function = Types.getFunctionType(Bool, std::move(Parameters));
      ASSERT_NE(Function, nullptr);
      Parameters.assign(3, Bool);
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
    EXPECT_EQ(Types.getFunctionType(nullptr, {Bool}), nullptr);
    EXPECT_EQ(Types.getFunctionType(Bool, {nullptr}), nullptr);
    for (std::size_t Index = 0; Index < 3; ++Index)
    {
      std::vector<const Type *> Parameters(3, Bool);
      Parameters[Index] = nullptr;
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
      const auto Function = Types.getFunctionType(Aggregate, {Aggregate, Aggregate});
      ASSERT_NE(Function, nullptr);
      const Type *TypeValue = Function;
      const Value *ValueValue = Function;
      EXPECT_EQ(TypeValue->kind(), ValueKind::FunctionType);
      EXPECT_EQ(TypeValue->size(), Target.pointerByteWidth());
      EXPECT_EQ(TypeValue->alignment(), Target.pointerByteWidth());
      EXPECT_EQ(ValueValue->kind(), ValueKind::FunctionType);
      EXPECT_TRUE(Type::classof(ValueValue));
      EXPECT_TRUE(FunctionType::classof(ValueValue));
    }
  }

  // Higher-order signatures retain callable return and parameter types as well as pointers to callable types.
  TEST(IRFunctionTypeTest, SupportsNestedCallableSignatures)
  {
    TypeContext Types(Target64);
    const auto Bool = Types.getBoolType();
    const auto Callback = Types.getFunctionType(Bool, {Bool});
    ASSERT_NE(Callback, nullptr);
    const auto CallbackPointer = Types.getPointerType(Callback);
    const auto Factory = Types.getFunctionType(Callback, {Callback, CallbackPointer});
    ASSERT_NE(Factory, nullptr);
    EXPECT_EQ(Factory->returnType(), Callback);
    ASSERT_EQ(Factory->parameterCount(), 2U);
    EXPECT_EQ(Factory->parameters()[0], Callback);
    EXPECT_EQ(Factory->parameters()[1], CallbackPointer);
    EXPECT_EQ(CallbackPointer->pointeeType(), Callback);
  }

  // Function signatures reject foreign return types and a foreign parameter at any position in a type list.
  TEST(IRFunctionTypeTest, RejectsForeignSignatureTypes)
  {
    TypeContext Types(Target64);
    const TypeContext OtherTypes(Target64);
    const auto Bool = Types.getBoolType();
    const auto Foreign = OtherTypes.getBoolType();
    EXPECT_EQ(Types.getFunctionType(Foreign), nullptr);
    EXPECT_EQ(Types.getFunctionType(Foreign, {Bool}), nullptr);
    for (std::size_t Index = 0; Index < 3; ++Index)
    {
      std::vector<const Type *> Parameters(3, Bool);
      Parameters[Index] = Foreign;
      EXPECT_EQ(Types.getFunctionType(Bool, std::move(Parameters)), nullptr);
    }
  }
} // namespace ink::ir::test
