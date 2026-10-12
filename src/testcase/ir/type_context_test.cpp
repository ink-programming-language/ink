#include "ink/core/name_pool.h"
#include "ink/ir/constant/array_constant.h"
#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/nominal_type/method.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/type_context.h"

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
        MethodTypeLifetimeInstruction(const TypeContext &Types, const Function &Callable, const NominalType &Owner, const FunctionType &ExpectedSignature, const Type &ExpectedReturnType, const ArrayType &Array, bool LateSignature, std::size_t &DestructionCount) noexcept
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
          EXPECT_TRUE(Types.owns(Signature));
          EXPECT_TRUE(Types.owns(&Owner));
          EXPECT_EQ(Owner.size(), 16U);
          EXPECT_EQ(Owner.alignment(), 8U);
          EXPECT_TRUE(Owner.isFrozen());
          if (!Types.owns(Signature))
          {
            return;
          }
          EXPECT_EQ(Signature->kind(), ValueKind::FunctionType);
          EXPECT_EQ(Signature->size(), 8U);
          EXPECT_EQ(Signature->alignment(), 8U);
          EXPECT_EQ(Signature->returnType(), &ExpectedReturnType);
          EXPECT_TRUE(Types.owns(Signature->returnType()));
          EXPECT_EQ(Signature->returnType()->size(), LateSignature ? 16U : 1U);
          EXPECT_EQ(Signature->parameterCount(), LateSignature ? 3U : 2U);
          for (const auto *ParameterType : Signature->parameters())
          {
            EXPECT_TRUE(Types.owns(ParameterType));
            EXPECT_TRUE(ParameterType->hasLayout());
            EXPECT_NE(ParameterType->alignment(), 0U);
          }
          EXPECT_FALSE(Signature->parameters().empty());
          if (!Signature->parameters().empty())
          {
            EXPECT_EQ(Signature->parameters().back(), &Array);
          }
          EXPECT_TRUE(Types.owns(&Array));
          EXPECT_EQ(Array.elementType(), Types.getInt32Type());
          EXPECT_EQ(Array.elementCount(), 3U);
          EXPECT_EQ(Array.size(), 12U);
          EXPECT_EQ(Array.alignment(), 4U);
        }

      private:
        const TypeContext &Types;
        const Function &Callable;
        const NominalType &Owner;
        const FunctionType &ExpectedSignature;
        const Type &ExpectedReturnType;
        const ArrayType &Array;
        bool LateSignature;
        std::size_t &DestructionCount;
    };

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
    EXPECT_EQ(Types.getVoidType(), Types.getVoidType());
    EXPECT_EQ(Types.getBoolType(), Types.getBoolType());
    EXPECT_EQ(Types.getUInt8Type(), Types.getIntegerType(8, false));
    EXPECT_EQ(Types.getInt32Type(), Types.getIntegerType(32, true));
    EXPECT_EQ(Types.getUInt32Type(), Types.getIntegerType(32, false));
    EXPECT_EQ(Types.getInt64Type(), Types.getIntegerType(64, true));
    EXPECT_EQ(Types.getUInt64Type(), Types.getIntegerType(64, false));
    EXPECT_EQ(Types.getFloat32Type(), Types.getFloatType(32));
    EXPECT_EQ(Types.getFloat64Type(), Types.getFloatType(64));
    EXPECT_EQ(Types.getNullPointerType(), Types.getNullPointerType());
    EXPECT_EQ(Types.getIntegerType(16, true), nullptr);
    EXPECT_EQ(Types.getFloatType(16), nullptr);
    const std::array<const Type *, 10> Builtins = {
        Types.getVoidType(),
        Types.getBoolType(),
        Types.getUInt8Type(),
        Types.getInt32Type(),
        Types.getUInt32Type(),
        Types.getInt64Type(),
        Types.getUInt64Type(),
        Types.getFloat32Type(),
        Types.getFloat64Type(),
        Types.getNullPointerType(),
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
    EXPECT_EQ(Types32.getUInt8Type()->size(), 1U);
    EXPECT_EQ(Types64.getUInt8Type()->size(), 1U);
    const PointerType *Pointer32 = Types32.getPointerType(Types32.getBoolType());
    const PointerType *Pointer64 = Types64.getPointerType(Types64.getBoolType());
    ASSERT_NE(Pointer32, nullptr);
    ASSERT_NE(Pointer64, nullptr);
    EXPECT_EQ(Pointer32->size(), 4U);
    EXPECT_EQ(Pointer64->size(), 8U);
    EXPECT_NE(Types64.getVoidType(), Other64.getVoidType());
    EXPECT_NE(Types64.getBoolType(), Other64.getBoolType());
    EXPECT_NE(Types64.getUInt8Type(), Other64.getUInt8Type());
    EXPECT_NE(Types64.getInt32Type(), Other64.getInt32Type());
    EXPECT_NE(Types64.getFloat32Type(), Other64.getFloat32Type());
    EXPECT_NE(Types64.getNullPointerType(), Other64.getNullPointerType());
    EXPECT_FALSE(Types64.owns(Other64.getBoolType()));
    EXPECT_FALSE(Types64.owns(Other64.getUInt8Type()));
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

  // Function keys use return types and the order, identities, repetitions and count of parameter types.
  TEST(IRTypeContextTest, FunctionLookupUsesCompletePreservedSignature)
  {
    TypeContext Types(Target64);
    const Type *Bool = Types.getBoolType();
    const Type *Int32 = Types.getInt32Type();
    const FunctionType *Function = Types.getFunctionType(Bool, {Int32, Bool});
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function, Types.getFunctionType(Bool, {Int32, Bool}));
    EXPECT_NE(Function, Types.getFunctionType(Int32, {Int32, Bool}));
    EXPECT_NE(Function, Types.getFunctionType(Bool, {Bool, Int32}));
    EXPECT_NE(Function, Types.getFunctionType(Bool, {Bool, Bool}));
    EXPECT_NE(Function, Types.getFunctionType(Bool, {Int32}));
    EXPECT_NE(Function, Types.getFunctionType(Bool, {Int32, Int32}));
    EXPECT_EQ(Types.getFunctionType(Bool, {Int32, Int32}), Types.getFunctionType(Bool, {Int32, Int32}));
    EXPECT_EQ(Types.getFunctionType(Bool), Types.getFunctionType(Bool, {}));
    EXPECT_TRUE(Types.owns(Function));
  }

  // All structural operand positions reject types borrowed from another context.
  TEST(IRTypeContextTest, RejectsForeignTypeOperands)
  {
    TypeContext Types(Target64);
    const TypeContext Other(Target64);
    const Type *Local = Types.getBoolType();
    const std::array<const Type *, 2> ForeignTypes{Other.getBoolType(), Other.getUInt8Type()};
    for (const Type *Foreign : ForeignTypes)
    {
      EXPECT_EQ(Types.getPointerType(Foreign), nullptr);
      EXPECT_EQ(Types.getReferenceType(Foreign), nullptr);
      EXPECT_EQ(Types.getArrayType(Foreign, 0), nullptr);
      EXPECT_EQ(Types.getArrayType(Foreign, 1), nullptr);
      EXPECT_EQ(Types.getFunctionType(Foreign), nullptr);
      EXPECT_EQ(Types.getFunctionType(Local, {Foreign}), nullptr);
      EXPECT_FALSE(Types.owns(Foreign));
    }
    EXPECT_EQ(Types.getPointerType(nullptr), nullptr);
    EXPECT_EQ(Types.getReferenceType(nullptr), nullptr);
  }

  // Boolean constants use their supplied context's canonical type and can populate matching arrays.
  TEST(IRTypeContextTest, BooleanConstantsUseContextTypeIdentity)
  {
    TypeContext Types(Target64);
    auto &Constants = Types.constantPool();
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

  // Context teardown destroys frozen nominal methods while signatures created before and after their owners remain readable.
  TEST(IRTypeContextTest, DestroysOwnedMethodsBeforeSignaturesAndTypeRegistry)
  {
    std::array<std::size_t, 6> DestructionCounts{};
    {
      TypeContext Types(Target64);
      const auto *Array = Types.getArrayType(Types.getInt32Type(), 3);
      ASSERT_NE(Array, nullptr);
      const auto *EarlySignature = Types.getFunctionType(Types.getBoolType(), {Types.getInt32Type(), Array});
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
      const auto *LateSignature = Types.getFunctionType(Owners[2], {Owners[0], Owners[1], Array});
      ASSERT_NE(LateSignature, nullptr);
      const std::array<const FunctionType *, 2> Signatures{EarlySignature, LateSignature};
      const core::Name Name = core::NamePool::instance().intern("IRTypeContextTest.ownedMethod");
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
