#include "ink/core/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/type/nominal_type/field.h"
#include "ink/ir/type/null_pointer_type.h"
#include "ink/ir/type/type_context.h"

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

    class UnownedNullTypedConstant final : public Constant
    {
      public:
        explicit UnownedNullTypedConstant(const Type *ConstantType, ValueKind Kind = ValueKind::IntegerConstant) noexcept
            : Constant(Kind, ConstantType)
        {
        }
    };
  } // namespace

  static_assert(std::is_final_v<NullPointerType>);
  static_assert(std::is_base_of_v<Type, NullPointerType>);
  static_assert(std::is_same_v<decltype(std::declval<const TypeContext &>().getNullPointerType()), const NullPointerType *>);
  static_assert(std::is_same_v<decltype(std::declval<const ConstantPool &>().getNullPointerConstant()), const NullPointerConstant *>);
  static_assert(noexcept(std::declval<const ConstantPool &>().getNullPointerConstant()));
  static_assert(noexcept(std::declval<const Constant &>().canInitialize(nullptr)));

  // The null literal type is distinct from pointer types and uses its own context's target pointer layout.
  TEST(IRNullPointerTypeTest, KeepsTargetLayoutAndContextIdentity)
  {
    const TypeContext Types32(Target32);
    const TypeContext Types64(Target64);
    TypeContext Other64(Target64);
    const auto Null32 = Types32.getNullPointerType();
    const auto Null64 = Types64.getNullPointerType();
    ASSERT_NE(Null32, nullptr);
    ASSERT_NE(Null64, nullptr);
    EXPECT_EQ(Null32->size(), 4U);
    EXPECT_EQ(Null32->alignment(), 4U);
    EXPECT_EQ(Null64->size(), 8U);
    EXPECT_EQ(Null64->alignment(), 8U);
    EXPECT_TRUE(Null32->hasLayout());
    EXPECT_TRUE(Null64->hasLayout());
    EXPECT_EQ(&Null32->typeContext(), &Types32);
    EXPECT_EQ(&Null64->typeContext(), &Types64);
    EXPECT_NE(Null64, Other64.getNullPointerType());
    EXPECT_EQ(Null64->kind(), ValueKind::NullPointer);
    EXPECT_EQ(valueKindName(ValueKind::NullPointer), "null_pointer");
    EXPECT_TRUE(Type::classof(Null64));
    EXPECT_TRUE(NullPointerType::classof(Null64));
    EXPECT_FALSE(PointerType::classof(Null64));
    EXPECT_FALSE(Constant::classof(Null64));
    EXPECT_FALSE(NullPointerType::classof(nullptr));
    EXPECT_FALSE(NullPointerType::classof(Other64.getPointerType(Other64.getBoolType())));
    EXPECT_FALSE(NullPointerType::classof(Types64.constantPool().getNullPointerConstant()));
    EXPECT_TRUE(Types64.owns(Null64));
    EXPECT_FALSE(Other64.owns(Null64));
  }

  // A pool's one null literal retains its address and type through type and constant storage growth and const access.
  TEST(IRNullPointerTypeTest, ReusesOneStableNullLiteralPerPool)
  {
    TypeContext Types(Target64);
    TypeContext OtherTypes(Target64);
    auto &Constants = Types.constantPool();
    const auto Null = Constants.getNullPointerConstant();
    ASSERT_NE(Null, nullptr);
    EXPECT_EQ(Null->type(), Types.getNullPointerType());
    EXPECT_TRUE(NullPointerConstant::classof(Null));
    EXPECT_TRUE(Constant::classof(Null));
    EXPECT_FALSE(Type::classof(Null));
    EXPECT_NE(Null, OtherTypes.constantPool().getNullPointerConstant());
    for (std::uint64_t Count = 0; Count < 128; ++Count)
    {
      const auto Array = Types.getArrayType(Types.getInt32Type(), Count);
      ASSERT_NE(Array, nullptr);
      ASSERT_NE(Types.getPointerType(Array), nullptr);
      ASSERT_NE(Constants.getUnsignedIntegerConstant(Types.getUInt64Type(), Count), nullptr);
    }
    const TypeContext &ReadOnly = Types;
    EXPECT_EQ(ReadOnly.constantPool().getNullPointerConstant(), Null);
    EXPECT_EQ(Null->type(), ReadOnly.getNullPointerType());
    EXPECT_TRUE(ReadOnly.constantPool().owns(Null));
    EXPECT_FALSE(OtherTypes.constantPool().owns(Null));
    EXPECT_FALSE(ReadOnly.constantPool().owns(OtherTypes.constantPool().getNullPointerConstant()));
  }

  // Only the actual null literal gains conversion to local pointer types; nonpointers and foreign types stay incompatible.
  TEST(IRNullPointerTypeTest, InitializesOnlyIdenticalOrLocalPointerTypes)
  {
    TypeContext Types(Target64);
    TypeContext OtherTypes(Target64);
    const auto Null = Types.constantPool().getNullPointerConstant();
    const auto Function = Types.getFunctionType(Types.getVoidType(), {Types.getInt32Type()});
    const auto Nominal = Types.createStructType(8, 8);
    const auto IntPointer = Types.getPointerType(Types.getInt32Type());
    ASSERT_NE(Function, nullptr);
    ASSERT_NE(Nominal, nullptr);
    ASSERT_NE(IntPointer, nullptr);
    const Type *Pointees[] = {
        Types.getBoolType(),
        Types.getInt32Type(),
        Types.getVoidType(),
        IntPointer,
        Function,
        Nominal,
    };
    for (const Type *Pointee : Pointees)
    {
      const auto Pointer = Types.getPointerType(Pointee);
      ASSERT_NE(Pointer, nullptr);
      EXPECT_TRUE(Null->canInitialize(Pointer));
    }
    const Type *Rejected[] = {
        nullptr,
        Types.getBoolType(),
        Types.getInt32Type(),
        Types.getFloat64Type(),
        Types.getVoidType(),
        Types.getVariadicType(),
        Types.getReferenceType(Types.getInt32Type()),
        Types.getArrayType(IntPointer, 1),
        Function,
        Nominal,
        OtherTypes.getPointerType(OtherTypes.getInt32Type()),
        OtherTypes.getNullPointerType(),
    };
    for (const Type *Destination : Rejected)
    {
      EXPECT_FALSE(Null->canInitialize(Destination));
    }
    EXPECT_TRUE(Null->canInitialize(Types.getNullPointerType()));
    const UnownedNullTypedConstant Unowned(Types.getNullPointerType());
    EXPECT_TRUE(Unowned.canInitialize(Types.getNullPointerType()));
    EXPECT_FALSE(Unowned.canInitialize(IntPointer));
    const UnownedNullTypedConstant UnownedNull(Types.getNullPointerType(), ValueKind::NullPointerConstant);
    EXPECT_FALSE(Types.constantPool().owns(&UnownedNull));
    EXPECT_FALSE(UnownedNull.canInitialize(IntPointer));
    const auto Zero = Types.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 0);
    ASSERT_NE(Zero, nullptr);
    EXPECT_TRUE(Zero->canInitialize(Types.getInt32Type()));
    EXPECT_FALSE(Zero->canInitialize(Types.getUInt32Type()));
    EXPECT_FALSE(Zero->canInitialize(IntPointer));
  }

  // Fields of distinct pointer types share the literal while clearing a field remains distinct from explicit null initialization.
  TEST(IRNullPointerTypeTest, SharesNullAcrossFieldsAndPreservesAbsentInitializers)
  {
    TypeContext Types(Target64);
    const auto Null = Types.constantPool().getNullPointerConstant();
    const auto BoolPointer = Types.getPointerType(Types.getBoolType());
    const auto IntPointer = Types.getPointerType(Types.getInt32Type());
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

  // Invalid or foreign null field bindings and frozen mutations reject changes without losing the registered initializer.
  TEST(IRNullPointerTypeTest, RejectsInvalidAndFrozenFieldInitializers)
  {
    TypeContext Types(Target64);
    TypeContext OtherTypes(Target64);
    const auto Null = Types.constantPool().getNullPointerConstant();
    const auto ForeignNull = OtherTypes.constantPool().getNullPointerConstant();
    const auto Pointer = Types.getPointerType(Types.getInt32Type());
    const core::Name Name = core::NamePool::instance().intern("IRNullPointerTypeTest.member");
    auto Owner = Types.createStructType(16, 8);
    ASSERT_NE(Owner, nullptr);
    ASSERT_TRUE(Owner->addField(Field(Pointer, Name, 0, Null)));
    const UnownedNullTypedConstant Unowned(Types.getNullPointerType());
    const Field Rejected[] = {
        Field(Types.getInt32Type(), Name, 0, Null),
        Field(Types.getReferenceType(Types.getInt32Type()), Name, 0, Null),
        Field(Pointer, Name, 0, ForeignNull),
        Field(Pointer, Name, 0, &Unowned),
        Field(OtherTypes.getPointerType(OtherTypes.getInt32Type()), Name, 0, ForeignNull),
    };
    for (const Field &Entry : Rejected)
    {
      EXPECT_FALSE(Owner->addField(Entry));
      EXPECT_FALSE(Owner->replaceField(0, Entry));
      ASSERT_EQ(Owner->fieldCount(), 1U);
      EXPECT_EQ(Owner->field(0)->type(), Pointer);
      EXPECT_EQ(Owner->field(0)->initializer(), Null);
    }
    Owner->freeze();
    EXPECT_FALSE(Owner->addField(Field(Pointer, Name, 8, Null)));
    EXPECT_FALSE(Owner->replaceField(0, Field(Pointer, Name)));
    EXPECT_FALSE(Owner->replaceField(0, Field(Pointer, Name, 0, Null)));
    EXPECT_EQ(Owner->field(0)->initializer(), Null);
    EXPECT_EQ(Owner->fieldCount(), 1U);
  }

  // Distinct pointer globals accept one literal and clear it explicitly; rejected replacements preserve the existing value.
  TEST(IRNullPointerTypeTest, SharesNullAcrossGlobalsAndRejectsInvalidReplacements)
  {
    TypeContext Types(Target64);
    TypeContext OtherTypes(Target64);
    const auto Null = Types.constantPool().getNullPointerConstant();
    const auto BoolPointer = Types.getPointerType(Types.getBoolType());
    const auto IntPointer = Types.getPointerType(Types.getInt32Type());
    GlobalVariable BoolVariable(*BoolPointer);
    GlobalVariable IntVariable(*IntPointer);
    ASSERT_TRUE(BoolVariable.setInitializer(Null));
    ASSERT_TRUE(IntVariable.setInitializer(Null));
    EXPECT_EQ(BoolVariable.initializer(), IntVariable.initializer());
    EXPECT_TRUE(BoolVariable.hasInitializer());
    const auto ForeignNull = OtherTypes.constantPool().getNullPointerConstant();
    EXPECT_FALSE(BoolVariable.setInitializer(ForeignNull));
    EXPECT_FALSE(BoolVariable.setInitializer(Types.constantPool().getBoolConstant(false)));
    EXPECT_EQ(BoolVariable.initializer(), Null);
    ASSERT_TRUE(BoolVariable.setInitializer(nullptr));
    EXPECT_FALSE(BoolVariable.hasInitializer());
    EXPECT_EQ(BoolVariable.initializer(), nullptr);
    EXPECT_EQ(IntVariable.initializer(), Null);
    GlobalVariable NonPointer(*Types.getBoolType());
    const auto False = Types.constantPool().getBoolConstant(false);
    ASSERT_TRUE(NonPointer.setInitializer(False));
    EXPECT_FALSE(NonPointer.setInitializer(Null));
    EXPECT_EQ(NonPointer.initializer(), False);
    GlobalVariable ForeignVariable(*OtherTypes.getPointerType(OtherTypes.getBoolType()));
    EXPECT_FALSE(ForeignVariable.setInitializer(Null));
    EXPECT_FALSE(ForeignVariable.hasInitializer());
  }

  // Pointer arrays share null elements while array identity, nested element types and constant-pool ownership stay exact.
  TEST(IRNullPointerTypeTest, ReusesNullElementsWithoutConvertingArrayTypes)
  {
    TypeContext Types(Target64);
    TypeContext OtherTypes(Target64);
    auto &Constants = Types.constantPool();
    const auto Null = Constants.getNullPointerConstant();
    const auto BoolArrayType = Types.getArrayType(Types.getPointerType(Types.getBoolType()), 2);
    const auto IntArrayType = Types.getArrayType(Types.getPointerType(Types.getInt32Type()), 2);
    ASSERT_NE(BoolArrayType, nullptr);
    ASSERT_NE(IntArrayType, nullptr);
    const auto BoolArray = Constants.getArrayConstant(BoolArrayType, {Null, Null});
    const auto IntArray = Constants.getArrayConstant(IntArrayType, {Null, Null});
    ASSERT_NE(BoolArray, nullptr);
    ASSERT_NE(IntArray, nullptr);
    EXPECT_NE(BoolArray, IntArray);
    EXPECT_EQ(BoolArray->type(), BoolArrayType);
    EXPECT_EQ(IntArray->type(), IntArrayType);
    EXPECT_EQ(BoolArray->elements()[0], IntArray->elements()[0]);
    EXPECT_EQ(BoolArray, Constants.getArrayConstant(BoolArrayType, {Null, Null}));
    EXPECT_TRUE(BoolArray->canInitialize(BoolArrayType));
    EXPECT_FALSE(BoolArray->canInitialize(IntArrayType));
    const auto NestedType = Types.getArrayType(BoolArrayType, 2);
    ASSERT_NE(NestedType, nullptr);
    EXPECT_NE(Constants.getArrayConstant(NestedType, {BoolArray, BoolArray}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(NestedType, {BoolArray, IntArray}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(NestedType, {Null, Null}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(BoolArrayType, {Null, nullptr}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(BoolArrayType, {Null, OtherTypes.constantPool().getNullPointerConstant()}), nullptr);
    EXPECT_EQ(Constants.getArrayConstant(Types.getArrayType(Types.getBoolType(), 2), {Null, Null}), nullptr);
    const auto ForeignArrayType = OtherTypes.getArrayType(OtherTypes.getPointerType(OtherTypes.getBoolType()), 2);
    EXPECT_EQ(Constants.getArrayConstant(ForeignArrayType, {Null, Null}), nullptr);
    GlobalVariable Variable(*IntArrayType);
    ASSERT_TRUE(Variable.setInitializer(IntArray));
    EXPECT_FALSE(Variable.setInitializer(BoolArray));
    EXPECT_EQ(Variable.initializer(), IntArray);
  }
} // namespace ink::ir::test
