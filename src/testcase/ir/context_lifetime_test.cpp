#include "ink/core/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/nominal_type/field.h"
#include "ink/ir/type/nominal_type/method.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    std::array<NominalType *, 3> createNominalTypes(TypeContext &Types)
    {
      return {Types.createClassType(8, 8), Types.createInterfaceType(8, 8), Types.createStructType(8, 8)};
    }

    class UnownedConstant final : public Constant
    {
      public:
        explicit UnownedConstant(const Type *ConstantType) noexcept
            : Constant(ValueKind::IntegerConstant, ConstantType)
        {
        }
    };

    class InitializerLifetimeInstruction final : public Instruction
    {
      public:
        InitializerLifetimeInstruction(const TypeContext &Types, const NominalType &Owner, const FunctionType &Signature, std::uint64_t ExpectedBits, std::uint32_t &DestructionCount) noexcept
            : Types(Types),
              Owner(Owner),
              Signature(Signature),
              ExpectedBits(ExpectedBits),
              DestructionCount(DestructionCount)
        {
        }

        ~InitializerLifetimeInstruction() noexcept override
        {
          ++DestructionCount;
          EXPECT_TRUE(Types.owns(&Owner));
          EXPECT_TRUE(Types.owns(&Signature));
          EXPECT_EQ(Signature.returnType(), Types.getInt32Type());
          EXPECT_EQ(Signature.parameterCount(), 1U);
          if (Signature.parameterCount() == 1)
          {
            EXPECT_EQ(Signature.parameters()[0], &Owner);
          }
          const Field *Member = Owner.field(0);
          EXPECT_NE(Member, nullptr);
          if (Member == nullptr)
          {
            return;
          }
          EXPECT_TRUE(Member->valid());
          const Constant *Initializer = Member->initializer();
          EXPECT_NE(Initializer, nullptr);
          if (Initializer == nullptr)
          {
            return;
          }
          EXPECT_TRUE(Types.constantPool().owns(Initializer));
          EXPECT_TRUE(Types.owns(Initializer->type()));
          EXPECT_EQ(Initializer->type(), Member->type());
          EXPECT_EQ(Initializer->type(), Types.getInt32Type());
          EXPECT_EQ(Initializer->type()->size(), 4U);
          EXPECT_TRUE(IntegerConstant::classof(Initializer));
          if (IntegerConstant::classof(Initializer))
          {
            EXPECT_EQ(static_cast<const IntegerConstant *>(Initializer)->bits(), ExpectedBits);
          }
        }

      private:
        const TypeContext &Types;
        const NominalType &Owner;
        const FunctionType &Signature;
        std::uint64_t ExpectedBits;
        std::uint32_t &DestructionCount;
    };
  } // namespace

  static_assert(!std::is_constructible_v<ConstantPool, TypeContext &>);
  static_assert(!std::is_constructible_v<ConstantPool, const TypeContext &>);
  static_assert(!std::is_destructible_v<ConstantPool>);
  static_assert(std::is_same_v<decltype(std::declval<TypeContext &>().constantPool()), ConstantPool &>);
  static_assert(std::is_same_v<decltype(std::declval<const TypeContext &>().constantPool()), const ConstantPool &>);
  static_assert(std::is_same_v<decltype(std::declval<const Field &>().initializer()), const Constant *>);
  static_assert(std::is_nothrow_constructible_v<Field, const Type *, core::Name, std::uint64_t, const Constant *>);

  // The single type-owned constant pool and its values survive borrowed accessor scopes and storage growth.
  TEST(IRContextLifetimeTest, TypeContextKeepsOneStableConstantPool)
  {
    TypeContext Types(Target);
    ConstantPool *Pool = nullptr;
    const IntegerConstant *Initializer = nullptr;
    {
      ConstantPool &Constants = Types.constantPool();
      Pool = &Constants;
      Initializer = Constants.getSignedIntegerConstant(Types.getInt32Type(), 42);
      ASSERT_NE(Initializer, nullptr);
      EXPECT_EQ(&Constants.typeContext(), &Types);
    }
    for (std::uint64_t Index = 0; Index < 128; ++Index)
    {
      ASSERT_NE(Types.constantPool().getUnsignedIntegerConstant(Types.getUInt64Type(), Index), nullptr);
      ASSERT_NE(Types.getArrayType(Types.getInt32Type(), Index), nullptr);
    }
    const TypeContext &ReadOnly = Types;
    EXPECT_EQ(&Types.constantPool(), Pool);
    EXPECT_EQ(&ReadOnly.constantPool(), Pool);
    EXPECT_EQ(&ReadOnly.constantPool().typeContext(), &Types);
    EXPECT_TRUE(ReadOnly.constantPool().owns(Initializer));
    EXPECT_EQ(Initializer->type(), Types.getInt32Type());
    EXPECT_EQ(Initializer->bits(), 42U);
    EXPECT_EQ(Types.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 42), Initializer);
    EXPECT_EQ(ReadOnly.constantPool().getBoolConstant(false), Types.constantPool().getBoolConstant(false));
  }

  // All nominal kinds preserve explicit zero initializers separately from absent initializers, including copied metadata.
  TEST(IRContextLifetimeTest, FieldDistinguishesAbsentInitializerFromExplicitZero)
  {
    TypeContext Types(Target);
    const core::Name Name = core::NamePool::instance().intern("IRContextLifetimeTest.member");
    const auto Zero = Types.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 0);
    ASSERT_NE(Zero, nullptr);
    const Field Default(Types.getInt32Type(), Name);
    EXPECT_FALSE(Default.hasInitializer());
    EXPECT_EQ(Default.initializer(), nullptr);
    EXPECT_TRUE(Default.valid());
    const Field Initialized(Types.getInt32Type(), Name, 4, Zero);
    const Field Copied = Initialized;
    EXPECT_TRUE(Copied.hasInitializer());
    EXPECT_EQ(Copied.initializer(), Zero);
    EXPECT_TRUE(Copied.valid());
    for (NominalType *Owner : createNominalTypes(Types))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Default));
      ASSERT_TRUE(Owner->addField(Copied));
      ASSERT_EQ(Owner->fieldCount(), 2U);
      EXPECT_FALSE(Owner->field(0)->hasInitializer());
      EXPECT_EQ(Owner->field(0)->initializer(), nullptr);
      EXPECT_TRUE(Owner->field(1)->hasInitializer());
      EXPECT_EQ(Owner->field(1)->initializer(), Zero);
      EXPECT_EQ(Owner->field(1)->initializer()->type(), Owner->field(1)->type());
      EXPECT_EQ(static_cast<const IntegerConstant *>(Owner->field(1)->initializer())->bits(), 0U);
    }
  }

  // Replacement may set or clear an initializer until freeze, which preserves the previous initializer on rejection.
  TEST(IRContextLifetimeTest, ReplacesClearsAndFreezesFieldInitializers)
  {
    TypeContext Types(Target);
    const core::Name Name = core::NamePool::instance().intern("IRContextLifetimeTest.member");
    const auto First = Types.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 7);
    const auto Second = Types.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 19);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    for (NominalType *Owner : createNominalTypes(Types))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.getInt32Type(), Name, 0, First)));
      ASSERT_TRUE(Owner->replaceField(0, Field(Types.getInt32Type(), Name, 0, Second)));
      EXPECT_EQ(Owner->field(0)->initializer(), Second);
      ASSERT_TRUE(Owner->replaceField(0, Field(Types.getInt32Type(), Name)));
      EXPECT_FALSE(Owner->field(0)->hasInitializer());
      EXPECT_EQ(Owner->field(0)->initializer(), nullptr);
      ASSERT_TRUE(Owner->replaceField(0, Field(Types.getInt32Type(), Name, 0, First)));
      Owner->freeze();
      EXPECT_FALSE(Owner->addField(Field(Types.getInt32Type(), Name, 4, Second)));
      EXPECT_FALSE(Owner->replaceField(0, Field(Types.getInt32Type(), Name, 0, Second)));
      EXPECT_FALSE(Owner->replaceField(0, Field(Types.getInt32Type(), Name)));
      ASSERT_EQ(Owner->fieldCount(), 1U);
      EXPECT_TRUE(Owner->field(0)->hasInitializer());
      EXPECT_EQ(Owner->field(0)->initializer(), First);
    }
  }

  // Mismatched types, foreign pools and same-type unowned constants reject both mutations without changing existing fields.
  TEST(IRContextLifetimeTest, RejectsInvalidInitializersWithoutChangingFields)
  {
    TypeContext Types(Target);
    TypeContext OtherTypes(Target);
    const core::Name Name = core::NamePool::instance().intern("IRContextLifetimeTest.member");
    const auto Initializer = Types.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 11);
    const auto WrongType = Types.constantPool().getBoolConstant(true);
    const auto Foreign = OtherTypes.constantPool().getSignedIntegerConstant(OtherTypes.getInt32Type(), 11);
    ASSERT_NE(Initializer, nullptr);
    ASSERT_NE(WrongType, nullptr);
    ASSERT_NE(Foreign, nullptr);
    const UnownedConstant Unowned(Types.getInt32Type());
    EXPECT_FALSE(Types.constantPool().owns(&Unowned));
    EXPECT_FALSE(Field(Types.getInt32Type(), Name, 0, WrongType).valid());
    EXPECT_FALSE(Field(Types.getInt32Type(), Name, 0, Foreign).valid());
    EXPECT_TRUE(Field(Types.getInt32Type(), Name, 0, &Unowned).valid());
    const Field Rejected[] = {
        Field(Types.getInt32Type(), Name, 0, WrongType),
        Field(Types.getInt32Type(), Name, 0, Foreign),
        Field(OtherTypes.getInt32Type(), Name, 0, Foreign),
        Field(Types.getInt32Type(), Name, 0, &Unowned),
    };
    for (NominalType *Owner : createNominalTypes(Types))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.getInt32Type(), Name, 4, Initializer)));
      for (const Field &Entry : Rejected)
      {
        EXPECT_FALSE(Owner->addField(Entry));
        EXPECT_FALSE(Owner->replaceField(0, Entry));
        ASSERT_EQ(Owner->fieldCount(), 1U);
        EXPECT_EQ(Owner->field(0)->type(), Types.getInt32Type());
        EXPECT_EQ(Owner->field(0)->offset(), 4U);
        EXPECT_EQ(Owner->field(0)->initializer(), Initializer);
      }
    }
  }

  // Every owned method is destroyed while all nominal fields, their constants and referenced signatures remain readable.
  TEST(IRContextLifetimeTest, DestroysAllMethodsBeforeAnyFieldInitializerOrType)
  {
    std::uint32_t DestructionCount = 0;
    {
      TypeContext Types(Target);
      const core::Name FieldName = core::NamePool::instance().intern("IRContextLifetimeTest.member");
      const core::Name MethodName = core::NamePool::instance().intern("IRContextLifetimeTest.method");
      const auto Owners = createNominalTypes(Types);
      for (std::size_t Index = 0; Index < Owners.size(); ++Index)
      {
        ASSERT_NE(Owners[Index], nullptr);
        const auto Initializer = Types.constantPool().getSignedIntegerConstant(Types.getInt32Type(), static_cast<std::int64_t>(Index + 1));
        ASSERT_NE(Initializer, nullptr);
        ASSERT_TRUE(Owners[Index]->addField(Field(Types.getInt32Type(), FieldName, 0, Initializer)));
      }
      for (std::size_t Index = 0; Index < Owners.size(); ++Index)
      {
        for (const std::size_t ReferencedIndex : {Index, (Index + 1) % Owners.size()})
        {
          const auto Signature = Types.getFunctionType(Types.getInt32Type(), {Owners[ReferencedIndex]});
          ASSERT_NE(Signature, nullptr);
          auto Callable = std::make_unique<Function>(*Signature, MethodName);
          auto *Entry = Callable->createBlock();
          ASSERT_NE(Entry, nullptr);
          ASSERT_NE(Entry->appendInstruction(std::make_unique<InitializerLifetimeInstruction>(Types, *Owners[ReferencedIndex], *Signature, ReferencedIndex + 1, DestructionCount)), nullptr);
          ASSERT_TRUE(Owners[Index]->addMethod(Method(std::move(Callable))));
        }
        Owners[Index]->freeze();
      }
      EXPECT_EQ(DestructionCount, 0U);
    }
    EXPECT_EQ(DestructionCount, 6U);
  }
} // namespace ink::ir::test
