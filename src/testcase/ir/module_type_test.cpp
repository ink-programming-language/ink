#include "ink/ir/constant/array_constant.h"
#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"

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

    static_assert(!std::is_copy_constructible_v<Module>);
    static_assert(!std::is_move_constructible_v<Module>);
    static_assert(!std::is_copy_assignable_v<Module>);
    static_assert(!std::is_move_assignable_v<Module>);
  } // namespace

  // Named and width-based builtin lookups share immutable objects, including signedness distinctions.
  TEST(IRModuleTypeTest, BuiltinLookupsReturnCanonicalTypes)
  {
    IRContext TypesContext(Target64);
    const Module Types(TypesContext);
    EXPECT_EQ(Types.targetContext(), Target64);
    EXPECT_EQ(&Types.typePool().irContext(), &Types.irContext());
    EXPECT_EQ(Types.typePool().getVoidType(), Types.typePool().getVoidType());
    EXPECT_EQ(Types.typePool().getBoolType(), Types.typePool().getBoolType());
    EXPECT_EQ(Types.typePool().getInt8Type(), Types.typePool().getIntegerType(8, true));
    EXPECT_EQ(Types.typePool().getUInt8Type(), Types.typePool().getIntegerType(8, false));
    EXPECT_EQ(Types.typePool().getInt16Type(), Types.typePool().getIntegerType(16, true));
    EXPECT_EQ(Types.typePool().getUInt16Type(), Types.typePool().getIntegerType(16, false));
    EXPECT_EQ(Types.typePool().getInt32Type(), Types.typePool().getIntegerType(32, true));
    EXPECT_EQ(Types.typePool().getUInt32Type(), Types.typePool().getIntegerType(32, false));
    EXPECT_EQ(Types.typePool().getInt64Type(), Types.typePool().getIntegerType(64, true));
    EXPECT_EQ(Types.typePool().getUInt64Type(), Types.typePool().getIntegerType(64, false));
    EXPECT_EQ(Types.typePool().getFloat32Type(), Types.typePool().getFloatType(32));
    EXPECT_EQ(Types.typePool().getFloat64Type(), Types.typePool().getFloatType(64));
    EXPECT_EQ(Types.typePool().getNullPointerType(), Types.typePool().getNullPointerType());
    EXPECT_EQ(Types.typePool().getIntegerType(24, true), nullptr);
    EXPECT_EQ(Types.typePool().getFloatType(16), nullptr);
    const std::array<const Type *, 14> Builtins = {
        Types.typePool().getVoidType(),
        Types.typePool().getVariadicType(),
        Types.typePool().getBoolType(),
        Types.typePool().getInt8Type(),
        Types.typePool().getUInt8Type(),
        Types.typePool().getInt16Type(),
        Types.typePool().getUInt16Type(),
        Types.typePool().getInt32Type(),
        Types.typePool().getUInt32Type(),
        Types.typePool().getInt64Type(),
        Types.typePool().getUInt64Type(),
        Types.typePool().getFloat32Type(),
        Types.typePool().getFloat64Type(),
        Types.typePool().getNullPointerType(),
    };
    const std::unordered_set<const Type *> UniqueTypes(Builtins.begin(), Builtins.end());
    EXPECT_EQ(UniqueTypes.size(), Builtins.size());
    for (const Type *Builtin : Builtins)
    {
      EXPECT_TRUE(Types.ownsType(Builtin));
      EXPECT_TRUE(Types.typePool().owns(Builtin));
    }
    EXPECT_FALSE(Types.ownsType(nullptr));
    EXPECT_FALSE(Types.typePool().owns(nullptr));
  }

  // Modules in independent contexts retain separate target layouts and builtin identities even for equal targets.
  TEST(IRModuleTypeTest, IndependentContextsKeepSeparateTargetLayoutsAndIdentities)
  {
    IRContext Types32Context(Target32);
    Module Types32(Types32Context);
    IRContext Types64Context(Target64);
    Module Types64(Types64Context);
    IRContext Other64Context(Target64);
    const Module Other64(Other64Context);
    const target::TargetContext Native = target::TargetContext::native();
    IRContext NativeTypesContext(Native);
    const Module NativeTypes(NativeTypesContext);
    EXPECT_EQ(NativeTypes.targetContext(), Native);
    EXPECT_EQ(NativeTypes.typePool().getInt64Type()->alignment(), Native.integerAlignment(64));
    EXPECT_EQ(NativeTypes.typePool().getFloat64Type()->alignment(), Native.floatAlignment(64));
    EXPECT_EQ(Types32.typePool().getInt64Type()->alignment(), 4U);
    EXPECT_EQ(Types64.typePool().getInt64Type()->alignment(), 8U);
    EXPECT_EQ(Types32.typePool().getUInt8Type()->size(), 1U);
    EXPECT_EQ(Types64.typePool().getUInt8Type()->size(), 1U);
    const PointerType *Pointer32 = Types32.typePool().getPointerType(Types32.typePool().getBoolType());
    const PointerType *Pointer64 = Types64.typePool().getPointerType(Types64.typePool().getBoolType());
    ASSERT_NE(Pointer32, nullptr);
    ASSERT_NE(Pointer64, nullptr);
    EXPECT_EQ(Pointer32->size(), 4U);
    EXPECT_EQ(Pointer64->size(), 8U);
    EXPECT_NE(Types64.typePool().getVoidType(), Other64.typePool().getVoidType());
    EXPECT_NE(Types64.typePool().getBoolType(), Other64.typePool().getBoolType());
    EXPECT_NE(Types64.typePool().getUInt8Type(), Other64.typePool().getUInt8Type());
    EXPECT_NE(Types64.typePool().getInt32Type(), Other64.typePool().getInt32Type());
    EXPECT_NE(Types64.typePool().getFloat32Type(), Other64.typePool().getFloat32Type());
    EXPECT_NE(Types64.typePool().getNullPointerType(), Other64.typePool().getNullPointerType());
    EXPECT_FALSE(Types64.ownsType(Other64.typePool().getBoolType()));
    EXPECT_FALSE(Types64.ownsType(Other64.typePool().getUInt8Type()));
  }

  // Repeated structural lookups reuse objects and retain identity when owning storage and caches grow.
  TEST(IRModuleTypeTest, StructuralLookupsReuseStableObjects)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const Type *Element = Types.typePool().getInt32Type();
    const PointerType *Pointer = Types.typePool().getPointerType(Element);
    const ReferenceType *Reference = Types.typePool().getReferenceType(Element);
    const ArrayType *Array = Types.typePool().getArrayType(Element, 3);
    const ArrayType *Nested = Types.typePool().getArrayType(Array, 2);
    ASSERT_NE(Pointer, nullptr);
    ASSERT_NE(Reference, nullptr);
    ASSERT_NE(Array, nullptr);
    ASSERT_NE(Nested, nullptr);
    EXPECT_NE(Types.typePool().getPointerType(Element), Types.typePool().getPointerType(Types.typePool().getUInt32Type()));
    EXPECT_NE(Types.typePool().getReferenceType(Element), Types.typePool().getReferenceType(Types.typePool().getUInt32Type()));
    EXPECT_NE(Array, Types.typePool().getArrayType(Element, 4));
    EXPECT_NE(Array, Types.typePool().getArrayType(Types.typePool().getUInt32Type(), 3));
    for (std::uint64_t Count = 0; Count < 256; ++Count)
    {
      const ArrayType *Added = Types.typePool().getArrayType(Element, Count);
      ASSERT_NE(Added, nullptr);
      ASSERT_NE(Types.typePool().getPointerType(Added), nullptr);
      ASSERT_NE(Types.typePool().getReferenceType(Added), nullptr);
    }
    EXPECT_EQ(Pointer, Types.typePool().getPointerType(Element));
    EXPECT_EQ(Reference, Types.typePool().getReferenceType(Element));
    EXPECT_EQ(Array, Types.typePool().getArrayType(Element, 3));
    EXPECT_EQ(Nested, Types.typePool().getArrayType(Array, 2));
    EXPECT_EQ(Nested->elementType(), Array);
    EXPECT_EQ(Array->elementType(), Element);
    EXPECT_TRUE(Types.ownsType(Pointer));
    EXPECT_TRUE(Types.ownsType(Reference));
    EXPECT_TRUE(Types.ownsType(Array));
  }

  // Function keys use return types and the order, identities, repetitions and count of parameter types.
  TEST(IRModuleTypeTest, FunctionLookupUsesCompletePreservedSignature)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const Type *Bool = Types.typePool().getBoolType();
    const Type *Int32 = Types.typePool().getInt32Type();
    const FunctionType *Function = Types.typePool().getFunctionType(Bool, {Int32, Bool});
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function, Types.typePool().getFunctionType(Bool, {Int32, Bool}));
    EXPECT_NE(Function, Types.typePool().getFunctionType(Int32, {Int32, Bool}));
    EXPECT_NE(Function, Types.typePool().getFunctionType(Bool, {Bool, Int32}));
    EXPECT_NE(Function, Types.typePool().getFunctionType(Bool, {Bool, Bool}));
    EXPECT_NE(Function, Types.typePool().getFunctionType(Bool, {Int32}));
    EXPECT_NE(Function, Types.typePool().getFunctionType(Bool, {Int32, Int32}));
    EXPECT_EQ(Types.typePool().getFunctionType(Bool, {Int32, Int32}), Types.typePool().getFunctionType(Bool, {Int32, Int32}));
    EXPECT_EQ(Types.typePool().getFunctionType(Bool), Types.typePool().getFunctionType(Bool, {}));
    EXPECT_TRUE(Types.ownsType(Function));
  }

  // All structural operand positions reject types borrowed from another IRContext.
  TEST(IRModuleTypeTest, RejectsForeignTypeOperands)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    IRContext OtherContext(Target64);
    const Module Other(OtherContext);
    const Type *Local = Types.typePool().getBoolType();
    const std::array<const Type *, 2> ForeignTypes{Other.typePool().getBoolType(), Other.typePool().getUInt8Type()};
    for (const Type *Foreign : ForeignTypes)
    {
      EXPECT_EQ(Types.typePool().getPointerType(Foreign), nullptr);
      EXPECT_EQ(Types.typePool().getReferenceType(Foreign), nullptr);
      EXPECT_EQ(Types.typePool().getArrayType(Foreign, 0), nullptr);
      EXPECT_EQ(Types.typePool().getArrayType(Foreign, 1), nullptr);
      EXPECT_EQ(Types.typePool().getFunctionType(Foreign), nullptr);
      EXPECT_EQ(Types.typePool().getFunctionType(Local, {Foreign}), nullptr);
      EXPECT_FALSE(Types.ownsType(Foreign));
    }
    EXPECT_EQ(Types.typePool().getPointerType(nullptr), nullptr);
    EXPECT_EQ(Types.typePool().getReferenceType(nullptr), nullptr);
  }

  // Boolean constants use their owning context's canonical type and can populate matching arrays.
  TEST(IRModuleTypeTest, BooleanConstantsUseContextTypeIdentity)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    IRContext OtherContext(Target64);
    Module Other(OtherContext);
    const auto True = Constants.getBoolConstant(true);
    const auto False = Constants.getBoolConstant(false);
    ASSERT_NE(True, nullptr);
    ASSERT_NE(False, nullptr);
    EXPECT_EQ(True->type(), Types.typePool().getBoolType());
    EXPECT_EQ(False->type(), Types.typePool().getBoolType());
    const ArrayType *Array = Types.typePool().getArrayType(Types.typePool().getBoolType(), 2);
    const ArrayType *OtherArray = Other.typePool().getArrayType(Other.typePool().getBoolType(), 2);
    EXPECT_NE(Constants.getArrayConstant(Array, {True, False}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(OtherArray, {True, False}), nullptr);
  }
} // namespace ink::ir::test
