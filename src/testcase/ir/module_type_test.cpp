#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/array_constant.h"
#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/nominal_type/method.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <unordered_set>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::BigEndian);

    class MethodTypeLifetimeInstruction final : public Instruction
    {
      public:
        MethodTypeLifetimeInstruction(const Module &Types, const Function &Callable, const NominalType &Owner, const FunctionType &ExpectedSignature, const Type &ExpectedReturnType, const ArrayType &Array, bool LateSignature, std::size_t &DestructionCount) noexcept
            : Types(Types),
              Callable(Callable),
              Owner(Owner),
              ExpectedSignature(ExpectedSignature),
              ExpectedReturnType(ExpectedReturnType),
              Array(Array),
              LateSignature(LateSignature),
              DestructionCount(DestructionCount)
        {
        }

        ~MethodTypeLifetimeInstruction() noexcept override
        {
          ++DestructionCount;
          const auto *Signature = Callable.functionType();
          EXPECT_EQ(Signature, &ExpectedSignature);
          EXPECT_TRUE(Types.ownsType(Signature));
          EXPECT_TRUE(Types.ownsType(&Owner));
          EXPECT_EQ(Owner.size(), 16U);
          EXPECT_EQ(Owner.alignment(), 8U);
          EXPECT_TRUE(Owner.isFrozen());
          if (!Types.ownsType(Signature))
          {
            return;
          }
          EXPECT_EQ(Signature->kind(), ValueKind::FunctionType);
          EXPECT_EQ(Signature->size(), 8U);
          EXPECT_EQ(Signature->alignment(), 8U);
          EXPECT_EQ(Signature->returnType(), &ExpectedReturnType);
          EXPECT_TRUE(Types.ownsType(Signature->returnType()));
          EXPECT_EQ(Signature->returnType()->size(), LateSignature ? 16U : 1U);
          EXPECT_EQ(Signature->parameterCount(), LateSignature ? 3U : 2U);
          for (const auto *ParameterType : Signature->parameters())
          {
            EXPECT_TRUE(Types.ownsType(ParameterType));
            EXPECT_TRUE(ParameterType->hasLayout());
            EXPECT_NE(ParameterType->alignment(), 0U);
          }
          EXPECT_FALSE(Signature->parameters().empty());
          if (!Signature->parameters().empty())
          {
            EXPECT_EQ(Signature->parameters().back(), &Array);
          }
          EXPECT_TRUE(Types.ownsType(&Array));
          EXPECT_EQ(Array.elementType(), Types.typePool().getInt32Type());
          EXPECT_EQ(Array.elementCount(), 3U);
          EXPECT_EQ(Array.size(), 12U);
          EXPECT_EQ(Array.alignment(), 4U);
        }

      private:
        const Module &Types;
        const Function &Callable;
        const NominalType &Owner;
        const FunctionType &ExpectedSignature;
        const Type &ExpectedReturnType;
        const ArrayType &Array;
        bool LateSignature;
        std::size_t &DestructionCount;
    };

    static_assert(!std::is_copy_constructible_v<Module>);
    static_assert(!std::is_move_constructible_v<Module>);
    static_assert(!std::is_copy_assignable_v<Module>);
    static_assert(!std::is_move_assignable_v<Module>);
  } // namespace

  // Named and width-based builtin lookups share immutable objects, including signedness distinctions.
  TEST(IRModuleTypeTest, BuiltinLookupsReturnCanonicalTypes)
  {
    const Module Types(Target64);
    EXPECT_EQ(Types.targetContext(), Target64);
    EXPECT_EQ(&Types.typePool().module(), &Types);
    EXPECT_EQ(Types.typePool().getVoidType(), Types.typePool().getVoidType());
    EXPECT_EQ(Types.typePool().getBoolType(), Types.typePool().getBoolType());
    EXPECT_EQ(Types.typePool().getUInt8Type(), Types.typePool().getIntegerType(8, false));
    EXPECT_EQ(Types.typePool().getInt32Type(), Types.typePool().getIntegerType(32, true));
    EXPECT_EQ(Types.typePool().getUInt32Type(), Types.typePool().getIntegerType(32, false));
    EXPECT_EQ(Types.typePool().getInt64Type(), Types.typePool().getIntegerType(64, true));
    EXPECT_EQ(Types.typePool().getUInt64Type(), Types.typePool().getIntegerType(64, false));
    EXPECT_EQ(Types.typePool().getFloat32Type(), Types.typePool().getFloatType(32));
    EXPECT_EQ(Types.typePool().getFloat64Type(), Types.typePool().getFloatType(64));
    EXPECT_EQ(Types.typePool().getNullPointerType(), Types.typePool().getNullPointerType());
    EXPECT_EQ(Types.typePool().getIntegerType(16, true), nullptr);
    EXPECT_EQ(Types.typePool().getFloatType(16), nullptr);
    const std::array<const Type *, 11> Builtins = {
        Types.typePool().getVoidType(),
        Types.typePool().getVariadicType(),
        Types.typePool().getBoolType(),
        Types.typePool().getUInt8Type(),
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

  // Independent modules retain their own target layouts and builtin identities even for equal targets.
  TEST(IRModuleTypeTest, ModulesKeepSeparateTargetLayoutsAndIdentities)
  {
    Module Types32(Target32);
    Module Types64(Target64);
    const Module Other64(Target64);
    const target::TargetContext Native = target::TargetContext::native();
    const Module NativeTypes(Native);
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
    Module Types(Target64);
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
    Module Types(Target64);
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

  // All structural operand positions reject types borrowed from another module.
  TEST(IRModuleTypeTest, RejectsForeignTypeOperands)
  {
    Module Types(Target64);
    const Module Other(Target64);
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

  // Boolean constants use their owning module's canonical type and can populate matching arrays.
  TEST(IRModuleTypeTest, BooleanConstantsUseModuleTypeIdentity)
  {
    Module Types(Target64);
    auto &Constants = Types.constantPool();
    Module Other(Target64);
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

  // Module teardown destroys frozen nominal methods while signatures created before and after their owners remain readable.
  TEST(IRModuleTypeTest, DestroysOwnedMethodsBeforeSignaturesAndTypePool)
  {
    std::array<std::size_t, 6> DestructionCounts{};
    {
      Module Types(Target64);
      const auto *Array = Types.typePool().getArrayType(Types.typePool().getInt32Type(), 3);
      ASSERT_NE(Array, nullptr);
      const auto *EarlySignature = Types.typePool().getFunctionType(Types.typePool().getBoolType(), {Types.typePool().getInt32Type(), Array});
      ASSERT_NE(EarlySignature, nullptr);
      const std::array<NominalType *, 3> Owners = {
          Types.createClassType(16, 8),
          Types.createInterfaceType(16, 8),
          Types.createStructType(16, 8),
      };
      for (const auto *Owner : Owners)
      {
        ASSERT_NE(Owner, nullptr);
      }
      const auto *LateSignature = Types.typePool().getFunctionType(Owners[2], {Owners[0], Owners[1], Array});
      ASSERT_NE(LateSignature, nullptr);
      const std::array<const FunctionType *, 2> Signatures{EarlySignature, LateSignature};
      const core::Name Name = core::NamePool::instance().intern("IRModuleTypeTest.ownedMethod");
      for (std::size_t OwnerIndex = 0; OwnerIndex < Owners.size(); ++OwnerIndex)
      {
        auto *Owner = Owners[OwnerIndex];
        for (std::size_t SignatureIndex = 0; SignatureIndex < Signatures.size(); ++SignatureIndex)
        {
          const auto *Signature = Signatures[SignatureIndex];
          auto Callable = std::make_unique<Function>(*Signature, Name);
          auto *Block = Callable->createBlock();
          ASSERT_NE(Block, nullptr);
          const std::size_t DestructionIndex = OwnerIndex * Signatures.size() + SignatureIndex;
          ASSERT_NE(Block->appendInstruction(std::make_unique<MethodTypeLifetimeInstruction>(Types, *Callable, *Owner, *Signature, *Signature->returnType(), *Array, SignatureIndex != 0, DestructionCounts[DestructionIndex])), nullptr);
          ASSERT_TRUE(Owner->addMethod(Method(std::move(Callable))));
          EXPECT_EQ(DestructionCounts[DestructionIndex], 0U);
        }
        Owner->freeze();
        EXPECT_EQ(Owner->methodCount(), 2U);
      }
    }
    for (const std::size_t Count : DestructionCounts)
    {
      EXPECT_EQ(Count, 1U);
    }
  }
} // namespace ink::ir::test
