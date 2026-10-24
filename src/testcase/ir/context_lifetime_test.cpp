#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/type/nominal_type/field.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    std::array<NominalType *, 3> createNominalTypes(Module &Types)
    {
      return {Types.createClassType(8, 8), Types.createInterfaceType(8, 8), Types.createStructType(8, 8)};
    }
  } // namespace

  static_assert(!std::is_constructible_v<ConstantPool, IRContext &>);
  static_assert(!std::is_constructible_v<ConstantPool, const IRContext &>);
  static_assert(!std::is_destructible_v<ConstantPool>);
  static_assert(std::is_same_v<decltype(std::declval<const Field &>().initializer()), const Constant *>);
  static_assert(std::is_nothrow_constructible_v<Field, const Type *, core::Name, std::uint64_t, const Constant *>);

  // The single module-owned constant pool and its values survive borrowed accessor scopes and storage growth.
  TEST(IRContextLifetimeTest, ModuleKeepsOneStableConstantPool)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    ConstantPool *Pool = nullptr;
    const IntegerConstant *Initializer = nullptr;
    {
      ConstantPool &Constants = Types.constantPool();
      Pool = &Constants;
      Initializer = Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(42));
      ASSERT_NE(Initializer, nullptr);
      EXPECT_EQ(&Constants.irContext(), &Types.irContext());
    }
    for (std::uint64_t Index = 0; Index < 128; ++Index)
    {
      ASSERT_NE(Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(64, false), static_cast<std::uint64_t>(Index)), nullptr);
      ASSERT_NE(Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(32, true), Index), nullptr);
    }
    const Module &ReadOnly = Types;
    EXPECT_EQ(&Types.constantPool(), Pool);
    EXPECT_EQ(&ReadOnly.constantPool(), Pool);
    EXPECT_EQ(&ReadOnly.constantPool().irContext(), &Types.irContext());
    EXPECT_TRUE(ReadOnly.constantPool().owns(Initializer));
    EXPECT_EQ(Initializer->type(), Types.typePool().get<ink::ir::IntegerType>(32, true));
    EXPECT_EQ(Initializer->bits().tryGetUInt64(), 42U);
    EXPECT_EQ(Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(42)), Initializer);
    EXPECT_EQ(ReadOnly.constantPool().get<ink::ir::BoolConstant>(false), Types.constantPool().get<ink::ir::BoolConstant>(false));
  }

  // All nominal kinds preserve explicit zero initializers separately from absent initializers, including copied metadata.
  TEST(IRContextLifetimeTest, FieldDistinguishesAbsentInitializerFromExplicitZero)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const core::Name Name = core::NamePool::instance().intern("IRContextLifetimeTest.member");
    const auto Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
    ASSERT_NE(Zero, nullptr);
    const Field Default(Types.typePool().get<ink::ir::IntegerType>(32, true), Name);
    EXPECT_FALSE(Default.hasInitializer());
    EXPECT_EQ(Default.initializer(), nullptr);
    EXPECT_TRUE(Default.valid());
    const Field Initialized(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 4, Zero);
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
      EXPECT_EQ(static_cast<const IntegerConstant *>(Owner->field(1)->initializer())->bits().tryGetUInt64(), 0U);
    }
  }

  // Replacement may set or clear an initializer until freeze, which preserves the previous initializer on rejection.
  TEST(IRContextLifetimeTest, ReplacesClearsAndFreezesFieldInitializers)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const core::Name Name = core::NamePool::instance().intern("IRContextLifetimeTest.member");
    const auto First = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(7));
    const auto Second = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(19));
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    for (NominalType *Owner : createNominalTypes(Types))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 0, First)));
      ASSERT_TRUE(Owner->replaceField(0, Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 0, Second)));
      EXPECT_EQ(Owner->field(0)->initializer(), Second);
      ASSERT_TRUE(Owner->replaceField(0, Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name)));
      EXPECT_FALSE(Owner->field(0)->hasInitializer());
      EXPECT_EQ(Owner->field(0)->initializer(), nullptr);
      ASSERT_TRUE(Owner->replaceField(0, Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 0, First)));
      Owner->freeze();
      EXPECT_FALSE(Owner->addField(Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 4, Second)));
      EXPECT_FALSE(Owner->replaceField(0, Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 0, Second)));
      EXPECT_FALSE(Owner->replaceField(0, Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name)));
      ASSERT_EQ(Owner->fieldCount(), 1U);
      EXPECT_TRUE(Owner->field(0)->hasInitializer());
      EXPECT_EQ(Owner->field(0)->initializer(), First);
    }
  }
} // namespace ink::ir::test
