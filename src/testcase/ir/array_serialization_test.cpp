#include "ink/ir/ir_builder.h"
#include "ink/ir/module/module_serialization.h"

#include <gtest/gtest.h>
#include <llvm/Support/Endian.h>

#include <array>

namespace ink::ir::test
{
  namespace
  {
    constexpr std::string_view ArrayModuleText = R"(ink-ir 4
module @Arrays {
  type !pair = [2 x i32]
  type !nested = [2 x !pair]
  type !empty = [0 x i32]
  define i32 @main(i32 %index) {
  entry:
    %array = array !pair [1, %index]
    %repeated = array.repeat !pair [%index]
    %empty = array !empty []
    %empty_repeat = array.repeat !empty [%index]
    %slot = alloca !pair
    store !pair %array, ptr<rw, !pair> %slot
    %pointer = array.element ptr<rw, i32>, ptr<rw, !pair> %slot, i32 %index
    %element = array.extract i32, !pair %repeated, i32 1
    %row = array.extract !pair, !nested [[3, 4], [3, 4]], i32 0
    %constant = array.extract i32, !pair [3, 4], i32 0
    %empty_slot = alloca !empty
    store !empty [], ptr<rw, !empty> %empty_slot
    ret i32 %element
  }
}
)";

    struct ArrayBinaryRecord
    {
        std::size_t Offset;
        std::uint32_t Id;
        std::uint32_t Kind;
    };

