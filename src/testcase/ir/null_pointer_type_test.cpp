#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/type/nominal_type/field.h"
#include "ink/ir/type/builtin_type/null_pointer_type.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  static_assert(std::is_final_v<NullPointerType>);
  static_assert(std::is_base_of_v<Type, NullPointerType>);
  static_assert(std::is_same_v<decltype(std::declval<const Module &>().typePool().get<ink::ir::NullPointerType>()), const NullPointerType *>);
  static_assert(std::is_same_v<decltype(std::declval<const ConstantPool &>().get<ink::ir::NullPointerConstant>()), const NullPointerConstant *>);
  static_assert(noexcept(std::declval<const ConstantPool &>().get<ink::ir::NullPointerConstant>()));
  static_assert(noexcept(std::declval<const Constant &>().canInitialize(nullptr)));

  // The null literal type is distinct from pointer types and uses its owning module's target pointer layout.
  TEST(IRNullPointerTypeTest, KeepsTargetLayoutAndModuleIdentity)
  {
    IRContext Types32Context(Target32);
    const Module Types32(Types32Context);
    IRContext Types64Context(Target64);
    const Module Types64(Types64Context);
    IRContext Other64Context(Target64);
    Module Other64(Other64Context);
    const auto Null32 = Types32.typePool().get<ink::ir::NullPointerType>();
    const auto Null64 = Types64.typePool().get<ink::ir::NullPointerType>();
    ASSERT_NE(Null32, nullptr);
    ASSERT_NE(Null64, nullptr);
    EXPECT_EQ(Null32->size(), 4U);
    EXPECT_EQ(Null32->alignment(), 4U);
    EXPECT_EQ(Null64->size(), 8U);
    EXPECT_EQ(Null64->alignment(), 8U);
    EXPECT_TRUE(Null32->hasLayout());
    EXPECT_TRUE(Null64->hasLayout());
    EXPECT_EQ(&Null32->irContext(), &Types32.irContext());
    EXPECT_EQ(&Null64->irContext(), &Types64.irContext());
    EXPECT_NE(Null64, Other64.typePool().get<ink::ir::NullPointerType>());
    EXPECT_EQ(Null64->kind(), ValueKind::NullPointer);
    EXPECT_EQ(valueKindName(ValueKind::NullPointer), "null_pointer");
    EXPECT_TRUE(Type::classof(Null64));
    EXPECT_TRUE(NullPointerType::classof(Null64));
    EXPECT_FALSE(PointerType::classof(Null64));
    EXPECT_FALSE(Constant::classof(Null64));
    EXPECT_FALSE(NullPointerType::classof(nullptr));
    EXPECT_FALSE(NullPointerType::classof(Other64.typePool().get<ink::ir::PointerType>(Other64.typePool().get<ink::ir::BoolType>())));
    EXPECT_FALSE(NullPointerType::classof(Types64.constantPool().get<ink::ir::NullPointerConstant>()));
    EXPECT_TRUE(Types64.ownsType(Null64));
    EXPECT_FALSE(Other64.ownsType(Null64));
  }

  // A pool's one null literal retains its address and type through type and constant storage growth and const access.
  TEST(IRNullPointerTypeTest, ReusesOneStableNullLiteralPerPool)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    IRContext OtherTypesContext(Target64);
    Module OtherTypes(OtherTypesContext);
    auto &Constants = Types.constantPool();
    const auto Null = Constants.get<ink::ir::NullPointerConstant>();
    ASSERT_NE(Null, nullptr);
    EXPECT_EQ(Null->type(), Types.typePool().get<ink::ir::NullPointerType>());
    EXPECT_TRUE(NullPointerConstant::classof(Null));
    EXPECT_TRUE(Constant::classof(Null));
    EXPECT_FALSE(Type::classof(Null));
    EXPECT_NE(Null, OtherTypes.constantPool().get<ink::ir::NullPointerConstant>());
    for (std::uint64_t Count = 0; Count < 128; ++Count)
    {
      const auto Array = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(32, true), Count);
      ASSERT_NE(Array, nullptr);
      ASSERT_NE(Types.typePool().get<ink::ir::PointerType>(Array), nullptr);
      ASSERT_NE(Constants.get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(64, false), static_cast<std::uint64_t>(Count)), nullptr);
    }
    const Module &ReadOnly = Types;
    EXPECT_EQ(ReadOnly.constantPool().get<ink::ir::NullPointerConstant>(), Null);
    EXPECT_EQ(Null->type(), ReadOnly.typePool().get<ink::ir::NullPointerType>());
    EXPECT_TRUE(ReadOnly.constantPool().owns(Null));
    EXPECT_FALSE(OtherTypes.constantPool().owns(Null));
    EXPECT_FALSE(ReadOnly.constantPool().owns(OtherTypes.constantPool().get<ink::ir::NullPointerConstant>()));
  }

  // Fields of distinct pointer types share the literal while clearing a field remains distinct from explicit null initialization.
  TEST(IRNullPointerTypeTest, SharesNullAcrossFieldsAndPreservesAbsentInitializers)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Null = Types.constantPool().get<ink::ir::NullPointerConstant>();
    const auto BoolPointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::BoolType>());
    const auto IntPointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true));
    const core::Name Name = core::NamePool::instance().intern("IRNullPointerTypeTest.member");
    const std::array<NominalType *, 3> Owners = {
        Types.createClassType(16, 8),
        Types.createInterfaceType(16, 8),
        Types.createStructType(16, 8),
    };
    for (NominalType *Owner : Owners)
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(BoolPointer, Name, 0, Null)));
      ASSERT_TRUE(Owner->addField(Field(IntPointer, Name, 8, Null)));
      EXPECT_EQ(Owner->field(0)->initializer(), Null);
      EXPECT_EQ(Owner->field(1)->initializer(), Null);
      EXPECT_TRUE(Owner->field(0)->valid());
      EXPECT_TRUE(Owner->field(1)->valid());
      EXPECT_EQ(Owner->field(0)->type(), BoolPointer);
      EXPECT_EQ(Owner->field(1)->type(), IntPointer);
      ASSERT_TRUE(Owner->replaceField(0, Field(BoolPointer, Name)));
      EXPECT_FALSE(Owner->field(0)->hasInitializer());
      EXPECT_EQ(Owner->field(0)->initializer(), nullptr);
      ASSERT_TRUE(Owner->replaceField(0, Field(BoolPointer, Name, 0, Null)));
      EXPECT_TRUE(Owner->field(0)->hasInitializer());
      EXPECT_EQ(Owner->field(0)->initializer(), Null);
    }
  }

  // Distinct pointer globals accept one literal and clear it explicitly; rejected replacements preserve the existing value.
  TEST(IRNullPointerTypeTest, SharesNullAcrossGlobalsAndRejectsInvalidReplacements)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    IRContext OtherTypesContext(Target64);
    Module OtherTypes(OtherTypesContext);
    const auto Null = Types.constantPool().get<ink::ir::NullPointerConstant>();
    const auto BoolPointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::BoolType>());
    const auto IntPointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true));
    GlobalVariable BoolVariable(*BoolPointer);
    GlobalVariable IntVariable(*IntPointer);
    ASSERT_TRUE(BoolVariable.setInitializer(Null));
    ASSERT_TRUE(IntVariable.setInitializer(Null));
    EXPECT_EQ(BoolVariable.initializer(), IntVariable.initializer());
    EXPECT_TRUE(BoolVariable.hasInitializer());
    const auto ForeignNull = OtherTypes.constantPool().get<ink::ir::NullPointerConstant>();
    EXPECT_FALSE(BoolVariable.setInitializer(ForeignNull));
    EXPECT_FALSE(BoolVariable.setInitializer(Types.constantPool().get<ink::ir::BoolConstant>(false)));
    EXPECT_EQ(BoolVariable.initializer(), Null);
    ASSERT_TRUE(BoolVariable.setInitializer(nullptr));
    EXPECT_FALSE(BoolVariable.hasInitializer());
    EXPECT_EQ(BoolVariable.initializer(), nullptr);
    EXPECT_EQ(IntVariable.initializer(), Null);
    GlobalVariable NonPointer(*Types.typePool().get<ink::ir::BoolType>());
    const auto False = Types.constantPool().get<ink::ir::BoolConstant>(false);
    ASSERT_TRUE(NonPointer.setInitializer(False));
    EXPECT_FALSE(NonPointer.setInitializer(Null));
    EXPECT_EQ(NonPointer.initializer(), False);
    GlobalVariable ForeignVariable(*OtherTypes.typePool().get<ink::ir::PointerType>(OtherTypes.typePool().get<ink::ir::BoolType>()));
    EXPECT_FALSE(ForeignVariable.setInitializer(Null));
    EXPECT_FALSE(ForeignVariable.hasInitializer());
  }

  // Pointer arrays share null elements while array identity, nested element types and constant-pool ownership stay exact.
  TEST(IRNullPointerTypeTest, ReusesNullElementsWithoutConvertingArrayTypes)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    IRContext OtherTypesContext(Target64);
    Module OtherTypes(OtherTypesContext);
    auto &Constants = Types.constantPool();
    const auto Null = Constants.get<ink::ir::NullPointerConstant>();
    const auto BoolArrayType = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::BoolType>()), 2);
    const auto IntArrayType = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true)), 2);
    ASSERT_NE(BoolArrayType, nullptr);
    ASSERT_NE(IntArrayType, nullptr);
    const auto BoolArray = Constants.get<ink::ir::ArrayConstant>(BoolArrayType, {Null, Null});
    const auto IntArray = Constants.get<ink::ir::ArrayConstant>(IntArrayType, {Null, Null});
    ASSERT_NE(BoolArray, nullptr);
    ASSERT_NE(IntArray, nullptr);
    EXPECT_NE(BoolArray, IntArray);
    EXPECT_EQ(BoolArray->type(), BoolArrayType);
    EXPECT_EQ(IntArray->type(), IntArrayType);
    EXPECT_EQ(BoolArray->elements()[0], IntArray->elements()[0]);
    EXPECT_EQ(BoolArray, Constants.get<ink::ir::ArrayConstant>(BoolArrayType, {Null, Null}));
    EXPECT_TRUE(BoolArray->canInitialize(BoolArrayType));
    EXPECT_FALSE(BoolArray->canInitialize(IntArrayType));
    const auto NestedType = Types.typePool().get<ink::ir::ArrayType>(BoolArrayType, 2);
    ASSERT_NE(NestedType, nullptr);
    EXPECT_NE(Constants.get<ink::ir::ArrayConstant>(NestedType, {BoolArray, BoolArray}), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(NestedType, {BoolArray, IntArray}), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(NestedType, {Null, Null}), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(BoolArrayType, {Null, nullptr}), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(BoolArrayType, {Null, OtherTypes.constantPool().get<ink::ir::NullPointerConstant>()}), nullptr);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::BoolType>(), 2), {Null, Null}), nullptr);
    const auto ForeignArrayType = OtherTypes.typePool().get<ink::ir::ArrayType>(OtherTypes.typePool().get<ink::ir::PointerType>(OtherTypes.typePool().get<ink::ir::BoolType>()), 2);
    EXPECT_EQ(Constants.get<ink::ir::ArrayConstant>(ForeignArrayType, {Null, Null}), nullptr);
    GlobalVariable Variable(*IntArrayType);
    ASSERT_TRUE(Variable.setInitializer(IntArray));
    EXPECT_FALSE(Variable.setInitializer(BoolArray));
    EXPECT_EQ(Variable.initializer(), IntArray);
  }
} // namespace ink::ir::test
