#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <array>
#include <memory>
#include <type_traits>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  static_assert(!std::is_constructible_v<TypePool, IRContext &>);
  static_assert(!std::is_default_constructible_v<TypePool>);
  static_assert(!std::is_destructible_v<TypePool>);
  static_assert(!std::is_copy_constructible_v<TypePool>);
  static_assert(!std::is_move_constructible_v<TypePool>);
  static_assert(!std::is_copy_assignable_v<TypePool>);
  static_assert(!std::is_move_assignable_v<TypePool>);

  // Module lists own nominal declarations directly while TypePool owns only builtin and structural identities.
  TEST(IRModuleTypeOwnershipTest, TypePoolExcludesModuleOwnedNominalTypes)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    IRContext OtherContext(Target);
    Module Other(OtherContext);
    auto &Pool = Owner.typePool();
    const auto *Class = Owner.createClassType(8, 8);
    const auto *Interface = Owner.createInterfaceType(8, 8);
    const auto *Struct = Owner.createStructType(8, 8);
    const auto *Foreign = Other.createStructType(8, 8);
    const std::array<const Type *, 3> Nominals{Class, Interface, Struct};
    EXPECT_EQ(&Pool.irContext(), &Owner.irContext());
    ASSERT_EQ(Owner.classCount(), 1U);
    ASSERT_EQ(Owner.interfaceCount(), 1U);
    ASSERT_EQ(Owner.structCount(), 1U);
    EXPECT_EQ(Owner.classes()[0].get(), Class);
    EXPECT_EQ(Owner.interfaces()[0].get(), Interface);
    EXPECT_EQ(Owner.structs()[0].get(), Struct);
    for (const auto *Nominal : Nominals)
    {
      ASSERT_NE(Nominal, nullptr);
      EXPECT_FALSE(Pool.owns(Nominal));
      EXPECT_TRUE(Owner.ownsType(Nominal));
      EXPECT_TRUE(Owner.owns(Nominal));
      EXPECT_TRUE(Owner.contains(Nominal));
      EXPECT_FALSE(Other.ownsType(Nominal));
      EXPECT_FALSE(Other.typePool().owns(Nominal));
    }
    const std::array<const Type *, 8> Pooled = {
        Pool.get<ink::ir::IntegerType>(8, true),
        Pool.get<ink::ir::IntegerType>(16, true),
        Pool.get<ink::ir::IntegerType>(16, false),
        Pool.get<ink::ir::IntegerType>(32, true),
        Pool.get<ink::ir::PointerType>(Class),
        Pool.get<ink::ir::ReferenceType>(Interface),
        Pool.get<ink::ir::ArrayType>(Struct, 2),
        Pool.get<ink::ir::FunctionType>(Struct, {Class, Interface}),
    };
    for (const auto *PooledType : Pooled)
    {
      ASSERT_NE(PooledType, nullptr);
      EXPECT_TRUE(Pool.owns(PooledType));
      EXPECT_TRUE(Owner.ownsType(PooledType));
      EXPECT_TRUE(Owner.owns(PooledType));
      EXPECT_TRUE(Owner.contains(PooledType));
      EXPECT_FALSE(Other.typePool().owns(PooledType));
    }
    ASSERT_NE(Foreign, nullptr);
    EXPECT_FALSE(Pool.owns(Foreign));
    EXPECT_FALSE(Pool.owns(nullptr));
    EXPECT_FALSE(Owner.ownsType(Foreign));
    EXPECT_FALSE(Owner.ownsType(nullptr));
  }

  // Structural caches accept local nominal operands, reject every foreign type category and remain context-local.
  TEST(IRModuleTypeOwnershipTest, StructuralCachesRejectForeignNominalAndPooledTypes)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    IRContext OtherContext(Target);
    Module Other(OtherContext);
    auto &Pool = Owner.typePool();
    auto &OtherPool = Other.typePool();
    const auto *Nominal = Owner.createStructType(8, 8);
    const auto *ForeignNominal = Other.createStructType(8, 8);
    ASSERT_NE(Nominal, nullptr);
    ASSERT_NE(ForeignNominal, nullptr);
    const auto *LocalPointer = Pool.get<ink::ir::PointerType>(Nominal);
    const auto *LocalReference = Pool.get<ink::ir::ReferenceType>(Nominal);
    const auto *LocalArray = Pool.get<ink::ir::ArrayType>(Nominal, 2);
    const auto *LocalSignature = Pool.get<ink::ir::FunctionType>(Nominal, {Pool.get<ink::ir::BoolType>()});
    ASSERT_NE(LocalPointer, nullptr);
    ASSERT_NE(LocalReference, nullptr);
    ASSERT_NE(LocalArray, nullptr);
    ASSERT_NE(LocalSignature, nullptr);
    const std::array<const Type *, 6> ForeignTypes = {
        OtherPool.get<ink::ir::IntegerType>(32, true),
        ForeignNominal,
        OtherPool.get<ink::ir::PointerType>(ForeignNominal),
        OtherPool.get<ink::ir::ReferenceType>(ForeignNominal),
        OtherPool.get<ink::ir::ArrayType>(ForeignNominal, 2),
        OtherPool.get<ink::ir::FunctionType>(ForeignNominal),
    };
    for (const auto *Foreign : ForeignTypes)
    {
      ASSERT_NE(Foreign, nullptr);
      EXPECT_FALSE(Owner.contains(Foreign));
      EXPECT_FALSE(Owner.owns(Foreign));
      EXPECT_FALSE(Owner.ownsType(Foreign));
      EXPECT_FALSE(Pool.owns(Foreign));
      EXPECT_EQ(Pool.get<ink::ir::PointerType>(Foreign), nullptr);
      EXPECT_EQ(Pool.get<ink::ir::ReferenceType>(Foreign), nullptr);
      EXPECT_EQ(Pool.get<ink::ir::ArrayType>(Foreign, 1), nullptr);
      EXPECT_EQ(Pool.get<ink::ir::FunctionType>(Foreign), nullptr);
      EXPECT_EQ(Pool.get<ink::ir::FunctionType>(Pool.get<ink::ir::VoidType>(), {Foreign}), nullptr);
    }
    EXPECT_EQ(LocalPointer, Pool.get<ink::ir::PointerType>(Nominal));
    EXPECT_EQ(LocalReference, Pool.get<ink::ir::ReferenceType>(Nominal));
    EXPECT_EQ(LocalArray, Pool.get<ink::ir::ArrayType>(Nominal, 2));
    EXPECT_EQ(LocalSignature, Pool.get<ink::ir::FunctionType>(Nominal, {Pool.get<ink::ir::BoolType>()}));
    EXPECT_NE(LocalPointer, OtherPool.get<ink::ir::PointerType>(ForeignNominal));
    EXPECT_NE(LocalReference, OtherPool.get<ink::ir::ReferenceType>(ForeignNominal));
    EXPECT_NE(LocalArray, OtherPool.get<ink::ir::ArrayType>(ForeignNominal, 2));
    EXPECT_NE(LocalSignature, OtherPool.get<ink::ir::FunctionType>(ForeignNominal, {OtherPool.get<ink::ir::BoolType>()}));
  }

  // Constants and the canonical null literal remain bound to their defining context even when target layouts match.
  TEST(IRModuleTypeOwnershipTest, ConstantsAndNullRemainContextLocal)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    IRContext OtherContext(Target);
    Module Other(OtherContext);
    auto &Constants = Owner.constantPool();
    const auto *LocalNull = Constants.get<ink::ir::NullPointerConstant>();
    const auto *ForeignNull = Other.constantPool().get<ink::ir::NullPointerConstant>();
    EXPECT_EQ(&Constants.irContext(), &Owner.irContext());
    EXPECT_EQ(&Owner.typePool().get<ink::ir::NullPointerType>()->irContext(), &Owner.irContext());
    EXPECT_EQ(LocalNull->type(), Owner.typePool().get<ink::ir::NullPointerType>());
    EXPECT_NE(LocalNull, ForeignNull);
    EXPECT_TRUE(Constants.owns(LocalNull));
    EXPECT_FALSE(Constants.owns(ForeignNull));
    EXPECT_FALSE(Owner.ownsType(ForeignNull->type()));
    EXPECT_EQ(Constants.get<ink::ir::IntegerConstant>(Other.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(7)), nullptr);
    const auto *LocalArray = Owner.typePool().get<ink::ir::ArrayType>(Owner.typePool().get<ink::ir::BoolType>(), 1);
    const auto *ForeignArray = Other.typePool().get<ink::ir::ArrayType>(Other.typePool().get<ink::ir::BoolType>(), 1);
    ASSERT_NE(LocalArray, nullptr);
    ASSERT_NE(ForeignArray, nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(LocalArray, {Other.constantPool().get<ink::ir::BoolConstant>(true)}), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(ForeignArray, {Constants.get<ink::ir::BoolConstant>(true)}), nullptr);
    const auto *Pointer = Owner.typePool().get<ink::ir::PointerType>(Owner.typePool().get<ink::ir::IntegerType>(32, true));
    ASSERT_NE(Pointer, nullptr);
    auto *Variable = Owner.createGlobalVariable(*Pointer);
    ASSERT_NE(Variable, nullptr);
    EXPECT_TRUE(Variable->setInitializer(LocalNull));
    EXPECT_FALSE(Variable->setInitializer(ForeignNull));
    EXPECT_EQ(Variable->initializer(), LocalNull);
  }
  // A pooled address type cannot conceal a global object's nominal declaration owned by another module in the same context.
  TEST(IRModuleTypeOwnershipTest, ChecksGlobalObjectOwnershipBehindAddressType)
  {
    IRContext Context(Target);
    Module Owner(Context);
    Module Other(Context);
    auto *LocalObject = Owner.createStructType(8, 8);
    auto *OtherObject = Other.createStructType(8, 8);
    ASSERT_NE(LocalObject, nullptr);
    ASSERT_NE(OtherObject, nullptr);
    const auto *LocalAddress = Context.typePool().get<ink::ir::PointerType>(LocalObject);
    const auto *OtherAddress = Context.typePool().get<ink::ir::PointerType>(OtherObject);
    ASSERT_NE(LocalAddress, nullptr);
    ASSERT_NE(OtherAddress, nullptr);
    EXPECT_TRUE(Owner.ownsType(OtherAddress));
    EXPECT_FALSE(Owner.ownsType(OtherObject));
    EXPECT_EQ(Owner.createGlobalVariable(*OtherObject), nullptr);
    EXPECT_EQ(Owner.appendGlobalVariable(std::make_unique<GlobalVariable>(*OtherAddress)), nullptr);
    EXPECT_EQ(Owner.globalVariableCount(), 0U);
    auto *Local = Owner.appendGlobalVariable(std::make_unique<GlobalVariable>(*LocalAddress));
    ASSERT_NE(Local, nullptr);
    EXPECT_EQ(Local->type(), LocalAddress);
    EXPECT_EQ(Local->variableType(), LocalObject);
    EXPECT_EQ(Owner.globalVariableCount(), 1U);
  }
} // namespace ink::ir::test