    std::vector<ArrayBinaryRecord> arrayBinaryRecords(std::string_view Bytes)
    {
      std::vector<ArrayBinaryRecord> Records;
      for (std::size_t Offset = 16; Offset < Bytes.size();)
      {
        const auto *Header = Bytes.data() + Offset;
        Records.push_back({Offset, static_cast<std::uint32_t>(Records.size() + 1), llvm::support::endian::read32le(Header)});
        Offset += 24 + static_cast<std::size_t>(llvm::support::endian::read32le(Header + 12)) * 8 + static_cast<std::size_t>(llvm::support::endian::read64le(Header + 16));
      }
      return Records;
    }
  } // namespace

  // Text and binary round trips preserve array construction, repetition, index operands and nested constant interning.
  TEST(IRModuleSerializationTest, ArrayInstructionsAndConstantsRoundTrip)
  {
    core::CompilationContext Compilation;
    IRContext Source(Compilation);
    const auto Parsed = deserializeModuleText(Source, ArrayModuleText);
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    for (const bool Binary : {false, true})
    {
      const auto Encoded = Binary ? serializeModuleBinary(*Parsed.ModuleValue) : serializeModuleText(*Parsed.ModuleValue);
      ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
      IRContext Destination(Compilation);
      const auto Decoded = Binary ? deserializeModuleBinary(Destination, Encoded.Bytes) : deserializeModuleText(Destination, Encoded.Bytes);
      ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
      const auto &FunctionValue = static_cast<const Function &>(*Decoded.ModuleValue->entryBlock().values()[0]);
      const auto &Values = FunctionValue.entryBlock()->values();
      ASSERT_EQ(Values.size(), 13U);
      const auto &Array = static_cast<const ArrayInstruction &>(*Values[0]);
      EXPECT_FALSE(Array.repeated());
      ASSERT_EQ(Array.elements().size(), 2U);
      EXPECT_EQ(Array.elements()[1], FunctionValue.parameters()[0].get());
      EXPECT_TRUE(static_cast<const ArrayInstruction &>(*Values[1]).repeated());
      EXPECT_TRUE(static_cast<const ArrayInstruction &>(*Values[2]).elements().empty());
      EXPECT_EQ(static_cast<const ArrayInstruction &>(*Values[3]).arrayType().elementCount(), 0U);
      EXPECT_EQ(static_cast<const ArrayInstruction &>(*Values[3]).elements().size(), 1U);
      const auto &Pointer = static_cast<const ArrayElementPointerInstruction &>(*Values[6]);
      EXPECT_EQ(&Pointer.address(), Values[4].get());
      EXPECT_EQ(&Pointer.index(), FunctionValue.parameters()[0].get());
      const auto &Extract = static_cast<const ArrayExtractInstruction &>(*Values[7]);
      EXPECT_EQ(&Extract.array(), Values[1].get());
      const auto &Nested = static_cast<const ArrayConstant &>(static_cast<const ArrayExtractInstruction &>(*Values[8]).array());
      ASSERT_EQ(Nested.elements().size(), 2U);
      EXPECT_EQ(Nested.elements()[0], Nested.elements()[1]);
      EXPECT_EQ(Nested.elements()[0], &static_cast<const ArrayExtractInstruction &>(*Values[9]).array());
      EXPECT_TRUE(Destination.constantPool().owns(Nested));
      const auto &Empty = static_cast<const ArrayConstant &>(static_cast<const StoreInstruction &>(*Values[11]).storedValue());
      EXPECT_TRUE(Empty.elements().empty());
      const auto Reencoded = Binary ? serializeModuleBinary(*Decoded.ModuleValue) : serializeModuleText(*Decoded.ModuleValue);
      ASSERT_TRUE(Reencoded.succeeded()) << Reencoded.Message;
      EXPECT_EQ(Reencoded.Bytes, Encoded.Bytes);
    }
  }

  // Array wire records append IDs 56-59 while retaining version 4 and every existing tag identity.
  TEST(IRModuleSerializationTest, ArraysUseAppendedStableWireTags)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto Parsed = deserializeModuleText(Context, ArrayModuleText);
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    const auto Encoded = serializeModuleBinary(*Parsed.ModuleValue);
    ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
    EXPECT_EQ(llvm::support::endian::read32le(Encoded.Bytes.data() + 4), 4U);
    std::array<std::size_t, 4> Counts{};
    for (const auto &Record : arrayBinaryRecords(Encoded.Bytes))
    {
      if (Record.Kind >= 56 && Record.Kind <= 59)
      {
        ++Counts[Record.Kind - 56];
      }
    }
    EXPECT_EQ(Counts, (std::array<std::size_t, 4>{3, 4, 1, 3}));
  }

  // Invalid lengths, element types, indices, result types and cyclic or non-constant array elements fail before publication.
  TEST(IRModuleSerializationTest, RejectsMalformedArrayText)
  {
    constexpr std::string_view Invalid[] = {
        "%x = array [2 x i32] [1]",
        "%x = array [1 x i32] [1, 2]",
        "%x = array [1 x i32] [true]",
        "%x = array [1 x i32] [%flag]",
        "%x = array.repeat [2 x i32] []",
        "%x = array.repeat [2 x i32] [1, 2]",
        "%x = array i32 [1]",
        "%x = array.extract i32, [1 x i32] [1], bool true",
        "%x = array.extract bool, [1 x i32] [1], i32 0",
        "%x = array.extract i32, i32 1, i32 0",
        "%x = array.extract i32, [2 x i32] [1], i32 0",
        "%x = array.extract i32, [1 x i32] [%number], i32 0",
        "%x = array.extract i32, [1 x i32] [%x], i32 0",
        "%x = array.extract i32, [1 x i32] [%missing], i32 0",
        "%x = array.element ptr<rw, i32>, [1 x i32] [1], i32 0",
        "%p = alloca i32 %x = array.element ptr<rw, i32>, ptr<rw, i32> %p, i32 0",
        "%p = alloca [1 x i32] %x = array.element ptr<ro, i32>, ptr<rw, [1 x i32]> %p, i32 0",
    };
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    for (const auto Instructions : Invalid)
    {
      const auto Result = deserializeModuleText(Context, "ink-ir 4 module @Invalid { define void @f(i32 %number, bool %flag) { entry: " + std::string(Instructions) + " ret void } }");
      EXPECT_EQ(Result.Status, ModuleArchiveStatus::InvalidArchive) << Instructions << ": " << Result.Message;
      EXPECT_EQ(Result.ModuleValue, nullptr);
      EXPECT_TRUE(Context.modules().empty());
    }
  }

  // Binary arrays reject corrupt references, owners, result types and repetition flags without exposing partially restored modules.
  TEST(IRModuleSerializationTest, RejectsMalformedArrayBinary)
  {
    core::CompilationContext Compilation;
    IRContext Source(Compilation);
    const auto Parsed = deserializeModuleText(Source, ArrayModuleText);
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    const auto Encoded = serializeModuleBinary(*Parsed.ModuleValue);
    ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
    const auto Reject = [&](const std::string &Bytes)
    {
      IRContext Destination(Compilation);
      const auto Result = deserializeModuleBinary(Destination, Bytes);
      EXPECT_EQ(Result.Status, ModuleArchiveStatus::InvalidArchive) << Result.Message;
      EXPECT_EQ(Result.ModuleValue, nullptr);
      EXPECT_TRUE(Destination.modules().empty());
    };
    const auto Records = arrayBinaryRecords(Encoded.Bytes);
    std::uint32_t IntegerTypeId = 0;
    for (const auto &Record : Records)
    {
      if (Record.Kind == 21)
      {
        IntegerTypeId = Record.Id;
      }
    }
    ASSERT_NE(IntegerTypeId, 0U);
    for (const auto &Record : Records)
    {
      if (Record.Kind < 56 || Record.Kind > 59)
      {
        continue;
      }
      const auto FieldCount = llvm::support::endian::read32le(Encoded.Bytes.data() + Record.Offset + 12);
      if (Record.Kind != 59)
      {
        auto WrongType = Encoded.Bytes;
        llvm::support::endian::write32le(WrongType.data() + Record.Offset + 4, IntegerTypeId);
        Reject(WrongType);
      }
      for (std::uint32_t Index = Record.Kind == 57 ? 1 : 0; Index < FieldCount; ++Index)
      {
        for (std::uint64_t InvalidId : {std::uint64_t{0}, std::uint64_t{UINT32_MAX}, std::uint64_t{Record.Id}, std::uint64_t{1}})
        {
          auto WrongReference = Encoded.Bytes;
          llvm::support::endian::write64le(WrongReference.data() + Record.Offset + 24 + Index * 8, InvalidId);
          Reject(WrongReference);
        }
      }
      if (Record.Kind == 56)
      {
        auto OwnedConstant = Encoded.Bytes;
        llvm::support::endian::write32le(OwnedConstant.data() + Record.Offset + 8, 1);
        Reject(OwnedConstant);
      }
      if (Record.Kind == 57)
      {
        auto WrongFlag = Encoded.Bytes;
        llvm::support::endian::write64le(WrongFlag.data() + Record.Offset + 24, 2);
        Reject(WrongFlag);
      }
    }
  }

  // Array text decoding enforces configured element and nesting budgets before allocating unbounded constant trees.
  TEST(IRModuleSerializationTest, ArrayConstantsRespectArchiveLimits)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    ModuleArchiveLimits Limits;
    Limits.MaxFields = 4;
    const auto TooMany = deserializeModuleText(Context, "ink-ir 4 module @M { define [5 x i32] @f() { entry: ret [5 x i32] [1, 2, 3, 4, 5] } }", Limits);
    EXPECT_EQ(TooMany.Status, ModuleArchiveStatus::LimitExceeded);
    Limits = {};
    Limits.MaxNestingDepth = 4;
    const auto TooDeep = deserializeModuleText(Context, "ink-ir 4 module @M { type !a = [1 x i32] type !b = [1 x !a] type !c = [1 x !b] type !d = [1 x !c] type !e = [1 x !d] define !e @f() { entry: ret !e [[[[[1]]]]] } }", Limits);
    EXPECT_EQ(TooDeep.Status, ModuleArchiveStatus::LimitExceeded);
    EXPECT_TRUE(Context.modules().empty());
  }
} // namespace ink::ir::test
