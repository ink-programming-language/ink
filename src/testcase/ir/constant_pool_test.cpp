#include "ink/core/context.h"
#include "ink/ir/model/constant_pool.h"
#include "ink/ir/model/context.h"
#include "ink/ir/model/operand.h"
#include "ink/ir/model/struct_type.h"
#include "ink/ir/model/value_handle.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir
{
  namespace
  {
    struct ConstantPoolTestContext
    {
        core::CompilationContext Compilation;
        IRContext IR{Compilation};
    };

    // Verifies that integer interning includes the type, payload, and negative-sign flag while merging equal nonnegative representations.
    TEST(ConstantPoolTest, InternsIntegerConstantsByExactRepresentation)
    {
      ConstantPoolTestContext Context;
      ConstantPool &Pool = Context.IR.constantPool();
      const Type &ByteType = Context.IR.getType(TypeKind::Byte);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const IntegerConstant &SignedSeven = Pool.getIntegerConstant(I32Type, std::int32_t{7});
      const IntegerConstant &UnsignedSeven = Pool.getIntegerConstant(I32Type, std::uint64_t{7});
      const IntegerConstant &ByteSeven = Pool.getIntegerConstant(ByteType, std::uint8_t{7});
      const IntegerConstant &SignedMinusOne = Pool.getIntegerConstant(I32Type, std::int64_t{-1});
      const IntegerConstant &UnsignedMaximum = Pool.getIntegerConstant(I32Type, std::numeric_limits<std::uint64_t>::max());

      EXPECT_EQ(&SignedSeven, &UnsignedSeven);
      EXPECT_NE(&SignedSeven, &ByteSeven);
      EXPECT_NE(&SignedMinusOne, &UnsignedMaximum);
      EXPECT_EQ(SignedMinusOne.unsignedValue(), UnsignedMaximum.unsignedValue());
      EXPECT_TRUE(SignedMinusOne.isNegative());
      EXPECT_FALSE(UnsignedMaximum.isNegative());
      EXPECT_EQ(Pool.size(), 4U);
    }

    // Verifies that floating-point constants retain type, format, signed-zero bits, and invalid-but-representable format distinctions in their keys.
    TEST(ConstantPoolTest, InternsFloatingPointConstantsByTypeFormatAndBits)
    {
      ConstantPoolTestContext Context;
      ConstantPool &Pool = Context.IR.constantPool();
      const Type &F32Type = Context.IR.getType(TypeKind::F32);
      const Type &F64Type = Context.IR.getType(TypeKind::F64);
      const FloatConstant &PositiveZero = Pool.getFloatConstant(F32Type, FloatFormat::F32, 0x00000000ULL);
      const FloatConstant &DuplicatePositiveZero = Pool.getFloatConstant(F32Type, FloatFormat::F32, 0x00000000ULL);
      const FloatConstant &NegativeZero = Pool.getFloatConstant(F32Type, FloatFormat::F32, 0x80000000ULL);
      const FloatConstant &DifferentType = Pool.getFloatConstant(F64Type, FloatFormat::F32, 0x00000000ULL);
      const FloatConstant &DifferentFormat = Pool.getFloatConstant(F32Type, FloatFormat::F64, 0x00000000ULL);

      EXPECT_EQ(&PositiveZero, &DuplicatePositiveZero);
      EXPECT_NE(&PositiveZero, &NegativeZero);
      EXPECT_NE(&PositiveZero, &DifferentType);
      EXPECT_NE(&PositiveZero, &DifferentFormat);
      EXPECT_EQ(Pool.size(), 4U);
    }

    // Verifies that strings use all bytes, including embedded NULs, and their declared type when selecting a canonical object.
    TEST(ConstantPoolTest, InternsStringConstantsWithoutTextTruncation)
    {
      ConstantPoolTestContext Context;
      ConstantPool &Pool = Context.IR.constantPool();
      const Type &ConstByteSliceType = Context.IR.getType(TypeKind::ConstByteSlice);
      const Type &ByteSliceType = Context.IR.getType(TypeKind::ByteSlice);
      const std::string BinaryText("a\0b", 3);
      const StringConstant &First = Pool.getStringConstant(ConstByteSliceType, BinaryText);
      const StringConstant &Duplicate = Pool.getStringConstant(ConstByteSliceType, std::string("a\0b", 3));
      const StringConstant &DifferentBytes = Pool.getStringConstant(ConstByteSliceType, std::string("a\0c", 3));
      const StringConstant &DifferentType = Pool.getStringConstant(ByteSliceType, BinaryText);

      EXPECT_EQ(&First, &Duplicate);
      EXPECT_NE(&First, &DifferentBytes);
      EXPECT_NE(&First, &DifferentType);
      EXPECT_EQ(First.data(), BinaryText);
      EXPECT_EQ(Pool.size(), 3U);
    }

    // Verifies that one null constant is canonical per pointer type and that the pool reports only its own entries as owned.
    TEST(ConstantPoolTest, InternsNullConstantsPerTypeAndTracksOwnership)
    {
      ConstantPoolTestContext Context;
      ConstantPool &Pool = Context.IR.constantPool();
      const Type &BytePointerType = Context.IR.getType(TypeKind::BytePointer);
      const Type &ConstBytePointerType = Context.IR.getType(TypeKind::ConstBytePointer);
      const NullConstant &MutableNull = Pool.getNullConstant(BytePointerType);
      const NullConstant &DuplicateMutableNull = Pool.getNullConstant(BytePointerType);
      const NullConstant &ConstNull = Pool.getNullConstant(ConstBytePointerType);
      ConstantPool OtherPool;
      const NullConstant &ExternalNull = OtherPool.getNullConstant(BytePointerType);

      EXPECT_EQ(&MutableNull, &DuplicateMutableNull);
      EXPECT_NE(&MutableNull, &ConstNull);
      EXPECT_TRUE(Pool.owns(MutableNull));
      EXPECT_TRUE(Pool.owns(ConstNull));
      EXPECT_FALSE(Pool.owns(ExternalNull));
      EXPECT_EQ(Pool.size(), 2U);
    }

    // Verifies that zero initializers are canonical per type and remain distinct from every other zero-valued constant kind.
    TEST(ConstantPoolTest, InternsZeroInitializersPerType)
    {
      ConstantPoolTestContext Context;
      ConstantPool &Pool = Context.IR.constantPool();
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const Type &F32Type = Context.IR.getType(TypeKind::F32);
      const ZeroInitializer &I32Zero = Pool.getZeroInitializer(I32Type);
      const ZeroInitializer &DuplicateI32Zero = Pool.getZeroInitializer(I32Type);
      const ZeroInitializer &F32Zero = Pool.getZeroInitializer(F32Type);
      const IntegerConstant &IntegerZero = Pool.getIntegerConstant(I32Type, 0);

      EXPECT_EQ(&I32Zero, &DuplicateI32Zero);
      EXPECT_NE(&I32Zero, &F32Zero);
      EXPECT_NE(static_cast<const Constant *>(&I32Zero), static_cast<const Constant *>(&IntegerZero));
      EXPECT_EQ(Pool.size(), 3U);
    }

    // Verifies that aggregate interning compares element order recursively and canonicalizes constants received from another pool.
    TEST(ConstantPoolTest, InternsNestedAggregateConstantsStructurally)
    {
      ConstantPoolTestContext Context;
      ConstantPool &Pool = Context.IR.constantPool();
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const IntegerConstant &One = Pool.getIntegerConstant(I32Type, 1);
      const IntegerConstant &Two = Pool.getIntegerConstant(I32Type, 2);
      const StructType &PairType = Context.IR.createStructType("Pair", {&I32Type, &I32Type});
      const StructType &OtherPairType = Context.IR.createStructType("OtherPair", {&I32Type, &I32Type});
      const StructType &NestedType = Context.IR.createStructType("Nested", {&PairType, &I32Type});
      const AggregateConstant &Pair = Pool.getAggregateConstant(PairType, {One, Two});
      const AggregateConstant &DuplicatePair = Pool.getAggregateConstant(PairType, {One, Two});
      const AggregateConstant &ReversedPair = Pool.getAggregateConstant(PairType, {Two, One});
      const AggregateConstant &OtherPair = Pool.getAggregateConstant(OtherPairType, {One, Two});
      const AggregateConstant &Nested = Pool.getAggregateConstant(NestedType, {Pair, One});
      const AggregateConstant &DuplicateNested = Pool.getAggregateConstant(NestedType, {DuplicatePair, One});
      ConstantPool ForeignPool;
      const IntegerConstant &ForeignOne = ForeignPool.getIntegerConstant(I32Type, 1);
      const IntegerConstant &ForeignTwo = ForeignPool.getIntegerConstant(I32Type, 2);
      const AggregateConstant &CanonicalizedForeignPair = Pool.getAggregateConstant(PairType, {ForeignOne, ForeignTwo});

      EXPECT_EQ(&Pair, &DuplicatePair);
      EXPECT_NE(&Pair, &ReversedPair);
      EXPECT_NE(&Pair, &OtherPair);
      EXPECT_EQ(&Nested, &DuplicateNested);
      EXPECT_EQ(&CanonicalizedForeignPair, &Pair);
      ASSERT_EQ(Pair.elements().size(), 2U);
      EXPECT_EQ(&Pair.elements()[0].get(), &One);
      ASSERT_EQ(Nested.elements().size(), 2U);
      EXPECT_EQ(&Nested.elements()[0].get(), &Pair);
      EXPECT_EQ(Pool.size(), 6U);
    }

    // Verifies that references returned before substantial hash-table growth remain stable and retain their payload.
    TEST(ConstantPoolTest, KeepsCanonicalReferencesStableAcrossGrowth)
    {
      ConstantPoolTestContext Context;
      ConstantPool &Pool = Context.IR.constantPool();
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const IntegerConstant *Original = &Pool.getIntegerConstant(I32Type, std::uint64_t{42});
      for (std::uint64_t Value = 0; Value < 4096; ++Value)
      {
        Pool.getIntegerConstant(I32Type, Value);
      }

      EXPECT_EQ(&Pool.getIntegerConstant(I32Type, std::uint64_t{42}), Original);
      EXPECT_EQ(Original->unsignedValue(), 42U);
      EXPECT_EQ(Pool.size(), 4096U);
    }

    // Verifies that each IR context exposes an isolated, noncopyable constant pool through const and mutable accessors.
    TEST(ConstantPoolTest, IsScopedToItsIrContext)
    {
      static_assert(!std::is_copy_constructible_v<ConstantPool>);
      static_assert(!std::is_move_constructible_v<ConstantPool>);
      static_assert(!std::is_constructible_v<IntegerConstant, const Type &, int>);
      static_assert(!std::is_constructible_v<FloatConstant, const Type &, FloatFormat, std::uint64_t>);
      static_assert(!std::is_constructible_v<StringConstant, const Type &, std::string>);
      static_assert(!std::is_constructible_v<NullConstant, const Type &>);
      static_assert(!std::is_constructible_v<ZeroInitializer, const Type &>);
      static_assert(!std::is_constructible_v<AggregateConstant, const Type &, std::vector<std::reference_wrapper<const Constant>>>);
      static_assert(!std::is_copy_constructible_v<IntegerConstant>);
      static_assert(!std::is_copy_constructible_v<FloatConstant>);
      static_assert(!std::is_copy_constructible_v<StringConstant>);
      static_assert(!std::is_copy_constructible_v<NullConstant>);
      static_assert(!std::is_copy_constructible_v<ZeroInitializer>);
      static_assert(!std::is_copy_constructible_v<AggregateConstant>);
      ConstantPoolTestContext FirstContext;
      ConstantPoolTestContext SecondContext;
      const Type &FirstI32Type = FirstContext.IR.getType(TypeKind::I32);
      const Type &SecondI32Type = SecondContext.IR.getType(TypeKind::I32);
      const IntegerConstant &First = FirstContext.IR.constantPool().getIntegerConstant(FirstI32Type, 9);
      const IntegerConstant &Second = SecondContext.IR.constantPool().getIntegerConstant(SecondI32Type, 9);
      const IRContext &ConstIR = FirstContext.IR;

      EXPECT_NE(&First, &Second);
      EXPECT_EQ(&ConstIR.constantPool(), &FirstContext.IR.constantPool());
      EXPECT_TRUE(ConstIR.constantPool().owns(First));
      EXPECT_FALSE(ConstIR.constantPool().owns(Second));
    }

    // Verifies that value handles borrow canonical constants, own concrete operands, and preserve the correct pointer across moves.
    TEST(ConstantPoolTest, ValueHandleSeparatesBorrowedConstantsFromOwnedOperands)
    {
      static_assert(!std::is_copy_constructible_v<ValueHandle>);
      static_assert(std::is_nothrow_move_constructible_v<ValueHandle>);
      static_assert(std::is_constructible_v<ValueHandle, std::unique_ptr<ValueOperand>>);
      static_assert(!std::is_constructible_v<ValueHandle, std::unique_ptr<IntegerConstant>>);
      ConstantPoolTestContext Context;
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const IntegerConstant &ConstantValue = Context.IR.constantPool().getIntegerConstant(I32Type, 12);
      ValueHandle Borrowed(ConstantValue);
      ValueHandle MovedBorrowed(std::move(Borrowed));
      ValueHandle Owned(std::make_unique<ValueOperand>(I32Type, ValueId{3}));
      const Value *OwnedPointer = Owned.get();
      ValueHandle MovedOwned(std::move(Owned));

      EXPECT_FALSE(Borrowed);
      ASSERT_TRUE(MovedBorrowed);
      EXPECT_EQ(MovedBorrowed.get(), &ConstantValue);
      EXPECT_FALSE(Owned);
      ASSERT_TRUE(MovedOwned);
      EXPECT_EQ(MovedOwned.get(), OwnedPointer);
      EXPECT_EQ(MovedOwned->kind(), ValueKind::ValueOperand);
    }

    // Verifies that production deserialization interns repeated constants and stores borrowed canonical objects in instruction operands.
    TEST(ConstantPoolTest, DeserializationUsesCanonicalConstantsInInstructions)
    {
      ConstantPoolTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @first() {\n"
          "entry:\n"
          "  ret i32 7\n"
          "}\n"
          "define i32 @second() {\n"
          "entry:\n"
          "  ret i32 7\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.module().has_value());
      ASSERT_EQ(Result.module()->Functions.size(), 2U);
      const ReturnInstruction &FirstReturn = static_cast<const ReturnInstruction &>(*Result.module()->Functions[0].Blocks[0].Instructions[0]);
      const ReturnInstruction &SecondReturn = static_cast<const ReturnInstruction &>(*Result.module()->Functions[1].Blocks[0].Instructions[0]);
      ASSERT_TRUE(FirstReturn.ReturnValue);
      ASSERT_TRUE(SecondReturn.ReturnValue);
      EXPECT_EQ(FirstReturn.ReturnValue.get(), SecondReturn.ReturnValue.get());
      EXPECT_TRUE(Context.IR.constantPool().owns(static_cast<const Constant &>(*FirstReturn.ReturnValue)));
      EXPECT_EQ(Context.IR.constantPool().size(), 1U);
    }
  } // namespace
} // namespace ink::ir
