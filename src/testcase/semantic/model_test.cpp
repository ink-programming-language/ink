#include "ink/semantic/ir_builder.h"
#include "ink/semantic/context.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace ink::semantic::test
{
  static_assert(std::is_base_of_v<Value, Type>);
  static_assert(std::is_base_of_v<Value, Constant>);
  static_assert(std::is_base_of_v<Value, BasicBlock>);
  static_assert(std::is_base_of_v<Value, Module>);
  static_assert(!std::is_base_of_v<Value, Decl>);
  static_assert(!std::is_copy_constructible_v<AllocaInstruction>);

  // Type values use the unique metatype, whose own value type closes the cycle.
  TEST(SemanticModelTest, TypesAndConstantsHaveDistinctKindsAndValueTypes)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Type &Meta = Context.typePool().getType<TypeKind::Meta>();
    EXPECT_EQ(&Meta.type(), &Meta);
    EXPECT_EQ(&Context.typePool().getType<TypeKind::Void>().type(), &Meta);
    EXPECT_EQ(&Context.typePool().getType<TypeKind::Bool>().type(), &Meta);
    EXPECT_EQ(Meta.typeKind(), TypeKind::Meta);
    EXPECT_EQ(Context.typePool().getType<TypeKind::Void>().typeKind(), TypeKind::Void);
    EXPECT_EQ(Context.typePool().getType<TypeKind::Bool>().typeKind(), TypeKind::Bool);
    EXPECT_EQ(Context.typePool().getType<TypeKind::Label>().typeKind(), TypeKind::Label);
    EXPECT_EQ(Context.typePool().getType<TypeKind::Module>().typeKind(), TypeKind::Module);
    EXPECT_TRUE(Type::classof(&Meta));
    EXPECT_FALSE(Constant::classof(&Meta));
    const Value &Boolean = Context.constantPool().getBoolConstant(true);
    EXPECT_EQ(&Boolean.type(), &Context.typePool().getType<TypeKind::Bool>());
    EXPECT_TRUE(Constant::classof(&Boolean));
    EXPECT_TRUE(BoolConstant::classof(&Boolean));
    EXPECT_FALSE(Type::classof(&Boolean));
    EXPECT_FALSE(IntegerConstant::classof(&Boolean));
    EXPECT_EQ(&Context.compilationContext(), &Compilation);
  }

  // Integer types are canonical by width and signedness, with zero width rejected.
  TEST(SemanticModelTest, IntegerTypesAreCanonicalAndContextLocal)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const IntegerType *Signed32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    ASSERT_NE(Signed32, nullptr);
    EXPECT_EQ(Signed32, Context.typePool().getType<TypeKind::Integer>(32, true));
    EXPECT_NE(Signed32, Context.typePool().getType<TypeKind::Integer>(32, false));
    EXPECT_NE(Signed32, Context.typePool().getType<TypeKind::Integer>(64, true));
    EXPECT_NE(Signed32, Other.typePool().getType<TypeKind::Integer>(32, true));
    EXPECT_EQ(Signed32->bitWidth(), 32U);
    EXPECT_TRUE(Signed32->isSigned());
    EXPECT_EQ(&Signed32->type(), &Context.typePool().getType<TypeKind::Meta>());
    EXPECT_TRUE(IntegerType::classof(Signed32));
    EXPECT_FALSE(IntegerType::classof(&Context.typePool().getType<TypeKind::Bool>()));
    EXPECT_EQ(Context.typePool().getType<TypeKind::Integer>(0, true), nullptr);
  }

  // Constants reuse exact typed bit patterns without merging signed and unsigned values.
  TEST(SemanticModelTest, IntegerConstantsPreserveWideValuesAndSignedness)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Signed = Context.typePool().getType<TypeKind::Integer>(128, true);
    const IntegerType *Unsigned = Context.typePool().getType<TypeKind::Integer>(128, false);
    const std::uint64_t AllOneWords[] = {
        UINT64_MAX,
        UINT64_MAX,
    };
    IntegerBits Bits(128, AllOneWords);
    const IntegerConstant *Negative = Context.constantPool().getIntegerConstant(*Signed, Bits);
    const IntegerConstant *Positive = Context.constantPool().getIntegerConstant(*Unsigned, Bits);
    ASSERT_NE(Negative, nullptr);
    ASSERT_NE(Positive, nullptr);
    EXPECT_EQ(Negative, Context.constantPool().getIntegerConstant(*Signed, IntegerBits(128, AllOneWords)));
    EXPECT_NE(Negative, Positive);
    EXPECT_EQ(Negative->value().bitWidth(), 128U);
    EXPECT_EQ(Negative->value(), IntegerBits(128, AllOneWords));
    EXPECT_EQ(&Negative->type(), Signed);
    EXPECT_EQ(&Positive->type(), Unsigned);
    Bits = IntegerBits(128, 0);
    EXPECT_EQ(Negative->value(), IntegerBits(128, AllOneWords));
    EXPECT_NE(Negative, Context.constantPool().getIntegerConstant(*Signed, Bits));
    EXPECT_TRUE(IntegerConstant::classof(Negative));
  }

  // Invalid constant requests report failure instead of truncating bits or borrowing foreign types.
  TEST(SemanticModelTest, IntegerConstantsRejectMismatchedWidthsAndForeignTypes)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const IntegerType *Signed32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    EXPECT_EQ(Context.constantPool().getIntegerConstant(*Signed32, IntegerBits(64, 1)), nullptr);
    EXPECT_EQ(Context.constantPool().getIntegerConstant(*Other.typePool().getType<TypeKind::Integer>(32, true), IntegerBits(32, 1)), nullptr);
    EXPECT_NE(Context.constantPool().getIntegerConstant(*Signed32, IntegerBits(32, 1)), nullptr);
  }

  // Repeated bool requests share immutable values and keep false and true distinct.
  TEST(SemanticModelTest, BoolConstantsAreCanonical)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const BoolConstant &False = Context.constantPool().getBoolConstant(false);
    const BoolConstant &True = Context.constantPool().getBoolConstant(true);
    EXPECT_EQ(&False, &Context.constantPool().getBoolConstant(false));
    EXPECT_EQ(&True, &Context.constantPool().getBoolConstant(true));
    EXPECT_NE(&False, &True);
    EXPECT_FALSE(False.value());
    EXPECT_TRUE(True.value());
    EXPECT_EQ(&False.type(), &Context.typePool().getType<TypeKind::Bool>());
  }

  // Container growth preserves instruction operands, type identities and constant addresses.
  TEST(SemanticModelTest, ModelIdentitiesSurviveStorageGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    const IntegerType *Int32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    const IntegerConstant *Zero = Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 0));
    auto SlotOwner = Factory.createDetachedAllocaInstruction(*Int32);
    AllocaInstruction *Slot = SlotOwner.get();
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(Zero, nullptr);
    auto StoreOwner = Factory.createDetachedStoreInstruction(*Slot, *Zero);
    const StoreInstruction *Store = StoreOwner.get();
    ASSERT_NE(Store, nullptr);
    for (std::uint32_t Index = 1; Index <= 1024; ++Index)
    {
      ASSERT_NE(Context.typePool().getType<TypeKind::Integer>(Index, false), nullptr);
      ASSERT_NE(Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, Index)), nullptr);
      ASSERT_NE(Factory.createDetachedAllocaInstruction(*Int32), nullptr);
    }
    EXPECT_EQ(&Slot->allocatedType(), Int32);
    EXPECT_EQ(&Store->address(), Slot);
    EXPECT_EQ(&Store->storedValue(), Zero);
    EXPECT_EQ(Int32, Context.typePool().getType<TypeKind::Integer>(32, true));
    EXPECT_EQ(Zero, Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 0)));
    EXPECT_EQ(Zero->value(), IntegerBits(32, 0));
  }
} // namespace ink::semantic::test
