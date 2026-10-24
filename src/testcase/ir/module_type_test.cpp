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

  // Module and context template lookups share canonical builtin objects while signedness distinguishes integer types.
  TEST(IRModuleTypeTest, BuiltinLookupsReturnCanonicalTypes)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    EXPECT_EQ(Types.targetContext(), Target64);
    EXPECT_EQ(&Types.typePool().irContext(), &Types.irContext());
    EXPECT_EQ(Types.typePool().get<ink::ir::VoidType>(), Types.typePool().get<ink::ir::VoidType>());
    EXPECT_EQ(Types.typePool().get<ink::ir::BoolType>(), Types.typePool().get<ink::ir::BoolType>());
    EXPECT_EQ(Types.typePool().get<ink::ir::IntegerType>(8, true), TypesContext.typePool().get<ink::ir::IntegerType>(8, true));
    EXPECT_EQ(Types.typePool().get<ink::ir::IntegerType>(8, false), TypesContext.typePool().get<ink::ir::IntegerType>(8, false));
    EXPECT_EQ(Types.typePool().get<ink::ir::IntegerType>(16, true), TypesContext.typePool().get<ink::ir::IntegerType>(16, true));
    EXPECT_EQ(Types.typePool().get<ink::ir::IntegerType>(16, false), TypesContext.typePool().get<ink::ir::IntegerType>(16, false));
    EXPECT_EQ(Types.typePool().get<ink::ir::IntegerType>(32, true), TypesContext.typePool().get<ink::ir::IntegerType>(32, true));
    EXPECT_EQ(Types.typePool().get<ink::ir::IntegerType>(32, false), TypesContext.typePool().get<ink::ir::IntegerType>(32, false));
    EXPECT_EQ(Types.typePool().get<ink::ir::IntegerType>(64, true), TypesContext.typePool().get<ink::ir::IntegerType>(64, true));
    EXPECT_EQ(Types.typePool().get<ink::ir::IntegerType>(64, false), TypesContext.typePool().get<ink::ir::IntegerType>(64, false));
    EXPECT_EQ(Types.typePool().get<ink::ir::FloatType>(32), TypesContext.typePool().get<ink::ir::FloatType>(32));
    EXPECT_EQ(Types.typePool().get<ink::ir::FloatType>(64), TypesContext.typePool().get<ink::ir::FloatType>(64));
    EXPECT_EQ(Types.typePool().get<ink::ir::NullPointerType>(), Types.typePool().get<ink::ir::NullPointerType>());
    EXPECT_NE(TypesContext.typePool().get<ink::ir::IntegerType>(24, true), nullptr);
    EXPECT_EQ(Types.typePool().get<ink::ir::FloatType>(16), nullptr);
    const std::array<const Type *, 14> Builtins = {
        Types.typePool().get<ink::ir::VoidType>(),
        Types.typePool().get<ink::ir::VariadicType>(),
        Types.typePool().get<ink::ir::BoolType>(),
        Types.typePool().get<ink::ir::IntegerType>(8, true),
        Types.typePool().get<ink::ir::IntegerType>(8, false),
        Types.typePool().get<ink::ir::IntegerType>(16, true),
        Types.typePool().get<ink::ir::IntegerType>(16, false),
        Types.typePool().get<ink::ir::IntegerType>(32, true),
        Types.typePool().get<ink::ir::IntegerType>(32, false),
        Types.typePool().get<ink::ir::IntegerType>(64, true),
        Types.typePool().get<ink::ir::IntegerType>(64, false),
        Types.typePool().get<ink::ir::FloatType>(32),
        Types.typePool().get<ink::ir::FloatType>(64),
        Types.typePool().get<ink::ir::NullPointerType>(),
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
    Module Other64(Other64Context);
    const target::TargetContext Native = target::TargetContext::native();
    IRContext NativeTypesContext(Native);
    Module NativeTypes(NativeTypesContext);
    EXPECT_EQ(NativeTypes.targetContext(), Native);
    EXPECT_EQ(NativeTypes.typePool().get<ink::ir::IntegerType>(64, true)->alignment(), Native.integerAlignment(64));
    EXPECT_EQ(NativeTypes.typePool().get<ink::ir::FloatType>(64)->alignment(), Native.floatAlignment(64));
    EXPECT_EQ(Types32.typePool().get<ink::ir::IntegerType>(64, true)->alignment(), 4U);
    EXPECT_EQ(Types64.typePool().get<ink::ir::IntegerType>(64, true)->alignment(), 8U);
    EXPECT_EQ(Types32.typePool().get<ink::ir::IntegerType>(8, false)->size(), 1U);
    EXPECT_EQ(Types64.typePool().get<ink::ir::IntegerType>(8, false)->size(), 1U);
    const PointerType *Pointer32 = Types32.typePool().get<ink::ir::PointerType>(Types32.typePool().get<ink::ir::BoolType>());
    const PointerType *Pointer64 = Types64.typePool().get<ink::ir::PointerType>(Types64.typePool().get<ink::ir::BoolType>());
    ASSERT_NE(Pointer32, nullptr);
    ASSERT_NE(Pointer64, nullptr);
    EXPECT_EQ(Pointer32->size(), 4U);
    EXPECT_EQ(Pointer64->size(), 8U);
    EXPECT_NE(Types64.typePool().get<ink::ir::VoidType>(), Other64.typePool().get<ink::ir::VoidType>());
    EXPECT_NE(Types64.typePool().get<ink::ir::BoolType>(), Other64.typePool().get<ink::ir::BoolType>());
    EXPECT_NE(Types64.typePool().get<ink::ir::IntegerType>(8, false), Other64.typePool().get<ink::ir::IntegerType>(8, false));
    EXPECT_NE(Types64.typePool().get<ink::ir::IntegerType>(32, true), Other64.typePool().get<ink::ir::IntegerType>(32, true));
    EXPECT_NE(Types64.typePool().get<ink::ir::FloatType>(32), Other64.typePool().get<ink::ir::FloatType>(32));
    EXPECT_NE(Types64.typePool().get<ink::ir::NullPointerType>(), Other64.typePool().get<ink::ir::NullPointerType>());
    EXPECT_FALSE(Types64.ownsType(Other64.typePool().get<ink::ir::BoolType>()));
    EXPECT_FALSE(Types64.ownsType(Other64.typePool().get<ink::ir::IntegerType>(8, false)));
  }

  // Repeated structural lookups reuse objects and retain identity when owning storage and caches grow.
  TEST(IRModuleTypeTest, StructuralLookupsReuseStableObjects)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const Type *Element = Types.typePool().get<ink::ir::IntegerType>(32, true);
    const PointerType *Pointer = Types.typePool().get<ink::ir::PointerType>(Element);
    const ReferenceType *Reference = Types.typePool().get<ink::ir::ReferenceType>(Element);
    const ArrayType *Array = Types.typePool().get<ink::ir::ArrayType>(Element, 3);
    const ArrayType *Nested = Types.typePool().get<ink::ir::ArrayType>(Array, 2);
    ASSERT_NE(Pointer, nullptr);
    ASSERT_NE(Reference, nullptr);
    ASSERT_NE(Array, nullptr);
    ASSERT_NE(Nested, nullptr);
    EXPECT_NE(Types.typePool().get<ink::ir::PointerType>(Element), Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, false)));
    EXPECT_NE(Types.typePool().get<ink::ir::ReferenceType>(Element), Types.typePool().get<ink::ir::ReferenceType>(Types.typePool().get<ink::ir::IntegerType>(32, false)));
    EXPECT_NE(Array, Types.typePool().get<ink::ir::ArrayType>(Element, 4));
    EXPECT_NE(Array, Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(32, false), 3));
    for (std::uint64_t Count = 0; Count < 256; ++Count)
    {
      const ArrayType *Added = Types.typePool().get<ink::ir::ArrayType>(Element, Count);
      ASSERT_NE(Added, nullptr);
      ASSERT_NE(Types.typePool().get<ink::ir::PointerType>(Added), nullptr);
      ASSERT_NE(Types.typePool().get<ink::ir::ReferenceType>(Added), nullptr);
    }
    EXPECT_EQ(Pointer, Types.typePool().get<ink::ir::PointerType>(Element));
    EXPECT_EQ(Reference, Types.typePool().get<ink::ir::ReferenceType>(Element));
    EXPECT_EQ(Array, Types.typePool().get<ink::ir::ArrayType>(Element, 3));
    EXPECT_EQ(Nested, Types.typePool().get<ink::ir::ArrayType>(Array, 2));
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
    const Type *Bool = Types.typePool().get<ink::ir::BoolType>();
    const Type *Int32 = Types.typePool().get<ink::ir::IntegerType>(32, true);
    const FunctionType *Function = Types.typePool().get<ink::ir::FunctionType>(Bool, {Int32, Bool});
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function, Types.typePool().get<ink::ir::FunctionType>(Bool, {Int32, Bool}));
    EXPECT_NE(Function, Types.typePool().get<ink::ir::FunctionType>(Int32, {Int32, Bool}));
    EXPECT_NE(Function, Types.typePool().get<ink::ir::FunctionType>(Bool, {Bool, Int32}));
    EXPECT_NE(Function, Types.typePool().get<ink::ir::FunctionType>(Bool, {Bool, Bool}));
    EXPECT_NE(Function, Types.typePool().get<ink::ir::FunctionType>(Bool, {Int32}));
    EXPECT_NE(Function, Types.typePool().get<ink::ir::FunctionType>(Bool, {Int32, Int32}));
    EXPECT_EQ(Types.typePool().get<ink::ir::FunctionType>(Bool, {Int32, Int32}), Types.typePool().get<ink::ir::FunctionType>(Bool, {Int32, Int32}));
    EXPECT_EQ(Types.typePool().get<ink::ir::FunctionType>(Bool), Types.typePool().get<ink::ir::FunctionType>(Bool, {}));
    EXPECT_TRUE(Types.ownsType(Function));
  }

  // All structural operand positions reject types borrowed from another IRContext.
  TEST(IRModuleTypeTest, RejectsForeignTypeOperands)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    IRContext OtherContext(Target64);
    Module Other(OtherContext);
    const Type *Local = Types.typePool().get<ink::ir::BoolType>();
    const std::array<const Type *, 2> ForeignTypes{Other.typePool().get<ink::ir::BoolType>(), Other.typePool().get<ink::ir::IntegerType>(8, false)};
    for (const Type *Foreign : ForeignTypes)
    {
      EXPECT_EQ(Types.typePool().get<ink::ir::PointerType>(Foreign), nullptr);
      EXPECT_EQ(Types.typePool().get<ink::ir::ReferenceType>(Foreign), nullptr);
      EXPECT_EQ(Types.typePool().get<ink::ir::ArrayType>(Foreign, 0), nullptr);
      EXPECT_EQ(Types.typePool().get<ink::ir::ArrayType>(Foreign, 1), nullptr);
      EXPECT_EQ(Types.typePool().get<ink::ir::FunctionType>(Foreign), nullptr);
      EXPECT_EQ(Types.typePool().get<ink::ir::FunctionType>(Local, {Foreign}), nullptr);
      EXPECT_FALSE(Types.ownsType(Foreign));
    }
    EXPECT_EQ(Types.typePool().get<ink::ir::PointerType>(nullptr), nullptr);
    EXPECT_EQ(Types.typePool().get<ink::ir::ReferenceType>(nullptr), nullptr);
  }

  // Boolean constants use their owning context's canonical type and can populate matching arrays.
  TEST(IRModuleTypeTest, BooleanConstantsUseContextTypeIdentity)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    auto &Constants = Types.constantPool();
    IRContext OtherContext(Target64);
    Module Other(OtherContext);
    const auto True = Constants.get<ink::ir::BoolConstant>(true);
    const auto False = Constants.get<ink::ir::BoolConstant>(false);
    ASSERT_NE(True, nullptr);
    ASSERT_NE(False, nullptr);
    EXPECT_EQ(True->type(), Types.typePool().get<ink::ir::BoolType>());
    EXPECT_EQ(False->type(), Types.typePool().get<ink::ir::BoolType>());
    const ArrayType *Array = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::BoolType>(), 2);
    const ArrayType *OtherArray = Other.typePool().get<ink::ir::ArrayType>(Other.typePool().get<ink::ir::BoolType>(), 2);
    EXPECT_NE(Constants.get<ink::ir::ArrayConstant>(Array, {True, False}), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(OtherArray, {True, False}), nullptr);
  }
} // namespace ink::ir::test
