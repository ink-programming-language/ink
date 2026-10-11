#include "ink/core/name_pool.h"
#include "ink/ir/constant/array_constant.h"
#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/constant_context.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <array>
#include <type_traits>
#include <unordered_set>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::BigEndian);

    static_assert(!std::is_copy_constructible_v<TypeContext>);
    static_assert(!std::is_move_constructible_v<TypeContext>);
    static_assert(!std::is_copy_assignable_v<TypeContext>);
    static_assert(!std::is_move_assignable_v<TypeContext>);
  } // namespace

  // Named and width-based builtin lookups share immutable objects, including signedness distinctions.
  TEST(IRTypeContextTest, BuiltinLookupsReturnCanonicalTypes)
  {
    const TypeContext Types(Target64);
    EXPECT_EQ(Types.targetContext(), Target64);
    EXPECT_EQ(Types.getBoolType(), Types.getBoolType());
    EXPECT_EQ(Types.getInt32Type(), Types.getIntegerType(32, true));
    EXPECT_EQ(Types.getUInt32Type(), Types.getIntegerType(32, false));
    EXPECT_EQ(Types.getInt64Type(), Types.getIntegerType(64, true));
    EXPECT_EQ(Types.getUInt64Type(), Types.getIntegerType(64, false));
    EXPECT_EQ(Types.getFloat32Type(), Types.getFloatType(32));
    EXPECT_EQ(Types.getFloat64Type(), Types.getFloatType(64));
    EXPECT_EQ(Types.getStringType(), Types.getStringType());
    EXPECT_EQ(Types.getIntegerType(16, true), nullptr);
    EXPECT_EQ(Types.getFloatType(16), nullptr);
    const std::array<const Type *, 8> Builtins = {
        Types.getBoolType(),
        Types.getInt32Type(),
        Types.getUInt32Type(),
        Types.getInt64Type(),
        Types.getUInt64Type(),
        Types.getFloat32Type(),
        Types.getFloat64Type(),
        Types.getStringType(),
    };
    const std::unordered_set<const Type *> UniqueTypes(Builtins.begin(), Builtins.end());
    EXPECT_EQ(UniqueTypes.size(), Builtins.size());
    for (const Type *Builtin : Builtins)
    {
      EXPECT_TRUE(Types.owns(Builtin));
    }
    EXPECT_FALSE(Types.owns(nullptr));
  }

  // Independent contexts retain their own target layouts and builtin identities even for equal targets.
  TEST(IRTypeContextTest, ContextsKeepSeparateTargetLayoutsAndIdentities)
  {
    TypeContext Types32(Target32);
    TypeContext Types64(Target64);
    const TypeContext Other64(Target64);
    const target::TargetContext Native = target::TargetContext::native();
    const TypeContext NativeTypes(Native);
    EXPECT_EQ(NativeTypes.targetContext(), Native);
    EXPECT_EQ(NativeTypes.getInt64Type()->alignment(), Native.integerAlignment(64));
    EXPECT_EQ(NativeTypes.getFloat64Type()->alignment(), Native.floatAlignment(64));
    EXPECT_EQ(Types32.getInt64Type()->alignment(), 4U);
    EXPECT_EQ(Types64.getInt64Type()->alignment(), 8U);
    EXPECT_EQ(Types32.getStringType()->size(), 8U);
    EXPECT_EQ(Types64.getStringType()->size(), 16U);
    const PointerType *Pointer32 = Types32.getPointerType(Types32.getBoolType());
    const PointerType *Pointer64 = Types64.getPointerType(Types64.getBoolType());
    ASSERT_NE(Pointer32, nullptr);
    ASSERT_NE(Pointer64, nullptr);
    EXPECT_EQ(Pointer32->size(), 4U);
    EXPECT_EQ(Pointer64->size(), 8U);
    EXPECT_NE(Types64.getBoolType(), Other64.getBoolType());
    EXPECT_NE(Types64.getInt32Type(), Other64.getInt32Type());
    EXPECT_NE(Types64.getFloat32Type(), Other64.getFloat32Type());
    EXPECT_NE(Types64.getStringType(), Other64.getStringType());
    EXPECT_FALSE(Types64.owns(Other64.getBoolType()));
  }

  // Repeated structural lookups reuse objects and retain identity when owning storage and caches grow.
  TEST(IRTypeContextTest, StructuralLookupsReuseStableObjects)
  {
    TypeContext Types(Target64);
    const Type *Element = Types.getInt32Type();
    const PointerType *Pointer = Types.getPointerType(Element);
    const ReferenceType *Reference = Types.getReferenceType(Element);
    const ArrayType *Array = Types.getArrayType(Element, 3);
    const ArrayType *Nested = Types.getArrayType(Array, 2);
    ASSERT_NE(Pointer, nullptr);
    ASSERT_NE(Reference, nullptr);
    ASSERT_NE(Array, nullptr);
    ASSERT_NE(Nested, nullptr);
    EXPECT_NE(Types.getPointerType(Element), Types.getPointerType(Types.getUInt32Type()));
    EXPECT_NE(Types.getReferenceType(Element), Types.getReferenceType(Types.getUInt32Type()));
    EXPECT_NE(Array, Types.getArrayType(Element, 4));
    EXPECT_NE(Array, Types.getArrayType(Types.getUInt32Type(), 3));
    for (std::uint64_t Count = 0; Count < 256; ++Count)
    {
      const ArrayType *Added = Types.getArrayType(Element, Count);
      ASSERT_NE(Added, nullptr);
      ASSERT_NE(Types.getPointerType(Added), nullptr);
      ASSERT_NE(Types.getReferenceType(Added), nullptr);
    }
    EXPECT_EQ(Pointer, Types.getPointerType(Element));
    EXPECT_EQ(Reference, Types.getReferenceType(Element));
    EXPECT_EQ(Array, Types.getArrayType(Element, 3));
    EXPECT_EQ(Nested, Types.getArrayType(Array, 2));
    EXPECT_EQ(Nested->elementType(), Array);
    EXPECT_EQ(Array->elementType(), Element);
    EXPECT_TRUE(Types.owns(Pointer));
    EXPECT_TRUE(Types.owns(Reference));
    EXPECT_TRUE(Types.owns(Array));
  }

  // Function keys preserve return types, ordered parameter types, names and variadic metadata.
  TEST(IRTypeContextTest, FunctionLookupUsesCompletePreservedSignature)
  {
    TypeContext Types(Target64);
    const Type *Bool = Types.getBoolType();
    const Type *Int32 = Types.getInt32Type();
    const core::Name FirstName = core::NamePool::instance().intern("IRTypeContextTest.First");
    const core::Name SecondName = core::NamePool::instance().intern("IRTypeContextTest.Second");
    const FunctionType *Function = Types.getFunctionType(Bool, {Parameter(Int32, FirstName), Parameter(Bool)});
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function, Types.getFunctionType(Bool, {Parameter(Int32, FirstName), Parameter(Bool)}));
    EXPECT_NE(Function, Types.getFunctionType(Int32, {Parameter(Int32, FirstName), Parameter(Bool)}));
    EXPECT_NE(Function, Types.getFunctionType(Bool, {Parameter(Int32, SecondName), Parameter(Bool)}));
    EXPECT_NE(Function, Types.getFunctionType(Bool, {Parameter(Int32, FirstName, true), Parameter(Bool)}));
    EXPECT_NE(Function, Types.getFunctionType(Bool, {Parameter(Bool), Parameter(Int32, FirstName)}));
    EXPECT_NE(Function, Types.getFunctionType(Bool, {Parameter(Bool, FirstName), Parameter(Bool)}));
    EXPECT_NE(Function, Types.getFunctionType(Bool, {Parameter(Int32, FirstName)}));
    EXPECT_EQ(Types.getFunctionType(Bool), Types.getFunctionType(Bool, {}));
    EXPECT_EQ(Function->parameters()[0].name(), FirstName);
    EXPECT_TRUE(Types.owns(Function));
  }

  // All structural operand positions reject types borrowed from another context.
  TEST(IRTypeContextTest, RejectsForeignTypeOperands)
  {
    TypeContext Types(Target64);
    const TypeContext Other(Target64);
    const Type *Local = Types.getBoolType();
    const Type *Foreign = Other.getBoolType();
    EXPECT_EQ(Types.getPointerType(Foreign), nullptr);
    EXPECT_EQ(Types.getReferenceType(Foreign), nullptr);
    EXPECT_EQ(Types.getArrayType(Foreign, 0), nullptr);
    EXPECT_EQ(Types.getArrayType(Foreign, 1), nullptr);
    EXPECT_EQ(Types.getFunctionType(Foreign), nullptr);
    EXPECT_EQ(Types.getFunctionType(Local, {Parameter(Foreign)}), nullptr);
    EXPECT_EQ(Types.getPointerType(nullptr), nullptr);
    EXPECT_EQ(Types.getReferenceType(nullptr), nullptr);
    EXPECT_FALSE(Types.owns(Foreign));
  }

  // Boolean constants use their supplied context's canonical type and can populate matching arrays.
  TEST(IRTypeContextTest, BooleanConstantsUseContextTypeIdentity)
  {
    TypeContext Types(Target64);
    ConstantContext Constants(Types);
    TypeContext Other(Target64);
    const auto True = Constants.getBoolConstant(true);
    const auto False = Constants.getBoolConstant(false);
    ASSERT_NE(True, nullptr);
    ASSERT_NE(False, nullptr);
    EXPECT_EQ(True->type(), Types.getBoolType());
    EXPECT_EQ(False->type(), Types.getBoolType());
    const ArrayType *Array = Types.getArrayType(Types.getBoolType(), 2);
    const ArrayType *OtherArray = Other.getArrayType(Other.getBoolType(), 2);
    EXPECT_NE(Constants.getArrayConstant(Array, {True, False}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(OtherArray, {True, False}), nullptr);
  }
} // namespace ink::ir::test
