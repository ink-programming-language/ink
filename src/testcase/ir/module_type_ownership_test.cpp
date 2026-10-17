#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <memory>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    class UnpooledInitializer final : public Constant
    {
      public:
        explicit UnpooledInitializer(const Type *ConstantType) noexcept
            : Constant(ValueKind::IntegerConstant, ConstantType)
        {
        }
    };
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
        Pool.getInt8Type(),
        Pool.getInt16Type(),
        Pool.getUInt16Type(),
        Pool.getInt32Type(),
        Pool.getPointerType(Class),
        Pool.getReferenceType(Interface),
        Pool.getArrayType(Struct, 2),
        Pool.getFunctionType(Struct, {Class, Interface}),
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
    const auto *LocalPointer = Pool.getPointerType(Nominal);
    const auto *LocalReference = Pool.getReferenceType(Nominal);
    const auto *LocalArray = Pool.getArrayType(Nominal, 2);
    const auto *LocalSignature = Pool.getFunctionType(Nominal, {Pool.getBoolType()});
    ASSERT_NE(LocalPointer, nullptr);
    ASSERT_NE(LocalReference, nullptr);
    ASSERT_NE(LocalArray, nullptr);
    ASSERT_NE(LocalSignature, nullptr);
    const std::array<const Type *, 6> ForeignTypes = {
        OtherPool.getInt32Type(),
        ForeignNominal,
        OtherPool.getPointerType(ForeignNominal),
        OtherPool.getReferenceType(ForeignNominal),
        OtherPool.getArrayType(ForeignNominal, 2),
        OtherPool.getFunctionType(ForeignNominal),
    };
    for (const auto *Foreign : ForeignTypes)
    {
      ASSERT_NE(Foreign, nullptr);
      EXPECT_FALSE(Owner.contains(Foreign));
      EXPECT_FALSE(Owner.owns(Foreign));
      EXPECT_FALSE(Owner.ownsType(Foreign));
      EXPECT_FALSE(Pool.owns(Foreign));
      EXPECT_EQ(Pool.getPointerType(Foreign), nullptr);
      EXPECT_EQ(Pool.getReferenceType(Foreign), nullptr);
      EXPECT_EQ(Pool.getArrayType(Foreign, 1), nullptr);
      EXPECT_EQ(Pool.getFunctionType(Foreign), nullptr);
      EXPECT_EQ(Pool.getFunctionType(Pool.getVoidType(), {Foreign}), nullptr);
    }
    EXPECT_EQ(LocalPointer, Pool.getPointerType(Nominal));
    EXPECT_EQ(LocalReference, Pool.getReferenceType(Nominal));
    EXPECT_EQ(LocalArray, Pool.getArrayType(Nominal, 2));
    EXPECT_EQ(LocalSignature, Pool.getFunctionType(Nominal, {Pool.getBoolType()}));
    EXPECT_NE(LocalPointer, OtherPool.getPointerType(ForeignNominal));
    EXPECT_NE(LocalReference, OtherPool.getReferenceType(ForeignNominal));
    EXPECT_NE(LocalArray, OtherPool.getArrayType(ForeignNominal, 2));
    EXPECT_NE(LocalSignature, OtherPool.getFunctionType(ForeignNominal, {OtherPool.getBoolType()}));
  }

  // Constants and the canonical null literal remain bound to their defining context even when target layouts match.
  TEST(IRModuleTypeOwnershipTest, ConstantsAndNullRemainContextLocal)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    IRContext OtherContext(Target);
    Module Other(OtherContext);
    auto &Constants = Owner.constantPool();
    const auto *LocalNull = Constants.getNullPointerConstant();
    const auto *ForeignNull = Other.constantPool().getNullPointerConstant();
    EXPECT_EQ(&Constants.irContext(), &Owner.irContext());
    EXPECT_EQ(&Owner.typePool().getNullPointerType()->irContext(), &Owner.irContext());
    EXPECT_EQ(LocalNull->type(), Owner.typePool().getNullPointerType());
    EXPECT_NE(LocalNull, ForeignNull);
    EXPECT_TRUE(Constants.owns(LocalNull));
    EXPECT_FALSE(Constants.owns(ForeignNull));
    EXPECT_FALSE(Owner.ownsType(ForeignNull->type()));
    EXPECT_EQ(Constants.getSignedIntegerConstant(Other.typePool().getInt32Type(), 7), nullptr);
    const auto *LocalArray = Owner.typePool().getArrayType(Owner.typePool().getBoolType(), 1);
    const auto *ForeignArray = Other.typePool().getArrayType(Other.typePool().getBoolType(), 1);
    ASSERT_NE(LocalArray, nullptr);
    ASSERT_NE(ForeignArray, nullptr);
    EXPECT_EQ(Constants.getArrayConstant(LocalArray, {Other.constantPool().getBoolConstant(true)}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(ForeignArray, {Constants.getBoolConstant(true)}), nullptr);
    const auto *Pointer = Owner.typePool().getPointerType(Owner.typePool().getInt32Type());
    ASSERT_NE(Pointer, nullptr);
    auto *Variable = Owner.createGlobalVariable(*Pointer);
    ASSERT_NE(Variable, nullptr);
    EXPECT_TRUE(Variable->setInitializer(LocalNull));
    EXPECT_FALSE(Variable->setInitializer(ForeignNull));
    EXPECT_EQ(Variable->initializer(), LocalNull);
  }

  // Declaration factories reject foreign types and transfer rejects unpooled initializers without growing either list.
  TEST(IRModuleTypeOwnershipTest, GlobalDeclarationsRejectForeignTypesAndUnpooledInitializers)
  {
    IRContext OwnerContext(Target);
    Module Owner(OwnerContext);
    IRContext OtherContext(Target);
    Module Other(OtherContext);
    const auto *ForeignSignature = Other.typePool().getFunctionType(Other.typePool().getVoidType());
    const auto *LocalSignature = Owner.typePool().getFunctionType(Owner.typePool().getVoidType());
    ASSERT_NE(ForeignSignature, nullptr);
    ASSERT_NE(LocalSignature, nullptr);
    auto *LocalFunction = Owner.createGlobalFunction(*LocalSignature);
    auto *LocalVariable = Owner.createGlobalVariable(*Owner.typePool().getInt32Type());
    ASSERT_NE(LocalFunction, nullptr);
    ASSERT_NE(LocalVariable, nullptr);
    EXPECT_EQ(Owner.createGlobalFunction(*ForeignSignature), nullptr);
    EXPECT_EQ(Owner.appendGlobalFunction(std::make_unique<Function>(*ForeignSignature)), nullptr);
    EXPECT_EQ(Owner.createGlobalVariable(*Other.typePool().getInt32Type()), nullptr);
    EXPECT_EQ(Owner.appendGlobalVariable(std::make_unique<GlobalVariable>(*Other.typePool().getInt32Type())), nullptr);
    UnpooledInitializer Unpooled(Owner.typePool().getInt32Type());
    auto InvalidVariable = std::make_unique<GlobalVariable>(*Owner.typePool().getInt32Type());
    ASSERT_TRUE(InvalidVariable->setInitializer(&Unpooled));
    EXPECT_FALSE(Owner.constantPool().owns(&Unpooled));
    EXPECT_EQ(Owner.appendGlobalVariable(std::move(InvalidVariable)), nullptr);
    EXPECT_EQ(Owner.globalFunctionCount(), 1U);
    EXPECT_EQ(Owner.globalVariableCount(), 1U);
    EXPECT_EQ(Owner.globalFunction(0), LocalFunction);
    EXPECT_EQ(Owner.globalVariable(0), LocalVariable);
  }
} // namespace ink::ir::test
