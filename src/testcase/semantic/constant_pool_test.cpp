#include "ink/semantic/model/constant/constant_pool.h"
#include "ink/semantic/context.h"

#include <gtest/gtest.h>

#include <iterator>
#include <type_traits>
#include <vector>

namespace ink::semantic::test
{
  static_assert(!std::is_copy_constructible_v<ConstantPool>);
  static_assert(!std::is_copy_assignable_v<ConstantPool>);
  static_assert(!std::is_move_constructible_v<ConstantPool>);
  static_assert(!std::is_move_assignable_v<ConstantPool>);
  static_assert(!std::is_constructible_v<ConstantPool, const SemanticContext &>);

  // Both bool values are owned immediately, and const context/pool access preserves their identity.
  TEST(SemanticConstantPoolTest, BoolConstantsArePreallocatedAndSharedWithContext)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    const SemanticContext &ConstContext = Context;
    const ConstantPool &ConstPool = ConstContext.constantPool();
    EXPECT_EQ(&Pool, &ConstPool);
    EXPECT_EQ(&Pool.context(), &Context);
    EXPECT_EQ(Pool.size(), 2U);
    const BoolConstant &False = ConstPool.getBoolConstant(false);
    const BoolConstant &True = ConstPool.getBoolConstant(true);
    EXPECT_NE(&False, &True);
    EXPECT_EQ(&False, &ConstContext.getBoolConstant(false));
    EXPECT_EQ(&True, &Context.getBoolConstant(true));
    EXPECT_FALSE(False.value());
    EXPECT_TRUE(True.value());
    EXPECT_EQ(&False.type(), &Context.getBoolType());
    EXPECT_EQ(&True.type(), &Context.getBoolType());
    EXPECT_TRUE(ConstPool.owns(False));
    EXPECT_TRUE(ConstPool.owns(True));
    EXPECT_EQ(Pool.size(), 2U);
  }

  // Pool and context factories share one store keyed by exact integer type and bits.
  TEST(SemanticConstantPoolTest, IntegerInterningPreservesTypeAndPayloadIdentity)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    const IntegerType *Signed32 = Context.getIntegerType(32, true);
    const IntegerType *Unsigned32 = Context.getIntegerType(32, false);
    const IntegerType *Signed64 = Context.getIntegerType(64, true);
    ASSERT_NE(Signed32, nullptr);
    ASSERT_NE(Unsigned32, nullptr);
    ASSERT_NE(Signed64, nullptr);
    const IntegerConstant *One = Pool.getIntegerConstant(*Signed32, IntegerBits(32, 1));
    const IntegerConstant *Two = Context.getIntegerConstant(*Signed32, IntegerBits(32, 2));
    const IntegerConstant *UnsignedOne = Pool.getIntegerConstant(*Unsigned32, IntegerBits(32, 1));
    const IntegerConstant *WideOne = Pool.getIntegerConstant(*Signed64, IntegerBits(64, 1));
    ASSERT_NE(One, nullptr);
    ASSERT_NE(Two, nullptr);
    ASSERT_NE(UnsignedOne, nullptr);
    ASSERT_NE(WideOne, nullptr);
    EXPECT_EQ(One, Context.getIntegerConstant(*Signed32, IntegerBits(32, 1)));
    EXPECT_EQ(Two, Pool.getIntegerConstant(*Signed32, IntegerBits(32, 2)));
    EXPECT_NE(One, Two);
    EXPECT_NE(One, UnsignedOne);
    EXPECT_NE(One, WideOne);
    EXPECT_EQ(&One->type(), Signed32);
    EXPECT_EQ(&UnsignedOne->type(), Unsigned32);
    EXPECT_EQ(&WideOne->type(), Signed64);
    EXPECT_TRUE(Pool.owns(*One));
    EXPECT_TRUE(Pool.owns(*Two));
    EXPECT_TRUE(Pool.owns(*UnsignedOne));
    EXPECT_TRUE(Pool.owns(*WideOne));
    EXPECT_EQ(Pool.size(), 6U);
  }

  // Arbitrary-width payloads are copied and retain upper bits after caller mutation and destruction.
  TEST(SemanticConstantPoolTest, WidePayloadsOwnTheirBitsAndCompareEveryWord)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    const IntegerType *Integer = Context.getIntegerType(257, false);
    ASSERT_NE(Integer, nullptr);
    const IntegerConstant *HighBit = nullptr;
    const IntegerConstant *LowBits = nullptr;
    const std::uint64_t ExpectedWords[] = {
        7,
        0,
        0,
        0,
        1,
    };
    {
      IntegerBits Payload(257, ExpectedWords);
      HighBit = Pool.getIntegerConstant(*Integer, Payload);
      ASSERT_NE(HighBit, nullptr);
      Payload = IntegerBits(257, 7);
      LowBits = Pool.getIntegerConstant(*Integer, Payload);
      ASSERT_NE(LowBits, nullptr);
      Payload = IntegerBits(257, 0);
    }
    EXPECT_NE(HighBit, LowBits);
    const IntegerBits Expected(257, ExpectedWords);
    EXPECT_EQ(HighBit->value(), Expected);
    EXPECT_EQ(LowBits->value(), IntegerBits(257, 7));
    EXPECT_EQ(HighBit, Pool.getIntegerConstant(*Integer, Expected));
    EXPECT_EQ(LowBits, Pool.getIntegerConstant(*Integer, IntegerBits(257, 7)));
    EXPECT_TRUE(Pool.owns(*HighBit));
    EXPECT_TRUE(Pool.owns(*LowBits));
    EXPECT_EQ(Pool.size(), 4U);
  }

  // Integer payloads copy caller words in least-significant-first order and distinguish changes in any word.
  TEST(SemanticConstantPoolTest, IntegerPayloadsOwnEveryCallerWord)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    const IntegerType *Type = Context.getIntegerType(192, false);
    ASSERT_NE(Type, nullptr);
    const std::uint64_t ExpectedWords[] = {
        0x0123456789abcdef,
        0xfedcba9876543210,
        0x8000000000000001,
    };
    const IntegerConstant *Value = nullptr;
    {
      std::vector<std::uint64_t> Words(std::begin(ExpectedWords), std::end(ExpectedWords));
      IntegerBits Payload(192, Words);
      Words[0] = 0;
      Value = Pool.getIntegerConstant(*Type, Payload);
      ASSERT_NE(Value, nullptr);
      Payload = IntegerBits(192, 0);
    }
    ASSERT_EQ(Value->value().words().size(), 3U);
    for (std::size_t Index = 0; Index < 3; ++Index)
    {
      EXPECT_EQ(Value->value().words()[Index], ExpectedWords[Index]);
      std::vector<std::uint64_t> Changed(std::begin(ExpectedWords), std::end(ExpectedWords));
      Changed[Index] ^= 1;
      const IntegerConstant *Other = Pool.getIntegerConstant(*Type, IntegerBits(192, Changed));
      ASSERT_NE(Other, nullptr);
      EXPECT_NE(Value, Other);
    }
    EXPECT_EQ(Value, Context.getIntegerConstant(*Type, IntegerBits(192, ExpectedWords)));
    EXPECT_EQ(Pool.size(), 6U);
  }

  // Partial and whole words enforce exact storage length and zero unused bits without masking or inserting failures.
  TEST(SemanticConstantPoolTest, IntegerWordBoundariesRejectMalformedPayloads)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    struct Boundary
    {
        std::uint32_t Width;
        std::size_t WordCount;
        std::uint64_t HighWord;
    };
    constexpr Boundary Boundaries[] = {
        {1, 1, 1},
        {8, 1, 0xff},
        {63, 1, 0x7fffffffffffffff},
        {64, 1, UINT64_MAX},
        {65, 2, 1},
        {127, 2, 0x7fffffffffffffff},
        {128, 2, UINT64_MAX},
        {129, 3, 1},
        {257, 5, 1},
        {1025, 17, 1},
    };
    for (const Boundary &Case : Boundaries)
    {
      SCOPED_TRACE(Case.Width);
      const IntegerType *Type = Context.getIntegerType(Case.Width, false);
      ASSERT_NE(Type, nullptr);
      std::vector<std::uint64_t> Words(Case.WordCount, UINT64_MAX);
      Words.back() = Case.HighWord;
      const IntegerBits Payload(Case.Width, Words);
      EXPECT_TRUE(Payload.valid());
      const IntegerConstant *Value = Pool.getIntegerConstant(*Type, Payload);
      ASSERT_NE(Value, nullptr);
      EXPECT_EQ(Value->value().bitWidth(), Case.Width);
      EXPECT_EQ(Value->value().words().size(), Case.WordCount);
      const std::size_t OriginalSize = Pool.size();
      Words.pop_back();
      EXPECT_EQ(Pool.getIntegerConstant(*Type, IntegerBits(Case.Width, Words)), nullptr);
      Words.push_back(Case.HighWord);
      Words.push_back(0);
      EXPECT_EQ(Pool.getIntegerConstant(*Type, IntegerBits(Case.Width, Words)), nullptr);
      Words.pop_back();
      if (Case.HighWord != UINT64_MAX)
      {
        Words.back() = Case.HighWord + 1;
        EXPECT_EQ(Pool.getIntegerConstant(*Type, IntegerBits(Case.Width, Words)), nullptr);
      }
      if (Case.Width < 64)
      {
        EXPECT_EQ(Pool.getIntegerConstant(*Type, IntegerBits(Case.Width, Case.HighWord + 1)), nullptr);
      }
      EXPECT_EQ(Value, Context.getIntegerConstant(*Type, Payload));
      EXPECT_TRUE(Pool.owns(*Value));
      EXPECT_EQ(Pool.size(), OriginalSize);
    }
    const IntegerType *Type = Context.getIntegerType(32, false);
    ASSERT_NE(Type, nullptr);
    const std::uint64_t Word = 0;
    const IntegerBits ZeroWidth(0, 0);
    const IntegerBits MissingWords(32, std::span<const std::uint64_t>{});
    const IntegerBits HugeWidth(UINT32_MAX, std::span<const std::uint64_t>(&Word, 1));
    EXPECT_FALSE(ZeroWidth.valid());
    EXPECT_FALSE(MissingWords.valid());
    EXPECT_FALSE(HugeWidth.valid());
    const std::size_t OriginalSize = Pool.size();
    EXPECT_EQ(Pool.getIntegerConstant(*Type, ZeroWidth), nullptr);
    EXPECT_EQ(Pool.getIntegerConstant(*Type, MissingWords), nullptr);
    EXPECT_EQ(Pool.getIntegerConstant(*Type, HugeWidth), nullptr);
    EXPECT_EQ(Pool.size(), OriginalSize);
  }

  // Width mismatches and foreign types fail without modifying either pool's existing constants.
  TEST(SemanticConstantPoolTest, InvalidRequestsLeaveStorageUnchanged)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    ConstantPool &Pool = Context.constantPool();
    const IntegerType *Local = Context.getIntegerType(32, true);
    const IntegerType *Foreign = Other.getIntegerType(32, true);
    ASSERT_NE(Local, nullptr);
    ASSERT_NE(Foreign, nullptr);
    const IntegerConstant *One = Pool.getIntegerConstant(*Local, IntegerBits(32, 1));
    ASSERT_NE(One, nullptr);
    const std::size_t OriginalSize = Pool.size();
    const unsigned InvalidWidths[] = {
        1,
        31,
        33,
        65,
    };
    for (unsigned Width : InvalidWidths)
    {
      SCOPED_TRACE(Width);
      EXPECT_EQ(Pool.getIntegerConstant(*Local, IntegerBits(Width, 1)), nullptr);
      EXPECT_EQ(Pool.size(), OriginalSize);
    }
    EXPECT_EQ(Pool.getIntegerConstant(*Foreign, IntegerBits(32, 1)), nullptr);
    EXPECT_EQ(Other.constantPool().getIntegerConstant(*Local, IntegerBits(32, 1)), nullptr);
    EXPECT_EQ(Pool.size(), OriginalSize);
    EXPECT_EQ(Other.constantPool().size(), 2U);
    EXPECT_EQ(One, Pool.getIntegerConstant(*Local, IntegerBits(32, 1)));
    EXPECT_TRUE(Pool.owns(*One));
  }

  // Equal values in different contexts have distinct owners even when both borrow the same compilation.
  TEST(SemanticConstantPoolTest, OwnershipIsLocalToTheContext)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    ConstantPool &Pool = Context.constantPool();
    ConstantPool &OtherPool = Other.constantPool();
    const IntegerType *LocalType = Context.getIntegerType(1, false);
    const IntegerType *OtherType = Other.getIntegerType(1, false);
    ASSERT_NE(LocalType, nullptr);
    ASSERT_NE(OtherType, nullptr);
    const IntegerConstant *LocalOne = Pool.getIntegerConstant(*LocalType, IntegerBits(1, 1));
    const IntegerConstant *OtherOne = OtherPool.getIntegerConstant(*OtherType, IntegerBits(1, 1));
    ASSERT_NE(LocalOne, nullptr);
    ASSERT_NE(OtherOne, nullptr);
    EXPECT_NE(&Pool, &OtherPool);
    EXPECT_NE(LocalOne, OtherOne);
    EXPECT_TRUE(Pool.owns(*LocalOne));
    EXPECT_FALSE(Pool.owns(*OtherOne));
    EXPECT_TRUE(OtherPool.owns(*OtherOne));
    EXPECT_FALSE(OtherPool.owns(*LocalOne));
    EXPECT_FALSE(Pool.owns(OtherPool.getBoolConstant(false)));
    EXPECT_FALSE(Pool.owns(OtherPool.getBoolConstant(true)));
    EXPECT_FALSE(OtherPool.owns(Pool.getBoolConstant(false)));
    EXPECT_FALSE(OtherPool.owns(Pool.getBoolConstant(true)));
    EXPECT_NE(static_cast<const Constant *>(LocalOne), &Pool.getBoolConstant(true));
    EXPECT_EQ(Pool.size(), 3U);
    EXPECT_EQ(OtherPool.size(), 3U);
  }

  // Rehashing the pool preserves constant addresses, payload references and store operands.
  TEST(SemanticConstantPoolTest, StorageGrowthPreservesBorrowedReferences)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    ConstantPool &Pool = Context.constantPool();
    const IntegerType *Integer = Context.getIntegerType(128, true);
    ASSERT_NE(Integer, nullptr);
    const IntegerConstant *Zero = Pool.getIntegerConstant(*Integer, IntegerBits(128, 0));
    ASSERT_NE(Zero, nullptr);
    const IntegerBits &Bits = Zero->value();
    const BoolConstant &False = Pool.getBoolConstant(false);
    AllocaInstruction *Slot = Context.createAllocaInstruction(*Integer);
    ASSERT_NE(Slot, nullptr);
    const StoreInstruction *Store = Context.createStoreInstruction(*Slot, *Zero);
    ASSERT_NE(Store, nullptr);
    for (std::uint64_t Index = 1; Index <= 2048; ++Index)
    {
      const IntegerConstant *Item = Pool.getIntegerConstant(*Integer, IntegerBits(128, Index));
      ASSERT_NE(Item, nullptr);
      EXPECT_TRUE(Pool.owns(*Item));
    }
    EXPECT_EQ(Pool.size(), 2051U);
    EXPECT_EQ(Zero, Context.getIntegerConstant(*Integer, IntegerBits(128, 0)));
    EXPECT_EQ(&Zero->value(), &Bits);
    EXPECT_EQ(Bits, IntegerBits(128, 0));
    EXPECT_EQ(&Pool.getBoolConstant(false), &False);
    EXPECT_EQ(&Store->storedValue(), Zero);
    EXPECT_EQ(&Slot->allocatedType(), Integer);
    EXPECT_TRUE(Pool.owns(*Zero));
    EXPECT_TRUE(Pool.owns(False));
    EXPECT_EQ(Pool.size(), 2051U);
  }
} // namespace ink::semantic::test
