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
    const Type &Meta = Context.getMetaType();
    EXPECT_EQ(&Meta.type(), &Meta);
    EXPECT_EQ(&Context.getVoidType().type(), &Meta);
    EXPECT_EQ(&Context.getBoolType().type(), &Meta);
    EXPECT_EQ(Meta.typeKind(), TypeKind::Meta);
    EXPECT_EQ(Context.getVoidType().typeKind(), TypeKind::Void);
    EXPECT_EQ(Context.getBoolType().typeKind(), TypeKind::Bool);
    EXPECT_EQ(Context.getLabelType().typeKind(), TypeKind::Label);
    EXPECT_EQ(Context.getModuleType().typeKind(), TypeKind::Module);
    EXPECT_TRUE(Type::classof(&Meta));
    EXPECT_FALSE(Constant::classof(&Meta));
    const Value &Boolean = Context.getBoolConstant(true);
    EXPECT_EQ(&Boolean.type(), &Context.getBoolType());
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
    const IntegerType *Signed32 = Context.getIntegerType(32, true);
    ASSERT_NE(Signed32, nullptr);
    EXPECT_EQ(Signed32, Context.getIntegerType(32, true));
    EXPECT_NE(Signed32, Context.getIntegerType(32, false));
    EXPECT_NE(Signed32, Context.getIntegerType(64, true));
    EXPECT_NE(Signed32, Other.getIntegerType(32, true));
    EXPECT_EQ(Signed32->bitWidth(), 32U);
    EXPECT_TRUE(Signed32->isSigned());
    EXPECT_EQ(&Signed32->type(), &Context.getMetaType());
    EXPECT_TRUE(IntegerType::classof(Signed32));
    EXPECT_FALSE(IntegerType::classof(&Context.getBoolType()));
    EXPECT_EQ(Context.getIntegerType(0, true), nullptr);
  }

  // Constants reuse exact typed bit patterns without merging signed and unsigned values.
  TEST(SemanticModelTest, IntegerConstantsPreserveWideValuesAndSignedness)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Signed = Context.getIntegerType(128, true);
    const IntegerType *Unsigned = Context.getIntegerType(128, false);
    const std::uint64_t AllOneWords[] = {
        UINT64_MAX,
        UINT64_MAX,
    };
    IntegerBits Bits(128, AllOneWords);
    const IntegerConstant *Negative = Context.getIntegerConstant(*Signed, Bits);
    const IntegerConstant *Positive = Context.getIntegerConstant(*Unsigned, Bits);
    ASSERT_NE(Negative, nullptr);
    ASSERT_NE(Positive, nullptr);
    EXPECT_EQ(Negative, Context.getIntegerConstant(*Signed, IntegerBits(128, AllOneWords)));
    EXPECT_NE(Negative, Positive);
    EXPECT_EQ(Negative->value().bitWidth(), 128U);
    EXPECT_EQ(Negative->value(), IntegerBits(128, AllOneWords));
    EXPECT_EQ(&Negative->type(), Signed);
    EXPECT_EQ(&Positive->type(), Unsigned);
    Bits = IntegerBits(128, 0);
    EXPECT_EQ(Negative->value(), IntegerBits(128, AllOneWords));
    EXPECT_NE(Negative, Context.getIntegerConstant(*Signed, Bits));
    EXPECT_TRUE(IntegerConstant::classof(Negative));
  }

  // Invalid constant requests report failure instead of truncating bits or borrowing foreign types.
  TEST(SemanticModelTest, IntegerConstantsRejectMismatchedWidthsAndForeignTypes)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const IntegerType *Signed32 = Context.getIntegerType(32, true);
    EXPECT_EQ(Context.getIntegerConstant(*Signed32, IntegerBits(64, 1)), nullptr);
    EXPECT_EQ(Context.getIntegerConstant(*Other.getIntegerType(32, true), IntegerBits(32, 1)), nullptr);
    EXPECT_NE(Context.getIntegerConstant(*Signed32, IntegerBits(32, 1)), nullptr);
  }

  // Repeated bool requests share immutable values and keep false and true distinct.
  TEST(SemanticModelTest, BoolConstantsAreCanonical)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const BoolConstant &False = Context.getBoolConstant(false);
    const BoolConstant &True = Context.getBoolConstant(true);
    EXPECT_EQ(&False, &Context.getBoolConstant(false));
    EXPECT_EQ(&True, &Context.getBoolConstant(true));
    EXPECT_NE(&False, &True);
    EXPECT_FALSE(False.value());
    EXPECT_TRUE(True.value());
    EXPECT_EQ(&False.type(), &Context.getBoolType());
  }

  // Container growth preserves instruction operands, type identities and constant addresses.
  TEST(SemanticModelTest, ModelIdentitiesSurviveStorageGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const IntegerConstant *Zero = Context.getIntegerConstant(*Int32, IntegerBits(32, 0));
    AllocaInstruction *Slot = Context.createAllocaInstruction(*Int32);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(Zero, nullptr);
    const StoreInstruction *Store = Context.createStoreInstruction(*Slot, *Zero);
    ASSERT_NE(Store, nullptr);
    for (std::uint32_t Index = 1; Index <= 1024; ++Index)
    {
      ASSERT_NE(Context.getIntegerType(Index, false), nullptr);
      ASSERT_NE(Context.getIntegerConstant(*Int32, IntegerBits(32, Index)), nullptr);
      ASSERT_NE(Context.createAllocaInstruction(*Int32), nullptr);
    }
    EXPECT_EQ(&Slot->allocatedType(), Int32);
    EXPECT_EQ(&Store->address(), Slot);
    EXPECT_EQ(&Store->storedValue(), Zero);
    EXPECT_EQ(Int32, Context.getIntegerType(32, true));
    EXPECT_EQ(Zero, Context.getIntegerConstant(*Int32, IntegerBits(32, 0)));
    EXPECT_EQ(Zero->value(), IntegerBits(32, 0));
  }
} // namespace ink::semantic::test
